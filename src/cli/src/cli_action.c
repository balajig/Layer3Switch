#include <stdio.h>
#include "cparser.h"
#include "cparser_token.h"
#include "cparser_tree.h"
#include "cli.h"

static cparser_result_t cparser_cmd_enter_privileged_mode (cparser_t *parser, char *buf, int buf_size);


cparser_result_t cparser_cmd_lock(cparser_context_t *context)
{
	if (process_lock ())
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_logout(cparser_context_t *context)
{
	if (process_logout ())
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_show_users(cparser_context_t *context)
{
	if (show_users ())
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_show_mem_pool(cparser_context_t *context)
{
        if (show_mem_pool ())
                return CPARSER_NOT_OK;
        return CPARSER_OK;
}

cparser_result_t cparser_cmd_show_task_cpu(cparser_context_t *context)
{
	if (show_cpu_usage ())
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_enable_privileged_mode(cparser_context_t *context)
{
	char passwd[100];
	int rc;

	if (cparser_is_in_privileged_mode(context->parser)) {
		printf ("Already in privileged mode.\n");
		return CPARSER_NOT_OK;
	}

	/* Request privileged mode password */
	rc = cparser_user_input(context->parser, "Enter password (Enter: 'HELLO'): ", 0,
			passwd, sizeof(passwd), cparser_cmd_enter_privileged_mode);
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_configure_terminal(cparser_context_t *context)
{
	char prompt[CPARSER_MAX_PROMPT];
	/* Enter the submode */
	set_prompt ("(config)");
	get_prompt (prompt);
	set_curr_mode (GLOBAL_CONFIG_MODE);
	return cparser_submode_enter(context->parser, NULL, prompt);
}
cparser_result_t cparser_cmd_config_user_add_username_password_level
                 (cparser_context_t *context, char **username_ptr, char **password_ptr, int32_t *level_ptr)

{
	if (create_user (*username_ptr, *password_ptr, *level_ptr))
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_config_user_del_username(cparser_context_t *context,
    char **username_ptr)
{
	if (user_del (*username_ptr))
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}
cparser_result_t cparser_cmd_end(cparser_context_t *context)
{

}
cparser_result_t cparser_cmd_exit(cparser_context_t *context)
{
	return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_disable_privileged_mode(cparser_context_t *context)
{
	if (!cparser_is_in_privileged_mode(context->parser)) {
		printf ("Not in privileged mode.\n");
		return CPARSER_NOT_OK;
	}

	cparser_set_privileged_mode(context->parser, 0);
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_config_end(cparser_context_t *context)
{
        set_prompt ("");
        set_curr_mode (USER_EXEC_MODE);
}

cparser_result_t cparser_cmd_config_exit(cparser_context_t *context)
{
	if (!exit_mode ())
	{
		return cparser_submode_exit (context->parser);
	}
	return CPARSER_NOT_OK;
}


cparser_result_t cparser_cmd_show_mac_address_table(cparser_context_t *context)
{
	if (!show_mac_table ())
		return CPARSER_OK;
	return CPARSER_NOT_OK;
}

static cparser_result_t
cparser_cmd_enter_privileged_mode (cparser_t *parser, char *buf, int buf_size)
{
    if (strncmp(buf, "HELLO", buf_size)) {
        printf("\nPassword incorrect. Should enter 'HELLO'.\n");
    } else {
        printf("\nEnter privileged mode.\n");
        cparser_set_privileged_mode(parser, 1);
    }
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_run_script_filename(cparser_context_t *context, char **filename_ptr)
{
	FILE  *fp = fopen  (*filename_ptr, "r");
	int    c = 0;
	int do_echo = 0;

	if (!fp) {
		printf ("Unable to open file :  %s\n", *filename_ptr);
		return CPARSER_NOT_OK;
	}

	/* Reset FSM states and advance to the next line */
	cparser_record_command(context->parser, CPARSER_OK);
	cparser_fsm_reset(context->parser);
	if (!cparser_is_user_input(context->parser, &do_echo)) {
		cparser_print_prompt(context->parser);
	}
	cparser_line_advance(context->parser);	
	while ((c = fgetc (fp)) != EOF) {
        	cparser_input (context->parser, c, CPARSER_CHAR_REGULAR);
	}
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_clear_screen (cparser_context_t *context)
{
	printf("\033[2J");
	printf("\033[0;0f");
	fflush (stdout);
	return CPARSER_OK;
}
