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

int set_sntp_server (char *host);

cparser_result_t cparser_cmd_show_ntp_associations(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_show_ntp_status(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_show_clock(cparser_context_t *context UNUSED_PARAM)
{
	char buf[64];                                                                   
	struct timeval tv;                                                                 
	time_t curtime;                                                                    

	gettimeofday(&tv, NULL);                                                           
	curtime = tv.tv_sec;                                                                 
	strftime (buf, sizeof(buf), "%a %b %e %H:%M:%S %Z %Y", localtime(&curtime));                           
	printf("%s\n", buf);                                               
	return CPARSER_OK;          
}
cparser_result_t cparser_cmd_config_ntp_server_hostname_servername(cparser_context_t *context UNUSED_PARAM,
 							   char **servername_ptr)
{
	*servername_ptr = *servername_ptr;
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_ntp_server_serveripaddr(cparser_context_t *context UNUSED_PARAM,
 							   uint32_t *serveripaddr_ptr)
{
        uint8_t  addr[4];
        char  str[10];

        uint32_2_ipstring (ntohl(*serveripaddr_ptr), addr);

        sprintf (str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

	if (!set_sntp_server (str))
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_ntp_source_portnum(cparser_context_t *context UNUSED_PARAM,
 							int32_t *portnum_ptr UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_ntp_update_calendar(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_clock_timezone_zone_hours_mintues(cparser_context_t *context UNUSED_PARAM,
 								      char **zone_ptr UNUSED_PARAM,
								      int32_t *hours_ptr UNUSED_PARAM,
								       int32_t *mintues_ptr UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_clock_set_hours_mintues_secs_date_month_year(cparser_context_t *context UNUSED_PARAM,
 										 int32_t *hours_ptr UNUSED_PARAM,
 										 int32_t *mintues_ptr UNUSED_PARAM,
										 int32_t *secs_ptr UNUSED_PARAM,
										 int32_t *date_ptr UNUSED_PARAM,
										 int32_t *month_ptr UNUSED_PARAM,
										 int32_t *year_ptr UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_ntp_client_enable(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_ntp_client_disable(cparser_context_t *context UNUSED_PARAM)
{
	cli_printf ("Not Implemented\n");
	return CPARSER_OK;
}
