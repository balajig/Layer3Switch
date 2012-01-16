#include "common_types.h"
#include "cli.h"
#include "cparser.h"
#include "cparser_tree.h"
#include "ip_addr.h"
#include "netif/etharp.h"

cparser_result_t cparser_cmd_show_arp(cparser_context_t *context)
{
	if (!show_arp_entries ())
		return CPARSER_OK;
	return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_config_no_arp_ipaddr (cparser_context_t *context, uint32_t *ipaddr_ptr)
{
	ip_addr_t ipaddr;

	ipaddr.addr = *ipaddr_ptr;
	
	if (!etharp_remove_static_entry (&ipaddr))
		return CPARSER_OK;
	return CPARSER_NOT_OK;
}

cparser_result_t cparser_cmd_config_arp_ipaddr_hostmacaddr(cparser_context_t *context,
    uint32_t *ipaddr_ptr,
    cparser_macaddr_t *hostmacaddr_ptr)
{
	ip_addr_t ipaddr;
	struct eth_addr ethaddr;

	ipaddr.addr = *ipaddr_ptr;
	
	memcpy (ethaddr.addr, hostmacaddr_ptr->octet, sizeof (ethaddr.addr));

	if (!etharp_add_static_entry (&ipaddr, &ethaddr))
		return CPARSER_OK;
	return CPARSER_NOT_OK;
}

