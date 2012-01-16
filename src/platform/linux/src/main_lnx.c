#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include "common_types.h"
#include "ifmgmt.h"
#include "cli.h"

int parse_cmdline (int argc, char *argv[]);
int tmlib_init (void);
int cli_init (const char *prmt);
int spawn_pkt_processing_task (void);
int port_init (void);
int ip_init (void);
int bridge_init (void);
int vrrp_init (void);
int dhcp_init (void);
int init_task_cpu_usage_moniter_timer (void);
int start_cli_task (void);
void * packet_processing_task (void *unused);
void * if_link_monitor (void *unused);

char switch_mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x00};

void dump_task_info (void);
void execute_system_call (char *arg);
static int32_t  sockid_pkt = 0;

struct linux_if_mapping {
	int task_id;
	int linux_ifIndex;
}linux_if_map[MAX_PORTS];


void show_uptime (char *[]);

void execute_system_call (char *arg)
{
	system (arg);
}

int main (int argc, char **argv)
{
	if (argc < 2) {
		fprintf (stderr, "Usage : ./switch <instance>\n");
		return -1;
	}

	switch_mac[5] = atoi (argv[1]);

	create_raw_sock_for_pkt_capture ();

	layer3switch_init ();

	while (1) {
		sleep (-1);
	}
	return 0;
}

int read_port_mac_address (int port, uint8_t *p) 
{
	int i = 0;
	while (i < 6) {
		p[i] = port_cdb[port - 1].ifPhysAddress[i];
		i++;
	}
	return 0;
}

int spawn_pkt_processing_task (void)
{
	int ifport = 0;
	int  taskid = 0;
	char task_name[16];

	if (task_create ("linkmonitor", 98, 3, 32000, if_link_monitor, NULL, NULL, 
		          &taskid) == TSK_FAILURE) {
		printf ("Task creation failed : %s\n", task_name);
		exit (1);
	}

	
	while (ifport < get_max_ports ()) {
		sprintf (task_name, "%s-%d", "PKTRX", ifport);
		if (task_create (task_name, 98, 3, 32000, packet_processing_task, NULL, (void *)ifport, 
			          &linux_if_map[ifport].task_id) == TSK_FAILURE) {
			printf ("Task creation failed : %s\n", task_name);
			exit (1);
		}
		ifport++;
	}

	return 0;
}

int create_raw_sock_for_pkt_capture (void)
{
	struct sockaddr_in si_me;

	memset((char *) &si_me, 0, sizeof(si_me));

	if ((sockid_pkt =socket(AF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror ("SOCKET");
		return -1;
	}

	return  0;
}

void update_linux_if_map (int port, int ifindex)
{
	linux_if_map[port].linux_ifIndex = ifindex;
}
void send_packet (void *buf, uint16_t port, int len)
{
	struct sockaddr_ll socket_address;

	memset (&socket_address, 0, sizeof(socket_address));

	socket_address.sll_family   = PF_PACKET;	
	socket_address.sll_protocol = htons(ETH_P_ALL);	
	socket_address.sll_ifindex  = linux_if_map[port].linux_ifIndex;
	socket_address.sll_halen    = ETH_ALEN;		

        if (sendto ((int)port_cdb[port].platform, buf, len, 0,(struct sockaddr *)&socket_address,
                                sizeof(socket_address)) < 0) {
		;
        }
	return;
}
