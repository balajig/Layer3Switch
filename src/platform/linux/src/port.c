#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"

int stp_send_event (int event, int port, int vlanid);
int port_init (void);
void send_interface_enable_or_disable (int port , int state);
int read_interfaces (void);
int cli_show_interfaces (int port);

if_t port_cdb[MAX_PORTS];

int port_init (void)
{
	int idx = 0;

	read_interfaces ();

	idx = get_max_ports ();

	while (idx--) {
		interface_init (&port_cdb[idx], NULL, NULL);
		port_cdb[idx].ifLastChange = 0;
		port_cdb[idx].ifInOctets = 0;
		port_cdb[idx].ifInUcastPkts = 0;
		port_cdb[idx].ifInDiscards = 0;
		port_cdb[idx].ifInErrors = 0;
		port_cdb[idx].ifInUnknownProtos = 0;
		port_cdb[idx].ifOutOctets = 0;
		port_cdb[idx].ifOutUcastPkts = 0;
		port_cdb[idx].ifOutDiscards = 0;
		port_cdb[idx].ifOutErrors = 0;
		port_cdb[idx].pstp_info = NULL;
		port_cdb[idx].flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	}
	read_interfaces ();
	return 0;
}

void send_interface_enable_or_disable (int port , int state)
{
	int vlanid = 1;
	stp_send_event (state, port, vlanid);
}

int get_port_oper_state (uint32_t port)
{
	return port_cdb[port - 1].ifOperStatus;
}

int get_port_mac_address (uint32_t port, uint8_t *mac)
{
	memcpy (mac, port_cdb[port-1].ifPhysAddress, 6);
	return 0;
}

int cli_show_interfaces (int port)
{
	int idx = -1;

	const char *state[]  = {"UNKNWN", "UP", "DOWN"};

	printf (" Port      Name       MTU    Type    Admin    Oper   LastChange\n");
	printf (" ----     -----      -----  ------   ------  -----   ----------\n");
	while (++idx < get_max_ports ()) {
		if (port_cdb[idx].ifIndex < 0)
			break;
		printf (" %-3d       %-8s   %-5d   %-6s  %-4s    %-4s        %-4d\n",
		idx+1, port_cdb[idx].ifDescr,
		port_cdb[idx].ifMtu, "ETH", state[port_cdb[idx].ifAdminStatus],
		state[port_cdb[idx].ifOperStatus], port_cdb[idx].ifLastChange);
	}

	return 0;
}
