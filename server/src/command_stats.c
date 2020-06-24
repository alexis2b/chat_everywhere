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

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "str_utils.h"
#include "commands.h"
#include "chat_server.h"
#include "communication.h"
#include "logging_support.h"
#include "config_file.h"
#include "statistics.h"

/*
 * /stats [nick]
 * Returns the nick stats or the server stats if no nick is specified
 */
int dispatch_stats_message(struct chat_client *from_client, char *command)
{
	char **com_parsed;
	struct user_stats *current_stats = stats->first_user_stats;
	unsigned int n = 0;
	int min_level;
	struct chat_client *the_client;
	unsigned long time_on_chat;
	
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));

	if(parse_chat_message(command, com_parsed, 1) == 1) {
		/* The user wants to see the server stats */
		min_level = atoi(get_config_value("ServerStatsMinLevel"));
		if(min_level < 0 || min_level > 9)
			min_level = 9;

		if(from_client->level < min_level) {
			log(_("%s tried to view the server stats with a bad level (%d < %d)"), from_client->nick, from_client->level, min_level);
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_ERROR, "You need at least level %d to use this command!", min_level);
			free_string_array(com_parsed, 1);
			return -1;
		}

		/* Let's show him */
		if(!use_global_stats) {
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_ERROR, "The server admin has not activated server statistics");
			free_string_array(com_parsed, 1);
			return -1;
		}			
		
		log(_("%s requested server stats successfully."), from_client->nick);
		send_generic_to(from_client, SEND_MSG, "Server statistics");
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "-   Version     : %s", CHAT_SERVER_VERSION);		
		send_generic_to(from_client, SEND_MSG, "-   Launch date : %s", cut_last_char(ctime(&(stats->launch_time))));
		send_generic_to(from_client, SEND_MSG, "-   Uptime      : %s", seconds2ascii(time(NULL) - stats->launch_time));
		send_generic_to(from_client, SEND_MSG, "-   Kicks       : %d",     stats->kick_number);
		send_generic_to(from_client, SEND_MSG, "-   Max users   : %d", stats->max_users);
		
		free_string_array(com_parsed, 1);
		return 0;
	}
	
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /stats [Nick]");
		efree(com_parsed);
		return -1;
	}

	/*
	* The user has requested user stats : check permission and then display them
	*/

	min_level = atoi(get_config_value("UserStatsMinLevel"));
	if(min_level < 0 || min_level > 9)
		min_level = 9;

	/* The user may check his own stats, but need a certain level to view other's ones */
	if(from_client->level < min_level && strcasecmp(from_client->nick, com_parsed[1])) {
		log(_("%s tried to view the user stats of %s with a bad level (%d < %d)"), from_client->nick, com_parsed[1], from_client->level, min_level);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "You need at least level %d to use this command!", min_level);
		efree(com_parsed);
		return(-1);
	}

	/* Try to see if the client is still here */
	the_client = get_client_by_nick(com_parsed[1]);

	while(current_stats) {
		if(!strcasecmp(current_stats->user_nick, com_parsed[1])) {
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_MSG, "User statistics for %s", current_stats->user_nick);
			if(the_client != NULL) {
				send_generic_to(from_client, SEND_MSG, "-   Current level : %d", the_client->level);
				send_generic_to(from_client, SEND_MSG, "-   Lag           : %u seconds", the_client->last_lag);
			}
			if(the_client->user_agent)
				send_generic_to(from_client, SEND_MSG, "-   User agent    : %s",   the_client->user_agent);
			if(the_client->referer)
				send_generic_to(from_client, SEND_MSG, "-   Referer       : %s",   the_client->referer);
			if(the_client->user_lang)
				send_generic_to(from_client, SEND_MSG, "-   User language : %s",   the_client->user_lang);
			send_generic_to(from_client, SEND_MSG, "-   Kick number   : %d",   current_stats->kick_number);
			send_generic_to(from_client, SEND_MSG, "-   Login number  : %ld", current_stats->login_number);
			send_generic_to(from_client, SEND_MSG, "-   Max level     : %d",   current_stats->max_level);
			send_generic_to(from_client, SEND_MSG, "-   Last login    : %s",  cut_last_char(ctime(&(current_stats->last_login))));
			
			time_on_chat = current_stats->spent_time;
			if(current_stats->last_login != current_stats->last_quit) {
				send_generic_to(from_client, SEND_MSG, "-   Last logout   : %s", cut_last_char(ctime(&(current_stats->last_quit))));
			} else {
				time_on_chat += time(NULL) - current_stats->last_login;
				send_generic_to(from_client, SEND_MSG, "-   User is still here");
			}
			send_generic_to(from_client, SEND_MSG, "-   Time on chat  : %s",  seconds2ascii(time_on_chat));

			n++;
		}

		current_stats = current_stats->next;
	}

	if(!n) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "No stats for %s.", com_parsed[1]);
	}

	free_string_array(com_parsed, 2);
	return n;
}
