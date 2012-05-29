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

cparser_result_t cparser_cmd_traceroute_hostaddr(cparser_context_t *context,
		uint32_t *hostaddr_ptr)
{
        uint8_t  addr[4];
        uint8_t  str[10];

        uint32_2_ipstring (ntohl(*hostaddr_ptr), addr);

        sprintf (str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

	do_traceroute (str);

	return CPARSER_OK;
}
cparser_result_t cparser_cmd_traceroute_hostname(cparser_context_t *context,
		char **hostname_ptr)
{
	do_traceroute (*hostname_ptr);

	return CPARSER_OK;
}
