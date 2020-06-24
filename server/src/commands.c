/* This file is part of the Chat Everywhere package
 * copyright(c)1999-2004, A. de Bernis, <alexis@bernis.org>
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
/* General utilities for commands, root of the command_* files */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "i18n.h"

#include "server_config.h"
#include "mem_utils.h"
#include "str_utils.h"
#include "logging_support.h"
#include "commands.h"
#include "chat_server.h"
#include "communication.h"
#include "logging_support.h"
#include "ping_timeout.h"		/* for the PONG command */
#include "restrict_command.h"    /* for get_cmd_min_level */
#include "scheduler.h"

/* commands_init
 * some tasks have to be performed before any command can be accepted
 *   - set a timer on ban_timer()
 *   - set a timer on gag_timer()
 */
void commands_init()
{
	/* Check for expired bans regularly */
	register_alarm(TASK_BAN_TIMER, BAN_TIMER_DELAY, ban_timer, NULL, 1);
	
	/* Check for expired gags regularly */
	register_alarm(TASK_GAG_TIMER, GAG_TIMER_DELAY, gag_timer, NULL, 1);
}


int whatis_command(char* command)
{
	if(!strcmp(command, "TALK"))
		return(COM_TALK);
	else if(!strcmp(command, "PONG"))
		return COM_PONG;
	else if(!strcmp(command, "IGNORE"))
		return(COM_IGNORE);
	else if(!strcmp(command, "LOGIN"))
		return(COM_LOGIN);
	else if(!strcmp(command, "USERS"))
		return(COM_USERS);
	else if(!strcmp(command, "OP"))
		return(COM_OP);
	else if(!strcmp(command, "USERSINFO"))
		return(COM_USERSINFO);
	else if(!strcmp(command, "MSG"))
		return(COM_MSG);
	else if(!strcmp(command, "SETLEVEL"))
		return(COM_SETLEVEL);
	else if(!strcmp(command, "KICK"))
		return(COM_KICK);
	else if(!strcmp(command, "UNIGNORE"))
		return(COM_UNIGNORE);
	else if(!strcmp(command, "BAN"))
		return(COM_BAN);
	else if(!strcmp(command, "UNBAN"))
		return(COM_UNBAN);
	else if(!strcmp(command, "HELP"))
		return(COM_HELP);
	else if(!strcmp(command, "SEEN"))
		return COM_SEEN;
	else if(!strcmp(command, "STATS"))
		return COM_STATS;
	else if(!strcmp(command, "QUIT"))
		return COM_QUIT;
	else if(!strcmp(command, "ME"))
		return COM_ME;
	else if(!strcmp(command, "AUTH"))
		return COM_AUTH;
	else if(!strcmp(command, "GAG"))
		return COM_GAG;
	else if(!strcmp(command, "UNGAG"))
		return COM_UNGAG;
	else if(!strcmp(command, "CAPABILITIES"))
		return COM_CAPS;
	else
		return(COM_UNKNOWN);
}


/* Analyze the given command (coming from 'from_client') and calls the
 * function to treat it
 * Some functions that do not modify global variables are called in a
 * forked child
 * Returns : -1 if there is an error
 */
int treat_command(char* command, struct chat_client *from_client)
{
	char *tmps;
	char *pos;
	unsigned int cmd_code;
	
	
	/* Parse the command */
	if((pos = strstr(command,MSG_SEPARATOR))==0)
		return -1;

	pos = (char*) (pos + sizeof(MSG_SEPARATOR) - 1);
	tmps = estrdup(pos);
	if((pos = strstr(tmps, MSG_SEPARATOR)) == 0)
		return -1;

	tmps[pos - tmps] = '\0';
	tmps = erealloc(tmps, strlen(tmps) + 1);
	
	/* Check if we can run the command */
	cmd_code = whatis_command(tmps);
	if(from_client->login_complete && from_client->level <
	 get_cmd_min_level(cmd_code)) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "command %s is restricted",
		 tmps);
		goto fin_func;
	}

	switch(cmd_code) {
		case COM_TALK:
			if(fork())
				break;
				
			dispatch_talk_message(from_client, command);
			exit(0);
			/* Never reached */

		case COM_PONG:
			dispatch_pong_message(from_client, command);
			break;

		case COM_ME:
			if(fork())
				break;
			
			dispatch_action_message(from_client, command);
			exit(0);
			/* Never reached */

		case COM_UNKNOWN:
			log(_("Parse error : unknown command %s"), tmps);
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_ERROR, "Command unknown : %s", tmps);
			break;
	
		case COM_IGNORE:
			dispatch_ignore_message(from_client,command);
			break;

		case COM_LOGIN:
			dispatch_login_message(from_client, command);
			break;
	
		case COM_USERS:
			if(fork())
				break;
				
			dispatch_users_message(from_client);
			exit(0);
			/* Never Reached */

		case COM_OP:
			dispatch_op_message(from_client,command);
			break;

		case COM_USERSINFO:
			dispatch_usersinfo_message(from_client);
			break;
			
		case COM_MSG:
			dispatch_msg_message(from_client,command);
			break;
			
		case COM_SETLEVEL:
			dispatch_setlevel_message(from_client,command);
			break;

		case COM_KICK:
			dispatch_kick_message(from_client,command);
			break;

		case COM_UNIGNORE:
			dispatch_unignore_message(from_client,command);
			break;

		case COM_BAN:
			dispatch_ban_message(from_client,command);
			break;
			
		case COM_UNBAN:
			dispatch_unban_message(from_client,command);
			break;

		case COM_HELP:
			if(fork())
				break;
			
			dispatch_help_message(from_client, command);
			exit(0);
			/* Never reached */

		case COM_SEEN:
			if(fork())
				break;
			
			dispatch_seen_message(from_client, command);
			exit(0);
			/* Never reached */
		
		case COM_STATS:
			if(fork())
				break;

			dispatch_stats_message(from_client, command);
			exit(0);
			/* Never reached */

		case COM_QUIT:
			dispatch_quit_message(from_client, command);
			break;

		case COM_AUTH:
			dispatch_auth_message(from_client, command);
			break;

		case COM_GAG:
			dispatch_gag_message(from_client, command);
			break;

		case COM_UNGAG:
			dispatch_ungag_message(from_client, command);
			break;
		
		case COM_CAPS:
			dispatch_capabilities_message(from_client, command);

		default:
			break;
	}

fin_func:
	efree(tmps);
	return 1;
}

/* check_client_is_logged
 * Verify that the client has gone through login and tells him if not
 */
int check_client_is_logged(client_t *the_client)
{
	if(the_client->login_complete)
		return 1;

	send_generic_to(the_client, SEND_ERROR, "You must be logged in to use this command!");
	return 0;
}
