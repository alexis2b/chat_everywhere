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
/* Users management */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>              /* perror */
#include <unistd.h>             /* close */
#include <sys/socket.h>         /* shutdown */
#include <string.h>             /* strcasecmp */

#include "users.h"
#include "chat_server.h"        /* remove_fd : FIXME that's ugly! */
#include "server_config.h"		/* BUF_SIZE */

#include "i18n.h"
#include "communication.h"      /* send_generic_to_all */
#include "logging_support.h"    /* log */
#include "mem_utils.h"          /* efree */
#include "net_utils.h"          /* sin_addr2ip_char */
#include "statistics.h"         /* get_client_stats */
#include "users_list_file.h"			/* refresh_users_file */


/* new_client
 * allocate and initialize a new user structure
 */
struct chat_client *new_client()
{
	struct chat_client *res;
	
	res = emalloc(sizeof(*res));
	
	res->next = NULL;
	res->prev = NULL;
	res->nick = NULL;
	res->level = 0;
	res->fd = 0;
	res->address = NULL;
	res->ignored = emalloc(sizeof(*(res->ignored)));
	res->ignored[0] = NULL;
	res->ping_answered = 1;
	res->last_lag = 0;
	res->challenge = NULL;
	res->auth_asked = 0;
	res->auth_fails = 0;
	res->authentified = 0;
	res->login_complete = 0;
	res->user_agent = NULL;
	res->user_lang = NULL;
	res->referer = NULL;

	/* receiving buffer */
	res->recv_buffer = emalloc(CLIENT_RCVBUF_START_SIZE *
	 sizeof(*(res->recv_buffer)));
	res->recv_pos = 0;
	res->recv_size = CLIENT_RCVBUF_START_SIZE;
	
	/* gag parameters */
	res->gag = NULL;
	
	
	return res;
}



/*
 * Returns the struct of the given nick or NULL
 * if not found
 */
struct chat_client *get_client_by_nick(char *nick)
{
	struct chat_client *tmp_client = first_client;
	
	while(tmp_client) {
		if(tmp_client->login_complete) {
			if(!strcasecmp(tmp_client->nick, nick))
				return tmp_client;
		}
		
		tmp_client = tmp_client->next;
	}

	return NULL;
}


/*
 * Returns a string containing the IP address of the
 * client in quad-dotted notation.
 */
char *ip_of(struct chat_client *the_client)
{
	return sin_addr2ip_char(the_client->address);
}


/* 
 * User has cut the connection, notify the others and clean it from
 * the users list
 */
void user_deconnect(struct chat_client* the_client, const char *byemsg)
{
	int n = 0;
	struct user_stats *the_stats = NULL;
	char log_msg[BUF_SIZE + 1];


	/* Removing it from the client list */
	if(the_client->prev) {
		the_client->prev->next = the_client->next;
		
		if(the_client->next)
			the_client->next->prev = the_client->prev;
	} else {
		first_client = the_client->next;
		
		if(the_client->next)
			the_client->next->prev = NULL;
	}
	
	shutdown(the_client->fd, SHUT_RDWR);
	
	if(close(the_client->fd) == -1)
		perror("close");

	/* FIXME : I need to think about that fds thing... */
	remove_fd(the_client->fd);
	/* FD_CLR(the_client->fd, &current_fds); */
	
	if(the_client->login_complete) {
		log(_("User %s has quit (%s)"), the_client->nick, byemsg);
		snprintf(log_msg, BUF_SIZE, _("*** %s has quit (%s)"), the_client->nick, byemsg);
		log_to_talk(log_msg);


		send_generic_to_all(SEND_DECONNECTION, "%s <|> %s", the_client->nick, byemsg);

		/* updates the statistics */
		if((the_stats = get_user_stats(the_client)) != NULL) {
			the_stats->last_quit = time(NULL);
			the_stats->spent_time += the_stats->last_quit - the_stats->last_login;
		}
		
		/* removes the user from the file list */
		refresh_users_file();

		efree(the_client->nick);
	} else {
		log(_("Deconnecting %s"), ip_of(the_client));
	}

	/* efree all the memory allocated for this client */
	n = 0;
	while(the_client->ignored[n] != NULL) {
		efree(the_client->ignored[n]);
		n++;
	}
	efree(the_client->ignored);

	efree(the_client->recv_buffer);
	efree(the_client->address);

	if(the_client->user_agent)
		efree(the_client->user_agent);
	if(the_client->user_lang)
		efree(the_client->user_lang);
	if(the_client->referer)
		efree(the_client->referer);

	efree(the_client);
}


/* add_client
 * add a client to the clients linked list
 */
void add_client(struct chat_client *the_client)
{
	struct chat_client *tmp_client = first_client;

	if(first_client == NULL) {
		first_client = the_client;
		the_client->next = NULL;
		the_client->prev = NULL;
	} else {
		tmp_client = first_client;

		while(tmp_client->next)
			tmp_client = tmp_client->next;

		tmp_client->next = the_client;
		the_client->next = NULL;
		the_client->prev = tmp_client;
	}
}


/*
 * Returns the number of users
 */
int count_users()
{
	client_t *current = first_client;
	int n = 0;
	
	while(current) {
		if(current->login_complete)
			n++;
		current = current -> next;
	}
	
	return n;
}



/*
 * Reset the receive buffer of a given client
 */
void reset_recv_buffer(client_t *the_client)
{
	the_client->recv_buffer = erealloc(the_client->recv_buffer,
	 CLIENT_RCVBUF_START_SIZE * sizeof(*(the_client->recv_buffer)));
	the_client->recv_size = CLIENT_RCVBUF_START_SIZE;
	the_client->recv_pos = 0;
}


/* is_gagged
 * returns 0 if the user is not gagged or 1 if he is
 */
unsigned int is_gagged(client_t *the_client)
{
	/* if gagged : gagger != NULL */
	return (the_client->gag?1:0);
}



/* set_user_agent
 * replace the current user_agent information of the
 * user by the selected string
 */
void set_user_agent(client_t *client, char *u_a)
{
	if(client->user_agent)
		efree(client->user_agent);

	client->user_agent = estrdup(u_a);
}


/* set_referer
 * replace the current referer information of the
 * user by the selected string
 */
void set_referer(client_t *client, char *ref)
{
	if(client->referer)
		efree(client->referer);

	client->referer = estrdup(ref);
}


/* set_user_lang
 * replace the current user language information of the
 * user by the selected string
 */
void set_user_lang(client_t *client, char *lng)
{
	if(client->user_lang)
		efree(client->user_lang);

	client->user_lang = estrdup(lng);
}
