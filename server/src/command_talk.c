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

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "command_gag.h"
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"
#include "logging_support.h"


int dispatch_talk_message(struct chat_client* from_client, char* command)
{
	client_t *tmp_client = first_client;
	client_t *next_client;
	char** com_parsed;
	char msg[BUF_SIZE + 1];
	int r = 0;
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));
	
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		log(_("Invalid TALK message : %s"), command);
		efree(com_parsed);
		return -1;
	}

	/* log the text */
	snprintf(msg, BUF_SIZE, "<%s> %s", from_client->nick, com_parsed[1]);
	log_to_talk(msg);

	/* Prepare the message to be sent */
	snprintf(msg, BUF_SIZE, "%s <|> %s", from_client->nick, com_parsed[1]);

	/* if the user is gagged he is the only one to see its talks */
	if(is_gagged(from_client)) {
		send_generic_to(from_client, SEND_TALK, msg);
		free_string_array(com_parsed, 2);
		return 1;
	}
	
	/* Send it to everybody */
	while(tmp_client) {
		int n = 0;
		short ignore = 0;
		next_client = tmp_client->next;
		
		if(tmp_client->login_complete) {
			while(tmp_client->ignored[n]) {
				if(!strcasecmp(tmp_client->ignored[n++], from_client->nick)) {
					/* printf("%s is ignored by %s, passing through\n",from_client->nick,tmp_client->nick); */
					ignore = 1;
					break;
				}
			}
		
			if(ignore == 1) {
				tmp_client = next_client;
				continue;
			}
		
			send_generic_to(tmp_client, SEND_TALK, msg);
			r++;
		}
		tmp_client = next_client;
	}

	free_string_array(com_parsed, 2);

	/*
	* The returned value is the number of people we sent this
	* message to
	*/
	return r;
}
