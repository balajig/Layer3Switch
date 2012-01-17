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
#undef PKT_DBG 
#define MAX_MTU 2048

struct linux_if_mapping {
	int linux_ifIndex;
};

extern struct linux_if_mapping linux_if_map[];

void * packet_processing_task (void *unused);
int rcv_pkt (int sockid, void *buf);
void process_pkt (void  *pkt, int len, uint16_t port);

void * packet_processing_task (void *unused)
{
	int len = 0;
	int sock_id = -1;
	int ifport = (int)unused;

	sock_id = (int)port_cdb[ifport].platform;

	if (sock_id < 0)
		sleep (sock_id);

	while (1) {
		char *buf = malloc (MAX_MTU);
		len = rcv_pkt (sock_id, buf);
		if (len > 0) {
#ifdef PKT_DBG
			printf (" Index : %d\n", linux_if_map[ifport].linux_ifIndex);
#endif
			process_pkt (buf, len, ifport + 1);
		}
		free (buf);
	}
	return 0;
}

int rcv_pkt (int sockid, void *buf)
{
        struct sockaddr_ll sl;
        socklen_t slen = sizeof sl;

	int len = recvfrom (sockid, buf, MAX_MTU, 0, (struct sockaddr *) &sl, &slen);
#ifdef PKT_DBG
        printf("\nReceive Src ifindex %d %02x:%02x:%02x:%02x:%02x:%02x",
               sl.sll_ifindex,
               sl.sll_addr[0], sl.sll_addr[1], sl.sll_addr[2],
               sl.sll_addr[3], sl.sll_addr[4], sl.sll_addr[5]);
#endif
	return len;
}
