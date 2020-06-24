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
/* Command file for /BAN */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"
#include "logging_support.h"
#include "config_file.h"
#include "net_utils.h"
#include "statistics.h"
#include "scheduler.h"


/* global variables located here */
struct banishment* first_ban;   /* node of various linked */


/* internal functions */
static void send_ban_list(client_t *);



int dispatch_ban_message(client_t *from_client, char *command)
{
	char **com_parsed;
	struct banishment *tmp_ban=first_ban;
	client_t *tmp_client;
	unsigned long expires_at = 0;

	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(3 * sizeof(char *));

	/* No argument -> the user whishes to see the ban list */
	if(parse_chat_message(command, com_parsed, 1) == 1) {
		send_ban_list(from_client);
		free_string_array(com_parsed, 1);
		return 0;
	}

	/* More than two arguments -> error */
	if(parse_chat_message(command, com_parsed, 3) == 3) {
		/* expiration time is in minutes from now */
		expires_at = atoi(com_parsed[2])*60 + time(NULL);
		efree(com_parsed[2]);
	} else if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /ban [nick [expiration]]");
		efree(com_parsed);
		
		return -2;
	}

	if((tmp_client = get_client_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "can't find %s.",
		 com_parsed[1]);
		free_string_array(com_parsed, 2);

		return -3;
	}

	if(from_client->level <= tmp_client->level) {
		log(_("%s tried to ban %s with an inferior or equal level"),
		 from_client->nick, tmp_client->nick);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR,
		 "can't ban %s: permission denied", tmp_client->nick);
		free_string_array(com_parsed, 2);
		
		return -4;
	}

	tmp_ban = first_ban;
	while(tmp_ban) {
		if(!strcasecmp(tmp_ban->banned_nick, tmp_client->nick)) {
			if(tmp_ban->banner_level < from_client->level) {
				// MSG_TO_CLIENT
				send_generic_to(from_client, SEND_MSG,
				 "%s was previously banned by %s, just modifying level and nick.",
				 tmp_ban->banned_nick, tmp_ban->banner_nick);

				log(_("%s upgrades ban of %s to level %d"), from_client->nick,
				 tmp_ban->banned_nick, from_client->level);
				// MSG_TO_CLIENT
				send_generic_to_level(from_client->level, SEND_MSG,
				 "%s upgrades ban of %s to level %d.",
				 from_client->nick, tmp_ban->banned_nick, from_client->level);

				tmp_ban->banner_level = from_client->level;
				tmp_ban->banner_nick = from_client->nick;

				free_string_array(com_parsed, 2);
				
				return 2;
			}
			
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_ERROR, "%s is already banned !",
			 tmp_ban->banned_nick);
			free_string_array(com_parsed, 2);
			
			return 2;
		}
		
		tmp_ban = tmp_ban->next;
	}

	tmp_ban = emalloc(sizeof(*tmp_ban));
	tmp_ban->prev = NULL;
	tmp_ban->next = first_ban;
	tmp_ban->banner_nick = estrdup(from_client->nick);
	tmp_ban->banned_nick = estrdup(tmp_client->nick);
	tmp_ban->banner_level = from_client->level;
	tmp_ban->banned_address = emalloc(sizeof(struct sockaddr_in));
	memcpy(tmp_ban->banned_address, tmp_client->address,
	 sizeof(struct sockaddr_in));
	tmp_ban->expiration = expires_at;
	
	if(first_ban)
		first_ban->prev = tmp_ban;

	first_ban = tmp_ban;

	log(_("%s has been banned by %s (%d)"), tmp_ban->banned_nick,
	 tmp_ban->banner_nick, tmp_ban->banner_level);
	// MSG_TO_CLIENT
	send_generic_to_level(tmp_ban->banner_level, SEND_MSG,
	 "%s has been banned by %s(%d) !",
	 tmp_ban->banned_nick, tmp_ban->banner_nick, tmp_ban->banner_level);

	free_string_array(com_parsed, 2);

	return 1;
}


/* send_ban_list
 * sends the list of actual bans to the client
 * static void send_ban_list(client_t *client)
 * we just display those that he can unban (level <= than his)
 */
static void send_ban_list(client_t *client)
{
	unsigned int ban_num = 0;
	ban_t *tmp_ban = first_ban;

	while(tmp_ban != NULL) {

		if(tmp_ban->banner_level <= client->level) {
			char expires[BUF_SIZE] = "never";
			ban_num++;

			if(ban_num == 1) {  /* first one, we display the headers */
				// MSG_TO_CLIENT
				send_generic_to(client, SEND_MSG, "User           From            Banned by      Level Expires");
				send_generic_to(client, SEND_MSG, "-----------------------------------------------------------");
			}

			if(tmp_ban->expiration != 0)
				sprintf(expires, "%ldmin", (tmp_ban->expiration - time(NULL)) / 60);

			// MSG_TO_CLIENT
			send_generic_to(client, SEND_MSG, "%-14s %-15s %-14s   %d   %7s",
			 tmp_ban->banned_nick, sin_addr2ip_char(tmp_ban->banned_address),
			 tmp_ban->banner_nick, tmp_ban->banner_level, expires);
		}

		tmp_ban = tmp_ban->next;
	}

	if(ban_num == 0)
		send_generic_to(client, SEND_MSG, "Nobody has been banned yet");
	else
		send_generic_to(client, SEND_MSG, "-----------------------------------------------------------");
}



/* ban_timer
 * this function is called by the scheduler every minute
 * we see if a ban has expired and we remove it if it is the case
 */
void ban_timer(void *data)
{
	ban_t *cur = first_ban;
	ban_t *next;
	unsigned long now = time(NULL);
	
	while(cur != NULL) {
		next = cur->next;

		/* Is the ban expires ? */
		if(cur->expiration == 0 || cur->expiration > now) {
			cur = next;
			continue;
		}

		/* the ban has expired : let's remove it */
		log(_("The ban of %s is over"), cur->banned_nick);
		// MSG_TO_CLIENT
		send_generic_to_level(cur->banner_level, SEND_MSG,
		 "%s ban is over", cur->banned_nick);

		if(cur->prev) {
			cur->prev->next = cur->next;
        
			if(cur->next)
				cur->next->prev = cur->prev;
		} else {
	        first_ban = cur->next;
			
			if(cur->next)
				cur->next->prev = NULL;
		}

		efree(cur->banned_nick);
		efree(cur->banner_nick);
		efree(cur->banned_address);
		efree(cur);

		/* iterates */
		cur = next;
	}
}
