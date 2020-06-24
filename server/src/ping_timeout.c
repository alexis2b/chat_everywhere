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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdlib.h>
#include <unistd.h>			/* alarm */
#include <time.h>			/* time(null) */
#include <signal.h>			/* SIGALRM signal */

#include "mem_utils.h"		/* efree */
#include "config_file.h"	/* get_config_value */
#include "chat_server.h"	/* chat_client, parse_chat_message */
#include "server_config.h"	/* CFG_TIME_OUT, DEFAULT_TIME_OUT */
#include "communication.h"	/* send_generic_to */
#include "ping_timeout.h"
#include "scheduler.h"      /* register_alarm */

/* Module global variables */
static unsigned int timeout_delay;

/* Module internal functions */
static void timeout_sendping(void);
static void timeout_ping(struct chat_client *);

/* pingtimeout_init : module initialisation
 * Must be called first !
 * Initialize the module values
 */
unsigned int pingtimeout_init()
{
	timeout_delay = atoi(get_config_value("PingTimeout"));

	/* A value of 0 means time out is disabled */
	if(timeout_delay) {

		/* Set the alarm to the time out delay */
		register_alarm(TASK_PING_TIMEOUT, timeout_delay, alarm_handler,
		 NULL, 1);
	}
	return timeout_delay;
}


/* alarm_handler : SIGALRM called function
 */
void alarm_handler(void *data)
{
	/* Send a new PING and check for the answer */
	timeout_sendping();
}


/* timeout_sendping : send a PING to all the clients
 * and check that they have answered the previous one
 */
static void timeout_sendping(void)
{
	struct chat_client *tmp_client = first_client;
	struct chat_client *nxt_client;
	
	while(tmp_client) {
		/* Could be destroyed if we kick the user */
		nxt_client = tmp_client->next;
	
		if(tmp_client->login_complete) {
			if(tmp_client->ping_answered == 0) {
				/* The client has not answered our previous ping, we deconnect him */
				user_deconnect(tmp_client, "ping timeout");
			} else {
				timeout_ping(tmp_client);
			}
		}		
		tmp_client = nxt_client;
	}
}


/* timeout_ping : send a PING request to a client
 */
static void timeout_ping(struct chat_client *the_client)
{
	unsigned long timestamp = time(NULL);
	
	the_client->ping_answered = 0;
	send_generic_to(the_client, SEND_PING, "%u", timestamp);
}


/* treat_pong : called upon receiving a PONG reply
 */
void dispatch_pong_message(struct chat_client *from_client, char *command)
{
	char **com_parsed;
	unsigned long lag;
	
	/* The client has answered */
	from_client->ping_answered = 1;
	
	com_parsed = emalloc(2 * sizeof(char *));
	if(parse_chat_message(command, com_parsed, 1) == 1) {
		/* Old-style PONG (no timestamp) */
		free_string_array(com_parsed, 1);
		return;
	}

	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "invalid PONG answer, update your client!");
		efree(com_parsed);
		return;
	}

	/*
	 * Compute lag from timestamp
	 */
	parse_chat_message(command, com_parsed, 2);
	lag = time(NULL) - (unsigned long) atol(com_parsed[1]);
	from_client->last_lag = lag;
	free_string_array(com_parsed, 2);
}


/* pingtimeout_config_changed
 * Called when the user asks for a config reparse through SIGHUP
 * we must reread the value of timeout_delay
 */
void pingtimeout_config_changed()
{
	timeout_delay = atoi(get_config_value("PingTimeout"));

	/* Set the alarm to the time out delay */
	cancel_alarm(TASK_PING_TIMEOUT);
	register_alarm(TASK_PING_TIMEOUT, timeout_delay, alarm_handler,
	 NULL, 1);
}
