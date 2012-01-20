#include "common_types.h"
#include "cli.h"

int parse_cmdline (int argc, char *argv[]);
int lib_init (void);
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

	parse_cmdline (argc, argv);

	switch_mac[5] = atoi (argv[1]);

	layer3switch_init ();

	while (1) {
		sleep (-1);
	}
	return 0;
}

int read_port_mac_address (int port, uint8_t *p) 
{
	int i = 0;
	while (i < 5) {
		p[i] = switch_mac[i];
		i++;
	}
	p[5] = (uint8_t)port + 1;
	return 0;
}
