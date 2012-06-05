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

extern int telnet_to (char *host); 

cparser_result_t cparser_cmd_telnet_host(cparser_context_t *context, char **host_ptr)
{
	context = context;
	telnet_to (*host_ptr);
	return CPARSER_OK;
}
