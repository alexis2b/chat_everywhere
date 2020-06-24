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
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"
#include "logging_support.h"

int dispatch_ignore_message(client_t *from_client, char *command)
{
	char **com_parsed;
	int cur = 0;
	client_t *the_client;
	char *nick;
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));

	if(parse_chat_message(command, com_parsed, 1) == 1) {
		/* user whish to list its ignored nicks */
		int n = 0;
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "List of ignored nicks :");
		
		while(from_client->ignored[n]) {
			send_generic_to(from_client, SEND_MSG, "%s", from_client->ignored[n]);
			n++;
		}

		free_string_array(com_parsed, 1);
		return 1;
	}

	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /ignore [Nick]");

		efree(com_parsed);
		return 0;
	}

	/* Check to see if not already ignored... */
	cur = 0;
	while(from_client->ignored[cur]) {
		if(!strcasecmp(com_parsed[1], from_client->ignored[cur])) {
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_ERROR, "%s is already ignored ! use /unignore to remove it.", from_client->ignored[cur]);			
			free_string_array(com_parsed, 2);
			return -1;
		}
		cur++;
	}

	/* Ok, add it to the list */
	if((the_client = get_client_by_nick(com_parsed[1])) != NULL)
		nick = estrdup(the_client->nick);
	else
		nick = estrdup(com_parsed[1]);

	cur = 0;
	while(from_client->ignored[cur++]);
	
	from_client->ignored = erealloc(from_client->ignored, (cur + 1) * sizeof(char *));
	from_client->ignored[cur--] = NULL;
	from_client->ignored[cur] = nick;

	// MSG_TO_CLIENT
	send_generic_to(from_client, SEND_MSG, "%s is now ignored", nick);
	log(_("%s now ignore %s"), from_client->nick, nick);

	free_string_array(com_parsed, 2);

	return cur;
}
