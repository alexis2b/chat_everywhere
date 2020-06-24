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
/* Command file for client login */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>              /* putc */
#include <string.h>             /* strcascmp */
#include <sys/types.h>			/* needed before netinet/in.h for FreeBSD */
#include <netinet/in.h>         /* sin_addr on FreeBSD */


#include "server_config.h"
#include "i18n.h"

#include "auto_commands.h"      /* may_use_nick original_level_of */
#include "chat_server.h"
#include "commands.h"
#include "command_gag.h"
#include "communication.h"
#include "config_file.h"
#include "logging_support.h"
#include "mem_utils.h"
#include "password_auth.h"
#include "statistics.h"
#include "users_list_file.h"


/* internal functions */
static unsigned int is_nick_syntax_ok(client_t *, char *);


/* dispatch_login_message
 * called upon the reception of a LOGIN request
 * will check that the login is acceptable
 */
int dispatch_login_message(client_t *from_client, char *command)
{
	char **com_parsed;
	struct banishment* tmp_ban;
	client_t *tmp_client = first_client;
	

	/* this command can not be used if the client is already
	 * logged (for obvious reasons...)
	 * thanks to Christophe SAMMIEZ (MCM.net) for this
	 */
	if(from_client->login_complete) {
		log(_("%s attempted to use the LOGIN command while already logged!"), from_client->nick);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Already logged in");
		return -1;
	}


	com_parsed = emalloc(2 * sizeof(char *));
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		log(_("Login from %s failed : Invalid login attempt (%s)"), ip_of(from_client), command);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Invalid LOGIN command");
		efree(com_parsed);
		return -1;
	}

	/*
	 * Check the login attempt
	 */
	/* Verify if the nick is valid (ie : [a-zA-Z0-9-_-\[-\]-\-]*) */
	if(*com_parsed[1] == '\0') {		
		send_generic_to(from_client, SEND_ERROR, "You must type a nick!");
		log(_("Login from %s failed : empty nick"), ip_of(from_client));
		free_string_array(com_parsed, 2);
		return -1;
	}
	if(strlen(com_parsed[1]) > MAX_NICK_LENGTH) {
		send_generic_to(from_client, SEND_ERROR, "Nick too long (%d characters max.)", MAX_NICK_LENGTH);
		log(_("Login from %s failed : nick is too long (%s)"), ip_of(from_client), com_parsed[1]);
		free_string_array(com_parsed, 2);
		return -1;
	}
	if(is_nick_syntax_ok(from_client, com_parsed[1]) == 0) {
		log(_("Login from %s failed : nick is invalid (%s)"), ip_of(from_client), com_parsed[1]);
		free_string_array(com_parsed, 2);
		return -1;
	}

	/* Check if maximal number of users has been reached */
	if(count_users() == atoi(get_config_value("MaxUsers"))) {
		send_generic_to(from_client, SEND_ERROR, "Sorry, maximal number of users reached");
		log(_("Login from %s@%s failed : max users limit reached"), com_parsed[1], ip_of(from_client));
		free_string_array(com_parsed, 2);
		return -1;
	}

	/*
	 * Check if the user has reached his maximum number of
	 * simultaneous connections
	 */
	if(too_many_connections(from_client)) {
		send_generic_to(from_client, SEND_ERROR, "Too many simultaneous connections");
		log(_("Login from %s@%s failed : maximum number of connections was reached"), com_parsed[1], ip_of(from_client));
		free_string_array(com_parsed, 2);
		return -1;
	}

	/* Check if nick has been reserved */
	from_client->nick = estrdup(com_parsed[1]);
	if(!may_use_nick(from_client)) {
		send_generic_to(from_client, SEND_ERROR, "Nick has been reserved.");
		log(_("Login from %s@%s failed : nick was registered"), com_parsed[1], ip_of(from_client));
		free_string_array(com_parsed, 2);
		return -1;
	}

	/* Checks if nick is already used */
	while(tmp_client) {
		if(tmp_client->login_complete) {
			if(!strcasecmp(tmp_client->nick, com_parsed[1])) {
				send_generic_to(from_client, SEND_ERROR, "Nick already in use !");

				log(_("Login from %s@%s failed : nick in use"), com_parsed[1], ip_of(from_client));
				free_string_array(com_parsed, 2);
				return -1;
			}
		}
		tmp_client = tmp_client->next;
	}

	/* Checks if client is not banned */
	tmp_ban = first_ban;
	while(tmp_ban) {
		if(tmp_ban->banned_address->sin_addr.s_addr == from_client->address->sin_addr.s_addr) {
			send_generic_to(from_client, SEND_ERROR, "You are banned from this channel !");
			log(_("Login from %s@%s failed : banned but tried to join (as %s)"), tmp_ban->banned_nick, ip_of(from_client), from_client->nick);

			free_string_array(com_parsed, 2);
			return -1;
		}
		tmp_ban = tmp_ban->next;
	}

	/* Check if the client needs to authenticate */
	if(authentify_user(from_client, NULL) == AUTH_ASKED) {
		/* Further auth required, we stand by here */
		log(_("asked a password for user %s@%s"), from_client->nick, ip_of(from_client));
	} else {
		/* logs in as a guest */
		if(str_to_bool(get_config_value("AllowGuestLogin")) == 1) {
			accept_login(from_client);
		} else {
			log(_("Login from %s@%s failed : guest login not allowed"),
			 from_client->nick, ip_of(from_client));
			send_generic_to(from_client, SEND_ERROR, "Guest login not allowed");
			free_string_array(com_parsed, 2);
			
			return -1;
		}
	}

	free_string_array(com_parsed, 2);
	return 1;
}



void accept_login(client_t *the_client)
{
	char log_msg[BUF_SIZE + 1];

	/* Notify the new connection to everyone */
	send_generic_to_all(SEND_CONNECTION, "%s", the_client->nick);


	/* Get the initial level of the client */
	/* it may have been specified in the auth process already */
	if(the_client->level == 0)
		the_client->level = original_level_of(the_client);

	/* Notify the user that the login is completed */
	send_generic_to_nf(the_client, SEND_LOGIN_OK);

	/* if the user was gagged before logging out, do it again */
	if(is_new_client_gagged(the_client))
		log(_("%s matches a previous gag, applied again"), the_client->nick);

	/* Send him the motd file */
	send_motd_to(the_client);

	/* Login is now considered complete */
	the_client->login_complete = 1;

	log(_("Login successful from %s@%s"), the_client->nick, ip_of(the_client));
	snprintf(log_msg, BUF_SIZE, _("*** %s joined the chat"), the_client->nick);
	log_to_talk(log_msg);

	if(str_to_bool(get_config_value("BeepOnLogin")))
		putc(7, stderr);                 /* beep! */
				
	/* and updates the statistics */
	if(use_user_stats)
		stats_append_user(the_client);

	if(use_global_stats) {
		int tmp = count_users();

		if(tmp > stats->max_users)
			stats->max_users = tmp;
	}
	
	/* and refresh the connected users file */
	refresh_users_file();
}



/* is_nick_syntax_ok
 * returns 1 if the syntax of the nick is fine, 0 if not
 */
static unsigned int is_nick_syntax_ok(client_t *from_client, char *nick)
{
	unsigned int i, j;
	const char *allow_chars = get_config_value("AllowNickChar");
	const char *deny_chars = get_config_value("DenyNickChar");

	/* Verify if the nick is valid (ie : [a-zA-Z0-9-_-\[-\]-\-]*) */
	for(i = 0; i < strlen(nick); i++) {
		char c = nick[i];

		/* check if one of the chars is expressly denied */
		for(j = 0; j < strlen(deny_chars); j++) {
			if(c == deny_chars[j]) {
				send_generic_to(from_client, SEND_ERROR,
				 "Invalid nick: character '%c' is forbidden", c);
				return 0;
			}
		}

		/* check if one of the chars is expressly allowed */
		for(j = 0; j < strlen(allow_chars); j++) {
			/* spaces or tab can never be valid */
			if(c == ' ' || c == '\t')
				continue;

			if(c == allow_chars[j])
				break;
		}
		if(c == allow_chars[j])
			continue;

		/* we fall back to the default set */
		if(c >= 'a' && c <= 'z')
			continue;
		if(c >= 'A' && c <= 'Z')
			continue;
		if(c >= '0' && c <= '9')
			continue;
		if(c == '_' || c == '-'  || c == '[' || c == ']')
			continue;

		/* If we are still here, then the character is invalid */
		if(c == ' ' || c == '\t')
			send_generic_to(from_client, SEND_ERROR,
			 "Invalid nick: you can not have a space in your nick");
		else
			send_generic_to(from_client, SEND_ERROR,
			 "Invalid nick: character '%c' is forbidden", c);

		return 0;
	}

	/* nick syntax is fine */
	return 1;
}
