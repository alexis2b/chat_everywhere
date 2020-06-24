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
 * Chat_serveur.h
 * Comments in chat_serveur.c
 */
 
#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <time.h>        /* time_t */
#include "str_utils.h"   /* escape_t */
#include "users.h"       /* struct chat_client */

/* Structures */
typedef struct login_in_progress {
	struct chat_client *client;
	int pipe_fd;
	time_t connect_time;            /* I may add a timeout later */
	
	struct login_in_progress *next;
} lip_t;


/* Functions declarations */

extern int parse_chat_message(char *, char **, int);
extern void remove_fd(int);
extern struct banishment *get_ban_by_nick(char *);
extern escape_t *get_escape_table(client_t *);
extern void free_escape_table(escape_t *);
extern int too_many_connections(client_t *);
extern void send_motd_to(client_t *);

/* Global vars */
/*char *config_path;*/              /* path of the config file */

#endif /* CHAT_SERVER_H */
