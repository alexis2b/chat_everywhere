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


int dispatch_quit_message(struct chat_client* from_client, char* command)
{
	char **com_parsed;
	char byemsg[BUF_SIZE + 1] = "No message";

	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(3 * sizeof(char *));

	/* Get the quit message if there is one */
	if(parse_chat_message(command, com_parsed, 3) == 3) {
		snprintf(byemsg, BUF_SIZE, "%s %s", com_parsed[1], com_parsed[2]);
		free_string_array(com_parsed, 3);
	} else if(parse_chat_message(command, com_parsed, 2) == 2) {
		strncpy(byemsg, com_parsed[1], BUF_SIZE);
		free_string_array(com_parsed, 2);
	} else {
		efree(com_parsed);
	}

	// MSG_TO_CLIENT
	send_generic_to(from_client, SEND_MSG, "Quitting. Reload the page in your browser to reconnect.");
	user_deconnect(from_client, byemsg);
	return 1;
}
