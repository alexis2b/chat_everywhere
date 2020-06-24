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
/* Command file for chat_server */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "command_gag.h"
#include "chat_server.h"
#include "str_utils.h"
#include "communication.h"
#include "logging_support.h"	/* log_to_talk */

int dispatch_action_message(struct chat_client* from_client, char* command)
{
	struct chat_client* tmp_client = first_client;
	char **com_parsed = emalloc(3 * sizeof(char *));
	char action_txt[BUF_SIZE + 1] = "";
	char msg[BUF_SIZE + 1];
	int r = 0;

	/* the client must be logged */
	if(!check_client_is_logged(from_client))
		return -1;

	/* Get the message */
	if(parse_chat_message(command, com_parsed, 3) == 3) {
		snprintf(action_txt, BUF_SIZE, "%s %s", com_parsed[1], com_parsed[2]);
		free_string_array(com_parsed, 3);
	} else if(parse_chat_message(command, com_parsed, 2) == 2) {
		strncpy(action_txt, com_parsed[1], BUF_SIZE);
		free_string_array(com_parsed, 2);
	} else {
		efree(com_parsed);
	}

	/* log the action to the talk log */
	snprintf(msg, BUF_SIZE, "* %s %s", from_client->nick, action_txt);
	log_to_talk(msg);

	/* Prepare the message to be sent */
	snprintf(msg, BUF_SIZE, "%s <|> %s", from_client->nick, action_txt);

	/* if the user is gagged he is the only one to see its talks */
	if(is_gagged(from_client)) {
		send_generic_to(from_client, SEND_ACTION, msg);
		return 1;
	}
	
	while(tmp_client) {
		int n = 0;
		short ignore = 0;
		
		if(tmp_client->login_complete) {
			while(tmp_client->ignored[n]) {
				if(!strcasecmp(tmp_client->ignored[n++], from_client->nick)) {
					ignore = 1;
					break;
				}
			}

			if(ignore == 1) {
				tmp_client=tmp_client->next;
				continue;
			}
		
			send_generic_to(tmp_client, SEND_ACTION, msg);
			r++;
		}
		tmp_client = tmp_client->next;
	}

	/*
	* The returned value is the number of people who received this
	* message
	*/
	return r;
}
