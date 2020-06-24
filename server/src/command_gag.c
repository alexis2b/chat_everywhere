/* command_gag.c
 * copyright(c)2003-2004, A. de Bernis, <alexis@bernis.org>
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
/* Command file for /gag and /ungag : ignore annoying users */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> /* struct sockaddr_in */
#include <netinet/in.h> /* for FreeBSD */

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "chat_server.h"
#include "communication.h"
#include "logging_support.h"
#include "config_file.h"
#include "command_gag.h"


/* Structure definition is internal */
struct _gag
{
	struct _gag	       *next;

	char               *nick;       /* nick of the person gagged */
	struct sockaddr_in *address;    /* IP adress */


	char               *gagger;     /* operator who set up the gag */
	unsigned int        level;      /* level of the gag */

	unsigned long       expiration; /* expiration time of the gag (if != 0) */
};


/* Internal module variables */
static gag_t *first_gag = NULL;


/* internal functions */
static void   send_gag_list_to(client_t *from_client);
static void   remove_gag(gag_t *);
static gag_t *set_gag(client_t *, client_t *, unsigned long);


/* dispatch_gag_message
 * from_client requested a gag on somebody
 */
int dispatch_gag_message(client_t *from_client, char *command)
{
	char **com_parsed;
	client_t *tmp_client;
	unsigned long expires_at = 0;

	/* you need to be logged in to use this command */
	if(!check_client_is_logged(from_client))
		return -1;

	/* Max : 2 arguments */
	com_parsed = emalloc(3 * sizeof(char *));

	/* Used without argument : request gag list */
	if(parse_chat_message(command, com_parsed, 1) == 1) {
		send_gag_list_to(from_client);
		free_string_array(com_parsed, 1);
		return 0;
	}

	/* more than two arguments : error */
	if(parse_chat_message(command, com_parsed, 3) == 3) {
		/* expiration time is in minutes from now */
		expires_at = atoi(com_parsed[2])*60 + time(NULL);
		efree(com_parsed[2]);
	} else if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /gag [nick [expiration]]");
		efree(com_parsed);
		return -2;
	}
	
	/* Now there is exactly one argument */
	if((tmp_client = get_client_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "can't find %s.",
		 com_parsed[1]);
		free_string_array(com_parsed, 2);
		return -3;
	}

	if(from_client->level <= tmp_client->level) {
		log(_("%s(%d) tried to gag %s(%d) with an inferior or equal level"),
		 from_client->nick, from_client->level, tmp_client->nick, tmp_client->level);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR,
		 "can't gag %s: permission denied", tmp_client->nick);
		free_string_array(com_parsed, 2);
		return -4;
	}

	/* Check if the target has already been gagged */
	if(is_gagged(tmp_client)) {
		gag_t *gag = tmp_client->gag;

		if(gag->level < from_client->level) {
			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_MSG,
			 "%s was previously gagged by %s, just modifying level and nick.",
			 tmp_client->nick, gag->gagger);

			log(_("%s upgrades gag of %s to level %d"), from_client->nick,
			 tmp_client->nick, from_client->level);
			// MSG_TO_CLIENT
			send_generic_to_level(from_client->level, SEND_MSG,
			 "%s upgrades gag of %s to level %d.",
			 from_client->nick, tmp_client->nick, from_client->level);

			/* reset the gag to the new gagger / level */
			gag->level = from_client->level;
			efree(gag->gagger);
			gag->gagger = estrdup(from_client->nick);

			free_string_array(com_parsed, 2);
			return 2;
		}

		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "%s is already gagged !",
		 tmp_client->nick);
		free_string_array(com_parsed, 2);
		return 2;
	}


	/* Ok, we gag him */
	set_gag(from_client, tmp_client, expires_at);

	log(_("%s has been gagged by %s(%d)"), tmp_client->nick,
	 from_client->nick, from_client->level);
	// MSG_TO_CLIENT
	send_generic_to_level(from_client->level, SEND_MSG,
	 "%s has been gagged by %s(%d) !",
	 tmp_client->nick, from_client->nick, from_client->level);

	free_string_array(com_parsed, 2);
	return 1;
}


/* dispatch_ungag_message
 * Tries to "ungag" the selected client
 */
int dispatch_ungag_message(client_t *from_client, char *command)
{
	char **com_parsed;
	client_t *tmp_client;
	gag_t *gag;
	
	/* you need to be logged in to use this command */
	if(!check_client_is_logged(from_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));
	
	/* This command takes exactly one argument */
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /ungag nick");

		efree(com_parsed);
		return -1;
	}
	
	/* takes the selected client */
	if((tmp_client = get_client_by_nick(com_parsed[1])) == NULL) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "can't find %s.", com_parsed[1]);
		
		free_string_array(com_parsed, 2);
		return -2;
	}
	
	/* check if the client has been gagged */
	if(!is_gagged(tmp_client)) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "%s is not gagged",
		 tmp_client->nick);

		free_string_array(com_parsed, 2);
		return -1;
	}

	/* you can not ungag a user if you do not have the required level */
	gag = tmp_client->gag;
	if(from_client->level < gag->level) {
		log(_("%s(%d) tried to ungag %s (gag level %d)"), from_client->nick,
		 from_client->level, tmp_client->nick, gag->level);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Can't ungag %s, permission denied !",
		 tmp_client->nick);
		
		free_string_array(com_parsed, 2);
		return -3;
	}
	
	/* Ungag is authorized */
	remove_gag(gag);

	log(_("%s has been successfully ungagged by %s"), tmp_client->nick,
	 from_client->nick);
	// MSG_TO_CLIENT
	send_generic_to_level(from_client->level, SEND_MSG, "%s has been successfully ungagged by %s",
	 tmp_client->nick, from_client->nick);
	
	free_string_array(com_parsed, 2);
	return 1;
}


/* send_gag_list_to
 * sends the gag list the the client who requested it
 * we just display the gags set by a level <= than his
 */
static void send_gag_list_to(client_t *from_client)
{
	unsigned int gag_num = 0;
	gag_t *cur = first_gag;
	char expires[BUF_SIZE] = "never";

	while(cur != NULL) {

		if(cur->level <= from_client->level) {
			gag_num++;

			if(cur->expiration != 0)
				sprintf(expires, "%ldmin", (cur->expiration - time(NULL))/60);

			if(gag_num == 1) {  /* first one, we display the headers */
				// MSG_TO_CLIENT
				send_generic_to(from_client, SEND_MSG, "Nick           Gagged by      Level Expires");
				send_generic_to(from_client, SEND_MSG, "-------------------------------------------");
			}

			// MSG_TO_CLIENT
			send_generic_to(from_client, SEND_MSG, "%-14s %-14s   %d   %7s",
			 cur->nick, cur->gagger, cur->level, expires);
		}

		cur = cur->next;
	}
	
	if(gag_num == 0)
		send_generic_to(from_client, SEND_MSG, "Nobody has been gagged yet");
	else
		send_generic_to(from_client, SEND_MSG, "-------------------------------------------");
}



/* gag_timer
 * this function is called by the scheduler every minute
 * we see if a gag has expired and we remove it if it is the case
 */
void gag_timer(void *data)
{
	gag_t *cur = first_gag;
	gag_t *next;
	unsigned long now = time(NULL);

	while(cur != NULL) {
		next = cur->next;

		/* Is the gag expired ? */
		if(cur->expiration == 0 || cur->expiration > now) {
			cur = next;
			continue;
		}

		/* the gag has expired : let's remove it */
		log(_("The gag of %s is over"), cur->nick);
		// MSG_TO_CLIENT
		send_generic_to_level(cur->level, SEND_MSG,
		 "%s gag is over", cur->nick);

		remove_gag(cur);

		/* iterates */
		cur = next;
	}
}


/* set_gag
 * sets a gag on a user
 */
static gag_t *set_gag(client_t *from, client_t *target, unsigned long exp)
{
	gag_t *res;

	/* fill-in the structure */
	res = emalloc(sizeof(*res));
	res->nick = estrdup(target->nick);
	res->address = emalloc(sizeof(struct sockaddr_in));
	memcpy(res->address, target->address, sizeof(struct sockaddr_in));
	res->gagger = estrdup(from->nick);
	res->level = from->level;
	res->expiration = exp;

	/* sets the client gag */
	target->gag = res;
	
	/* adds it to the linked list */
	res->next = first_gag;
	first_gag = res;

	return res;
}



/* remove_gag
 * removes a gag_t struct from the linked list and free it
 */
static void remove_gag(gag_t *gag)
{
	gag_t *prev = first_gag;
	client_t *cur;
	
	/* Removes the struct from the linked list */
	if(gag == first_gag) {
		first_gag = gag->next;
	} else {
		while(prev != NULL && gag != prev->next)
			prev = prev->next;

		if(prev == NULL) { /* should not happen */
			fprintf(stderr, "ERROR in remove_gag : gag not found in linked list !\n");
			return;
		}
		
		prev->next = gag->next;
	}

	/* removes the link from the client */
	cur = first_client;
	while(cur != NULL) {
		if(cur->gag == gag)
			cur->gag = NULL;

		cur = cur->next;
	}

	/* free the struct */
	efree(gag->nick);
	efree(gag->gagger);
	efree(gag->address);
	efree(gag);
}

/* is_new_client_gagged
 * checks if a client that is connected needs to be gagged
 * because he was before
 * returns 1 if a previous gag was found, 0 before
 */
extern unsigned int is_new_client_gagged(client_t *client)
{
	gag_t *cur = first_gag;
	
	while(cur != NULL) {
		if(cur->address->sin_addr.s_addr == client->address->sin_addr.s_addr) {
			/* IP address matches : apply the gag */
			client->gag = cur;
			return 1;
		}
		cur = cur->next;
	}

	return 0;
}

