#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"
#include "cparser.h"
#include "cparser_tree.h"
#include "dhcp.h"

extern u8_t g_dhcp_debug;

cparser_result_t cparser_cmd_debug_dhcp_client (cparser_context_t *context)
{
	g_dhcp_debug = 0x80;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_no_debug_dhcp_client (cparser_context_t *context)
{
	g_dhcp_debug = 0x00;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_if_ip_address_dhcp(cparser_context_t *context)
{
	int port = cli_get_port ();
	if (!dhcp_start (IF_INFO (port)))
		return CPARSER_OK;
	return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_if_no_ip_address_dhcp(cparser_context_t *context)
{
	int port = cli_get_port ();
	if (!dhcp_release (IF_INFO (port))) {
		dhcp_stop (IF_INFO (port));
		return CPARSER_OK;
	}
	return CPARSER_NOT_OK;
}

cparser_result_t cparser_cmd_if_ip_dhcp_release(cparser_context_t *context)
{
	int port = cli_get_port ();
	if (!dhcp_release (IF_INFO (port))) {
		return CPARSER_OK;
	}
	return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_if_ip_dhcp_renew(cparser_context_t *context)
{
	int port = cli_get_port ();
	if (!dhcp_renew (IF_INFO (port))) {
		return CPARSER_OK;
	}
	return CPARSER_NOT_OK;
}

cparser_result_t cparser_cmd_show_dhcp_client_lease(cparser_context_t *context)
{
	int i = 0;

	cli_printf ("DHCP Client lease information\n");
	cli_printf ("-----------------------------\n");	

	while (i < get_max_ports ()) {
	
		struct interface *netif = IF_INFO(i + 1);

		if (netif && netif->dhcp && (netif->flags & NETIF_FLAG_DHCP)) {
			struct dhcp *p = netif->dhcp;
			uint8_t temp[4];
			uint8_t *addr = &temp[0];

			cli_printf  ("%s :\n", netif->ifDescr);

			cli_printf  ("\tInternet address is ");
			uint32_2_ipstring (p->offered_ip_addr.addr, addr);
			cli_printf("%u.%u.%u.%u", addr[0], addr[1],addr[2],addr[3]);

			cli_printf  (", subnet mask is ");
			uint32_2_ipstring (p->offered_sn_mask.addr, addr);
			cli_printf("%u.%u.%u.%u\n", addr[0], addr[1],addr[2],addr[3]);

			cli_printf ("\tdefault-gateway addr: ");
			uint32_2_ipstring (p->offered_gw_addr.addr, addr);
			cli_printf("%u.%u.%u.%u\n", addr[0], addr[1],addr[2],addr[3]);

			cli_printf ("\tDHCP Lease server: ");
			uint32_2_ipstring (p->server_ip_addr.addr, addr);
			cli_printf("%u.%u.%u.%u\n", addr[0], addr[1],addr[2],addr[3]);
			cli_printf ("\tRetries: %d\n", p->tries);
		        cli_printf ("\tLease: %u secs, Renewal: %u secs, Rebind: %u secs\n",
				p->offered_t0_lease, p->offered_t1_renew, p->offered_t2_rebind);

			if (netif->hostname[0])
				cli_printf ("\tHostname: %s\n", netif->hostname);
		}
		i++;
	}
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_if_ip_dhcp_client_hostname(cparser_context_t *context, char **hostname_ptr)
{
	int port = cli_get_port ();
	if_set_hostname (IF_INFO (port), *hostname_ptr);
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_if_ip_dhcp_client_lease_days_hours_mins(cparser_context_t *context,
    int32_t *days_ptr,
    int32_t *hours_ptr,
    int32_t *mins_ptr)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
