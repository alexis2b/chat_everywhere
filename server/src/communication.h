/* Chat Everywhere
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
/*
 * communication.h
 * Comments in communication.c
 */

#ifndef _COMMUNICATION_H
#define _COMMUNICATION_H

#include "users.h"

/* Typedefs */
typedef enum {
		SEND_MSG,
		SEND_ERROR,
		SEND_CONNECTION,
		SEND_DECONNECTION,
		SEND_ACTION,
		SEND_PING,
		SEND_AUTH,
		SEND_LOGIN_OK,
		SEND_USERS,
		SEND_PMSG,
		SEND_TALK} send_types_t;


/* Function declarations */
extern void send_generic_to(struct chat_client *, send_types_t, char *, ...);
extern void send_generic_to_nf(client_t *, send_types_t);
extern void send_generic_to_all(send_types_t, char *, ...);
extern void send_generic_to_level(int, send_types_t, char *, ...);


#endif /* COMMUNICATION_H */
