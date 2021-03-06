#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"

int stp_send_event (int event, int port, int vlanid);
int port_init (void);
void send_interface_enable_or_disable (int port , int state);
int cli_show_interfaces (int port);
int get_max_phy_ports (void);
int get_max_ports (void);
int make_if_down (if_t *p);
int make_if_up (if_t *p);

if_t port_cdb[MAX_PORTS];

int port_init (void)
{
	int idx = 0;
	if_loopif_init ();
	while (idx  < get_max_phy_ports ()) {
		interface_init (&port_cdb[idx], NULL, NULL);
		sprintf ((char *)port_cdb[idx].ifDescr, "%s%d","port",idx);
		port_cdb[idx].ifIndex = idx + 1;
		port_cdb[idx].ifType = 1;
		port_cdb[idx].ifMtu = 1500;
		port_cdb[idx].ifSpeed = 10;
		port_cdb[idx].ifAdminStatus = IF_DOWN;
		port_cdb[idx].ifOperStatus = IF_DOWN;
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
		if_connect_init (&port_cdb[idx]);
		read_port_mac_address (idx, &port_cdb[idx].ifPhysAddress[0]); 
		idx++;
	}
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

int get_max_phy_ports (void)
{
        return CONFIG_MAX_PHY_PORTS;
}

int get_max_ports (void)
{
        return CONFIG_MAX_PHY_PORTS;
}

int make_if_down (if_t *p)
{
	p->ifAdminStatus = IF_DOWN;
	p->ifOperStatus = IF_DOWN;
	return 0;
}

int make_if_up (if_t *p)
{
	p->ifAdminStatus = IF_UP;
	p->ifOperStatus = IF_UP;
	return 0;
}
