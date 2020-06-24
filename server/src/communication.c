/* Chat_server
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
/* Generic client - server communication module */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <netinet/in.h>  /* for FreeBSD */
#include <sys/socket.h>
#include <string.h>      /* strlen */

#include "server_config.h"
#include "commands.h"
#include "communication.h"

/* Quirks for some systems (HP-UX) */
#ifndef MSG_DONTWAIT
# define MSG_DONTWAIT 0
#endif /* MSG_DONTWAIT */

/* Private functions */
static char *send_types_text(send_types_t type);
static int send_to_socket(int s, char *buffer);


/* send_types_text : tells which is the text for the command */
static char *send_types_text(send_types_t type)
{
	static char *asc_types[] = {
		"MSG",
		"ERROR",
		"CONNECTION",
		"DECONNECTION",
		"ACTION",
		"PING",
		"AUTH",
		"LOGIN OK",
		"USERS",
		"PMSG",
		"TALK"};
	
	return asc_types[(int) type];
}


/* send_generic_to_*   : generic functions to send a message formatted
 * a-la printf
 */
void send_generic_to(struct chat_client *to_client, send_types_t type, char *format, ...)
{
	va_list ap;
	char msg[BUF_SIZE + 1];
	char format2[BUF_SIZE + 1];
	
	va_start(ap, format);
	snprintf(format2, BUF_SIZE, "SERVER <|> %s <|> %s <|> SERVER\n", send_types_text(type), format);
	vsnprintf(msg, BUF_SIZE, format2, ap);
	send_to_socket(to_client->fd, msg);
	
	/* printf(msg); */
}


/* FIXME : integrate that at send_generic_to (like when format = '\0') */
void send_generic_to_nf(client_t *to_client, send_types_t type)
{
	char format2[BUF_SIZE + 1];
	
	snprintf(format2, BUF_SIZE, "SERVER <|> %s <|> SERVER\n", send_types_text(type));
	send_to_socket(to_client->fd, format2);
}


void send_generic_to_all(send_types_t type, char *format, ...)
{
	va_list ap;
	char msg[BUF_SIZE + 1];
	char format2[BUF_SIZE + 1];
	struct chat_client* tmp_client = first_client;
	
	va_start(ap, format);
	snprintf(format2, BUF_SIZE, "SERVER <|> %s <|> %s <|> SERVER\n", send_types_text(type), format);
	vsnprintf(msg, BUF_SIZE, format2, ap);
	
	while(tmp_client) {
		if(tmp_client->login_complete)
			send_to_socket(tmp_client->fd, msg);
		tmp_client = tmp_client->next;
	}
}

void send_generic_to_level(int minlev, send_types_t type, char *format, ...)
{
	va_list ap;
	char msg[BUF_SIZE + 1];
	char format2[BUF_SIZE + 1];
	struct chat_client* tmp_client=first_client;
	
	va_start(ap, format);
	snprintf(format2, BUF_SIZE, "SERVER <|> %s <|> %s <|> SERVER\n", send_types_text(type), format);
	vsnprintf(msg, BUF_SIZE, format2, ap);
	
	while(tmp_client) {
		if(tmp_client->login_complete && tmp_client->level >= minlev)
			send_to_socket(tmp_client->fd, msg);
		tmp_client = tmp_client->next;
	}
}


/* send_to_socket
 * handles the lowest communication level : send() on the socket
 * this is factorised to get non-blocking I/O
 */
int send_to_socket(int s, char *buffer)
{
	int r;

	if((r = send(s, buffer, strlen(buffer), MSG_DONTWAIT)) == -1) {
		if(errno == EAGAIN) 
			fprintf(stderr, "WARNING : send() would block, discarded\n");
	}

	return r;
}

