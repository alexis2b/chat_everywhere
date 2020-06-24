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


	
int dispatch_setlevel_message(struct chat_client* from_client, char* command)
{
	char **com_parsed;
	int d_level,a_level;
	struct chat_client *the_client;
	struct user_stats *the_stats;
	

	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(3 * sizeof(char *));
	
	if(parse_chat_message(command, com_parsed, 1) == 1) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /setlevel [nick] level");
		free_string_array(com_parsed, 1);
		return -1;
	}
	
	if(parse_chat_message(command, com_parsed, 2) == 2) {
		d_level = atoi(com_parsed[1]);
		
		if(from_client->level==d_level) {
			// MSG_TO_CLIENT
			send_generic_to(from_client,SEND_MSG, "You are already at level %d", d_level);
			free_string_array(com_parsed, 2);
			return 1;
		}

		if(from_client->level < d_level) {
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_ERROR, "You can't get a higher level than you have !");
			send_generic_to_level(9, SEND_MSG, "%s tried to access level %d", from_client->nick,d_level);
			log(_("%s tried to access level %d"), from_client->nick, d_level);
			free_string_array(com_parsed, 2);
			return -1;
		}

		if(d_level < MIN_USER_LEVEL || d_level > MAX_USER_LEVEL) {
			send_generic_to(from_client, SEND_ERROR, "Level must be between %d and %d.", MIN_USER_LEVEL, MAX_USER_LEVEL);
			free_string_array(com_parsed, 2);
			return -1;
		}

		a_level = from_client->level;
		from_client->level = d_level;
		// MSG_TO_CLIENT
		send_generic_to_level(a_level, SEND_MSG, "%s downed himself to level %d", from_client->nick, d_level);
		log(_("%s downed himself to level %d"), from_client->nick, d_level);

		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "You are now at level %d", d_level);
		free_string_array(com_parsed, 2);
		return 1;
	}
	
	parse_chat_message(command,com_parsed,3);
	
	if((the_client = get_client_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Can't find %s !", com_parsed[1]);
		free_string_array(com_parsed, 3);
		return -1;
	}

	d_level=atoi(com_parsed[2]);

	if(d_level < MIN_USER_LEVEL || d_level > MAX_USER_LEVEL) {
		send_generic_to(from_client, SEND_ERROR, "Level must be between %d and %d.", MIN_USER_LEVEL, MAX_USER_LEVEL);
		free_string_array(com_parsed, 3);
		return -1;
	}

	if(the_client->level >= from_client->level) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "The user has a greater or equal level than you !");
		free_string_array(com_parsed, 3);
		return -1;
	}
	
	if(d_level > from_client->level) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "You can't set anybody higher than you are !");
		free_string_array(com_parsed, 3);
		return -1;
	}
	
	log(_("%s has been increased to level %d by %s"), the_client->nick, d_level, from_client->nick);
	// MSG_TO_CLIENT
	send_generic_to_level(d_level, SEND_MSG, "%s has been increased to level %d by %s", the_client->nick, d_level, from_client->nick);
	send_generic_to(the_client, SEND_MSG, "%s increased you to level %d !", from_client->nick, d_level);
	the_client->level = d_level;

	/* Check the stats */
	if((the_stats = get_user_stats(the_client)) != NULL) {
		if(the_client->level > the_stats->max_level)
			the_stats->max_level = the_client->level;
	}

	free_string_array(com_parsed, 3);
	return 1;
}
