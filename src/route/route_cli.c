#include "common_types.h"
#include "cli.h"
#include "ifmgmt.h"
#include "cparser.h"
#include "cparser_tree.h"

cparser_result_t cparser_cmd_show_ip_route(cparser_context_t *context)
{
	if (!cli_show_ip_route ())
                return CPARSER_OK;
        return CPARSER_NOT_OK;
}
