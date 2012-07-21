#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"
#include "cparser.h"
#include "cparser_tree.h"
#include "hal.h"
#include "rtm.h"

int  cli_set_port_enable (void);
int cli_show_interfaces (int port);
int  cli_set_port_disable (void);
void send_interface_enable_or_disable (int port , int state);
static int cli_show_interfaces_all (void);

cparser_result_t cparser_cmd_if_enable(cparser_context_t *context)
{
	context = context;
	if (!cli_set_port_enable ())
                return CPARSER_OK;
        return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_if_disable(cparser_context_t *context)
{
	context = context;
	if (!cli_set_port_disable ())
                return CPARSER_OK;
        return CPARSER_NOT_OK;
}
#if 0
cparser_result_t cparser_cmd_config_interface_ethernet_portnum(cparser_context_t *context,
    int32_t *portnum_ptr)
{
	char prompt[CPARSER_MAX_PROMPT];
	/* Enter the submode */
        cli_set_port (*portnum_ptr);
        sprintf (prompt, "%s%d%s","(config-if-", *portnum_ptr, ")");
        set_prompt (prompt);
	get_prompt (prompt);
        set_curr_mode (INTERFACE_MODE);
	return cparser_submode_enter(context->parser, NULL, prompt);

}
#endif
cparser_result_t cparser_cmd_if_exit(cparser_context_t *context)
{
	if (!exit_mode ())
	{
		return cparser_submode_exit (context->parser);
	}
	return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_interface_ethernet_portnum(cparser_context_t *context,  int32_t *portnum_ptr)
{
	char prompt[CPARSER_MAX_PROMPT];
	/* Enter the submode */
        cli_set_port (*portnum_ptr);
        sprintf (prompt, "%s%d%s","(config-if-", *portnum_ptr, ")");
        set_prompt (prompt);
	get_prompt (prompt);
        set_curr_mode (INTERFACE_MODE);
	return cparser_submode_enter(context->parser, NULL, prompt);

}

static int cli_show_interfaces_all (void)
{
	int idx = 0;

	const char *state[]  = {"UNKNWN", "UP", "DOWN"};
	const char *type []  = {"UNKNWN", "ETH", "LO"};

	cli_printf (" Port      Name       MTU    Type    Admin    Oper   LastChange\n");
	cli_printf (" ----     -----      -----  ------   ------  -----   ----------\n");
	while (idx < MAX_PORTS) {
		if (!port_cdb[idx].ifIndex) {
			idx++;
			continue;
		}
		cli_printf (" %-3d       %-8s   %-5d   %-6s  %-4s    %-4s        %-4d\n",
		idx+1, port_cdb[idx].ifDescr,
		port_cdb[idx].ifMtu, type[port_cdb[idx].ifType], state[port_cdb[idx].ifAdminStatus],
		state[port_cdb[idx].ifOperStatus], port_cdb[idx].ifLastChange);
		idx++;
	}

	return 0;
}

cparser_result_t cparser_cmd_show_interface(cparser_context_t *context)
{
	context = context;
	if (!cli_show_interfaces_all ())
                return CPARSER_OK;
        return CPARSER_NOT_OK;
}


int  cli_set_port_enable (void)
{
	int port = cli_get_port ();

	if (hal_interface_up (IF_INFO(port)) < 0) {
		return -1;
	}
	send_interface_enable_or_disable (port, IF_UP);
	return 0;
}

int cli_set_port_disable (void)
{
	int port = cli_get_port ();

	if (hal_interface_down (IF_INFO(port)) < 0) {
		return -1;
	}
	send_interface_enable_or_disable (port, IF_DOWN);
	return 0;
}

cparser_result_t cparser_cmd_interface_loopback_portnum(cparser_context_t *context,
    int32_t *portnum_ptr)
{
	char prompt[CPARSER_MAX_PROMPT];
	/* Enter the submode */
        cli_set_port (*portnum_ptr);
        sprintf (prompt, "%s%d%s","(config-if-lo", *portnum_ptr, ")");
        set_prompt (prompt);
	get_prompt (prompt);
        set_curr_mode (INTERFACE_MODE);
	return cparser_submode_enter(context->parser, NULL, prompt);
}
cparser_result_t cparser_cmd_iflo_enable(cparser_context_t *context UNUSED_PARAM)
{
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_disable(cparser_context_t *context UNUSED_PARAM)
{
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_ip_address_addr_mask(cparser_context_t *context UNUSED_PARAM,
    uint32_t *addr_ptr,
    uint32_t *mask_ptr)
{
	int port = cli_get_port () + CONFIG_MAX_PHY_PORTS;
	*addr_ptr = ntohl (*addr_ptr);
	*mask_ptr = ntohl (*mask_ptr);

	if (!set_ip_address (port, *addr_ptr, *mask_ptr))
	{
		connected_route_add (IF_INFO (port), addr_ptr, mask_ptr, 0);
		return CPARSER_OK;
	}
	return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_iflo_ip_address_dhcp(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented/Supported\n");
  	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_ip_dhcp_client_hostname(cparser_context_t *context UNUSED_PARAM,
    char **hostname_ptr UNUSED_PARAM)
{
	cli_printf ("Not Implemented/Supported\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_ip_dhcp_client_lease_days_hours_mins(cparser_context_t *context UNUSED_PARAM,
    int32_t *days_ptr UNUSED_PARAM,
    int32_t *hours_ptr UNUSED_PARAM,
    int32_t *mins_ptr UNUSED_PARAM)
{
	cli_printf ("Not Implemented/Supported\n");
  	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_ip_dhcp_release(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented/Supported\n");
  	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_ip_dhcp_renew(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented/Supported\n");
  	return CPARSER_OK;
}
cparser_result_t cparser_cmd_iflo_no_ip_address_addr_mask(cparser_context_t *context UNUSED_PARAM,
    uint32_t *addr_ptr,
    uint32_t *mask_ptr)
{
	int port = cli_get_port () + CONFIG_MAX_PHY_PORTS;

	*addr_ptr = ntohl (*addr_ptr);
	*mask_ptr = ntohl (*mask_ptr);

	if(!connected_route_delete (IF_INFO (port), addr_ptr, mask_ptr, 0))
		return CPARSER_OK;
  	return CPARSER_NOT_OK;
}
cparser_result_t cparser_cmd_iflo_no_ip_address_dhcp(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented/Supported\n");
  	return CPARSER_OK;
}

cparser_result_t cparser_cmd_iflo_exit(cparser_context_t *context)
{
	if (!exit_mode ())
	{
		return cparser_submode_exit (context->parser);
	}
	return CPARSER_NOT_OK;
}
