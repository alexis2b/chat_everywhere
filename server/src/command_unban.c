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


int dispatch_unban_message(struct chat_client* from_client, char* command)
{
	char** com_parsed;
	struct banishment* the_ban;
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));
	
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /unban Nick");
		
		efree(com_parsed);
		return -1;
	}
	
	if((the_ban = get_ban_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "%s hasn't been banned !", com_parsed[1]);
		
		free_string_array(com_parsed, 2);
		return -2;
	}
	
	if(from_client->level < the_ban->banner_level) {
		log(_("%s tried to unban %s"), from_client->nick, the_ban->banned_nick);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Can't unban %s, permission denied !", the_ban->banned_nick);
		
		free_string_array(com_parsed, 2);
		return -3;
	}
	
	log(_("%s has been successfully unbanned by %s"), the_ban->banned_nick, from_client->nick);
	// MSG_TO_CLIENT
	send_generic_to_level(from_client->level, SEND_MSG, "%s has been successfully unbanned by %s",
	 the_ban->banned_nick,from_client->nick);
	
	if(the_ban->prev) {
		the_ban->prev->next = the_ban->next;
		
		if(the_ban->next)
			the_ban->next->prev = the_ban->prev;
	} else {
		first_ban = the_ban->next;
			
		if(the_ban->next)
			the_ban->next->prev = NULL;
	}

	efree(the_ban->banned_nick);
	efree(the_ban->banner_nick);
	efree(the_ban->banned_address);
	efree(the_ban);
	
	
	free_string_array(com_parsed, 2);
	return 1;
}
