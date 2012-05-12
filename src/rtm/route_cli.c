#include "common_types.h"
#include "cli.h"
#include "cparser.h"
#include "cparser_tree.h"

cparser_result_t cparser_cmd_show_ip_route(cparser_context_t *context)
{
	if (!cli_show_ip_route ())
                return CPARSER_OK;
        return CPARSER_NOT_OK;
}

cparser_result_t cparser_cmd_config_ip_default_gateway_addr(cparser_context_t *context, uint32_t *addr_ptr)
{
	char  prefix_str[32];
	char gateway[32];

	convert_uint32_str_ip (gateway, *addr_ptr);

	convert_uint32_str_ip_mask(prefix_str, 0, 0);

	if (!zebra_static_ipv4(1,prefix_str,NULL, gateway,NULL,NULL))
		return CPARSER_OK;
	cli_printf ("Couldn't find any vaild interface\n");
	return CPARSER_NOT_OK;
}

cparser_result_t cparser_cmd_config_ip_route_network_mask_gateway(cparser_context_t *context,
    uint32_t *network_ptr,
    uint32_t *mask_ptr,
    uint32_t *gateway_ptr)
{
	uint8_t addr[4];
	char gateway[32];
	char  prefix_str[32];

	convert_uint32_str_ip_mask(prefix_str,*network_ptr,*mask_ptr);
	convert_uint32_str_ip (gateway, *gateway_ptr);

	if (!zebra_static_ipv4(1,prefix_str,NULL,gateway,NULL,NULL))
		return CPARSER_OK;
	cli_printf ("No vaild interface\n");
	return CPARSER_NOT_OK;
}
