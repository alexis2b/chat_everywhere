/* restrict_command.c
 * copyright(c)2003-2004, A. de Bernis, <alexis@bernis.org>
 * 
 * This file is part of Chat Everywhere.
 *
 * Chat Everywhere is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Chat Everywhere is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chat Everywhere; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* strchr */

#include "i18n.h"

#include "server_config.h"
#include "restrict_command.h"
#include "commands.h"
#include "config_file.h"
#include "str_utils.h"
#include "mem_utils.h"

/*
#include "chat_server.h"
#include "communication.h"
#include "logging_support.h"
#include "fd_utils.h"
#include "net_utils.h"
#include "auto_commands.h"
#include "statistics.h"
#include "ping_timeout.h"
#include "password_auth.h"
#include "users_list_file.h"
*/



/* Internal defines */


/* Set some global variables */
static unsigned int restrictions[COM_MAX_ID + 1] = { 0 };


/* Idem for static functions */
static void parse_restrictions_config(const char *);

/* restrict_command_init
 * to be called when we need to (re)read the configuration file
 */
void restrict_command_init()
{
	unsigned int i;

	/* Initialize the array : all commands are allowed */
	for(i = 0; i <= COM_MAX_ID; i++)
		restrictions[i] = 0;

	/* Put the default restrictions */
	restrictions[COM_BAN] = DEFAULT_BAN_RES;
	restrictions[COM_GAG] = DEFAULT_GAG_RES;
	restrictions[COM_USERSINFO] = DEFAULT_USERSINFO_RES;

	/* Parse the config file directive */
	parse_restrictions_config(get_config_value("RestrictCommand"));
}


/* parse_restrictions_config
 * modify the restrictions table according to the user choices
 */
static void parse_restrictions_config(const char *value)
{
	int i, n;
	char **user_res;
	
	/* split the value */
	user_res = tokenize(value, ",", &n);
	
	/* Analyze each of the restrictions */
	for(i = 0; i < n; i++) {
		char *cfg, *cmd;
		int min_level = MAX_USER_LEVEL + 1;
		unsigned int cmd_code;

		cfg = str_clean(user_res[i]);

		/* Is there a minimum level ? */
		if(strchr(cfg, ':') != NULL) {
			char *tmp = cut_from(cfg, ":");

			cmd = cut_to(cfg, ":");
			min_level = atoi(tmp);
			efree(tmp);
		} else {
			cmd = estrdup(cfg);
		}
		efree(cfg);
		
		/* Get the command code */
		cmd_code = whatis_command(str_upper(cmd));

		if(cmd_code == COM_UNKNOWN) {
			fprintf(stderr,
			 _("Error : unknown command %s in the RestrictCommand directive\n"),
			 cmd);
		} else {
			restrictions[cmd_code] = min_level;
		}
	}

	free_string_array(user_res, n);
}

/* get_cmd_min_level
 * returns the minimum level required to run the command
 * given by the CMD code cmd
 */
extern unsigned int get_cmd_min_level(const unsigned int cmd)
{
	if(cmd > COM_MAX_ID)
		return 0;
	
	return restrictions[cmd];
}
