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
#include <time.h>

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"
#include "statistics.h"

/*
 * /seen nick
 * Returns last time since nick has gone
 */
int dispatch_seen_message(struct chat_client *from_client, char *command)
{
	char **com_parsed;
	struct user_stats *current_stats = stats->first_user_stats;
	unsigned int n = 0;
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));
	
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /seen Nick");
		efree(com_parsed);
		return -1;
	}

	while(current_stats) {
		if(!strcasecmp(current_stats->user_nick, com_parsed[1])) {

			if(current_stats->last_login == current_stats->last_quit)
				// MSG_TO_CLIENT
				send_generic_to(from_client, SEND_MSG, "%s is still here!", current_stats->user_nick);
			else
				// MSG_TO_CLIENT
				send_generic_to(from_client, SEND_MSG, "%s has quit since %s",
				 current_stats->user_nick, seconds2ascii(time(NULL) - current_stats->last_quit));

			n++;
		}
	
		current_stats = current_stats->next;
	}

	if(!n)
		send_generic_to(from_client, SEND_ERROR, "No stats for %s", com_parsed[1]);
	
	free_string_array(com_parsed, 2);
	return n;
}
