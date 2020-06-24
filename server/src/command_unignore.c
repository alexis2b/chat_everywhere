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

int dispatch_unignore_message(struct chat_client* from_client, char* command)
{
	char** com_parsed;
	int n=0;
	int last=0;
	struct chat_client* the_client;
	char nick[BUF_SIZE + 1];
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));

	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /unignore Nick");

		efree(com_parsed);
		return 0;
	}
	
	if((the_client = get_client_by_nick(com_parsed[1])) != NULL)
		strcpy(nick, the_client->nick);
	else
		strcpy(nick, com_parsed[1]);

	n = 0;
	while(from_client->ignored[n] != NULL) {
		if(!strcasecmp(nick, from_client->ignored[n]))
			break;
		n++;
	}

	if(from_client->ignored[n] == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "%s is not on your ignore list!", nick);
		
		free_string_array(com_parsed, 2);
		return -1;
	}

	last = 0;
	while(from_client->ignored[last] != NULL)
		last++;

	from_client->ignored[n] = from_client->ignored[last - 1];
	from_client->ignored[last - 1] = NULL;
	from_client->ignored = erealloc(from_client->ignored, last * sizeof(char *));

	log(_("%s does not ignore %s anymore"), from_client->nick, nick);
	// MSG_TO_CLIENT
	send_generic_to(from_client, SEND_MSG, "%s successfully removed from your ignore list!", nick);

	free_string_array(com_parsed, 2);
	return 1;
}
