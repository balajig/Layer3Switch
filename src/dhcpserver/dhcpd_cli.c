/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */


#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"
#include "cparser.h"
#include "cparser_tree.h"
#include "dhcp.h"

cparser_result_t cparser_cmd_service_dhcp(cparser_context_t *context)
{
	if (dhcpd_start ())
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_ip_dhcp_excluded_address_lowaddress_highaddress(cparser_context_t *context,
		uint32_t *lowaddress_ptr,
		uint32_t *highaddress_ptr)
{
}
cparser_result_t cparser_cmd_ip_dhcp_ping_packets_count(cparser_context_t *context,
		int32_t *count_ptr)
{
}
cparser_result_t cparser_cmd_ip_dhcp_ping_timeout_milliseconds(cparser_context_t *context,
		int32_t *milliseconds_ptr)
{
}
cparser_result_t cparser_cmd_ip_dhcp_pool_name(cparser_context_t *context,
		char **name_ptr)
{
}

cparser_result_t cparser_cmd_dhcppool_network_networkaddr_mask(cparser_context_t *context,
		uint32_t *networkaddr_ptr,
		uint32_t *mask_ptr)
{

}
cparser_result_t cparser_cmd_dhcppool_domain_name_domain(cparser_context_t *context,
		char **domain_ptr)
{
}
cparser_result_t cparser_cmd_dhcppool_dns_server_serveraddr(cparser_context_t *context,
		uint32_t *serveraddr_ptr)
{
}
cparser_result_t cparser_cmd_dhcppool_default_router_address(cparser_context_t *context,
		uint32_t *address_ptr)
{
}
cparser_result_t cparser_cmd_dhcppool_lease_days_hours_minutes(cparser_context_t *context,
		int32_t *days_ptr,
		int32_t *hours_ptr,
		int32_t *minutes_ptr)
{
}
cparser_result_t cparser_cmd_dhcppool_lease_infinite(cparser_context_t *context)
{
}

cparser_result_t cparser_cmd_clear_ip_dhcp_binding_address(cparser_context_t *context,
		uint32_t *address_ptr)
{
}
cparser_result_t cparser_cmd_clear_ip_dhcp_server_statistics(cparser_context_t *context)
{
}
cparser_result_t cparser_cmd_show_ip_dhcp_binding_address(cparser_context_t *context,
		uint32_t *address_ptr)
{
}
cparser_result_t cparser_cmd_show_ip_dhcp_server_statistics(cparser_context_t *context)
{
}