#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>      
#include <sys/stat.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <getopt.h>
#include <netdb.h>
#include <asm/types.h>
#include "common_types.h"
#include "ifmgmt.h"

#define MAX_MTU 2048

void * packet_processing_task (void *unused)
{
	int retval = -1;
	int len = 0;
	int retry = 0;
	char *pkt_buf = NULL;
	char *buf = NULL;
	fd_set rfds;
	int max_fd = -1;
	int max_ports = get_max_ports ();

	while (1) {
		int i = 0;

		FD_ZERO(&rfds);

		while (i < max_ports) {
			FD_SET((int)port_cdb[i].platform, &rfds);
			i++;
		}

		max_fd = (int)port_cdb[i-1].platform;

		retval = select(max_fd + 1, &rfds, NULL, NULL, NULL);

		if (retval < 0)
			continue;
		i = 0;
		while (i < max_ports) {
			if (FD_ISSET((int)port_cdb[i].platform, &rfds)) {
				char *buf = malloc (MAX_MTU);
				len = rcv_pkt ((int)port_cdb[i].platform, buf);
				if (len > 0)
					process_pkt (buf, len, i);
				free (buf);
			}
			i++;
		}
	}

	return 0;
}

int rcv_pkt (int sockid, void *buf)
{
	struct sockaddr_in si_other;
	socklen_t slen=sizeof(si_other);

	int len = recvfrom (sockid, buf, ETH_FRAME_LEN, 0, (struct sockaddr *) &si_other, &slen);
#ifdef PKT_DBG
	printf("Received packet from %s:%d\n", 
			inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
#endif

	return len;
}
