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
#include "statistics.h"

int dispatch_kick_message(struct chat_client* from_client, char* command)
{
	char** com_parsed=emalloc(3*sizeof(char*));
	struct chat_client* the_client;
	char byemsg[BUF_SIZE + 1] = "No message";
	char msg[BUF_SIZE + 1];
	
	if(!check_client_is_logged(from_client))
		return -1;

	if(parse_chat_message(command, com_parsed, 3) == 3) {
		strcpy(byemsg, com_parsed[2]);
		efree(com_parsed[2]);
	} else {
		if(parse_chat_message(command, com_parsed, 2) != 2) {
			send_generic_to(from_client, SEND_MSG, "Usage : /kick Nick [message]");
			efree(com_parsed);
			return(-1);
		}
	}
	
	if((the_client = get_client_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "%s : no such nick !", com_parsed[1]);
		free_string_array(com_parsed, 2);
		return(-1);
	}
	
	if(the_client->level >= from_client->level) {
		log(_("%s tried to kick %s with an inferior level"), from_client->nick, the_client->nick);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "You can't kick a person higher or equal than you !");
		send_generic_to(the_client, SEND_ERROR, "%s tried to kick you !", from_client->nick);
		free_string_array(com_parsed, 2);
		return(-1);
	}
	
	/* Ok, proceed with the kick */

	log(_("%s has been kicked by %s (%s)"), the_client->nick, from_client->nick, byemsg);
	// MSG_TO_CLIENT
	send_generic_to(the_client, SEND_MSG, "You were kicked by %s (%s)", from_client->nick, byemsg);
	send_generic_to_level(from_client->level, SEND_MSG, "%s kicked %s (%s)", from_client->nick,the_client->nick,byemsg);

	/* Updates the stats */
	if(use_global_stats)
		stats->kick_number++;
	if(use_user_stats) {
		struct user_stats *the_stats = get_user_stats(the_client);
		
		if(the_stats)
			the_stats->kick_number++;
	}

	snprintf(msg, BUF_SIZE, "kicked by %s : %s", from_client->nick, byemsg);
	user_deconnect(the_client, msg);
	free_string_array(com_parsed, 2);
	return(1);
}
