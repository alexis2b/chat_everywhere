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
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"

int dispatch_msg_message(client_t *from_client, char *command)
{
	char **com_parsed;
	client_t *to_client;
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(3*sizeof(char*));

	if(parse_chat_message(command, com_parsed, 3) != 3)	{
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /msg Nick_or_level  txt");
		efree(com_parsed);
		return -1;
	}

	/*
	* Check whether we must send this message to a level
	* or to  nick.
	*/

	if(strlen(com_parsed[1]) == 1 && *com_parsed[1] >= '1' && *com_parsed[1] <= '9') {
		/* Level specific message */
		int level = atoi(com_parsed[1]);
		/* or : int level = (int) (*com_parsed - '0'); */

		send_generic_to_level(level, SEND_PMSG, "%d (%s) <|> %s", level,
		 from_client->nick, com_parsed[2]);

		free_string_array(com_parsed, 3);
		return 1;
	}

	/* Normal private message */
	if((to_client = get_client_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Unknow nick : %s", com_parsed[1]);
		free_string_array(com_parsed, 3);
		return -1;
	}
	
	send_generic_to(to_client, SEND_PMSG, "%s <|> %s", from_client->nick,
	 com_parsed[2]);
	free_string_array(com_parsed, 3);

	return 1;
}	
