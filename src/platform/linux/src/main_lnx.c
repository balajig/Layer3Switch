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

char switch_mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x00};

void dump_task_info (void);
void execute_system_call (char *arg);

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

	tmlib_init ();

	cli_init ("OpenSwitch");

	spawn_pkt_processing_task ();

	ip_init ();

	port_init ();

	bridge_init ();

	vrrp_init ();

	dhcp_init ();

        install_cmd_handler ("uptime", "Displays the uptime", show_uptime, NULL, USER_EXEC_MODE);

	start_cli_task ();

	init_task_cpu_usage_moniter_timer ();

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
	return 0;
}


void send_packet (void *buf, uint16_t port, int len)
{
}
