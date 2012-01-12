#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"
#include "cparser.h"
#include "cparser_tree.h"

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

cparser_result_t cparser_cmd_if_ip_dhcp_client_hostname(cparser_context_t *context, char **hostname_ptr)
{
	printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_if_ip_dhcp_client_lease_days_hours_mins(cparser_context_t *context,
    int32_t *days_ptr,
    int32_t *hours_ptr,
    int32_t *mins_ptr)
{
	printf ("Not Implemented\n");
	return CPARSER_OK;
}
