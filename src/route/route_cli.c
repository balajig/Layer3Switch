#include "common_types.h"
#include "cli.h"
#include "route.h"
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
	uint8_t addr[4];
	uint32_2_ipstring (ntohl(*addr_ptr), &addr);

	if (!route_add_gateway ("0.0.0.0", 0, addr))
		return CPARSER_OK;
	printf ("Couldn't find any vaild interface\n");
	return CPARSER_NOT_OK;
}

cparser_result_t cparser_cmd_config_ip_route_network_mask_gateway(cparser_context_t *context,
    uint32_t *network_ptr,
    uint32_t *mask_ptr,
    uint32_t *gateway_ptr)
{
	uint8_t addr[4];
	uint8_t gateway[4];

	uint32_2_ipstring (ntohl(*network_ptr), &addr);
	uint32_2_ipstring (ntohl(*gateway_ptr), &gateway);

	if (!route_add_gateway (addr, u32ip_masklen (ntohl(*mask_ptr)), gateway))
		return CPARSER_OK;

	printf ("No vaild interface\n");
	return CPARSER_NOT_OK;
}
