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
 * This is the header file for the client management module
 */

#ifndef USERS_H
#define USERS_H

#include "command_gag.h" /* needs gag_t */

/* Structures declarations */
struct chat_client				/* User representation */
{
	struct chat_client* prev;
	struct chat_client* next;

	char   *nick;					/* Nick of the person */
	int		level;					/* [0-9] level of the person */

	int 	fd;						/* Socket fd */
	struct sockaddr_in *address;	/* IP and such stuff */
	
	char **ignored;					/* ignore list */

	unsigned int	ping_answered;	/* Last PING has been answered ? */
	unsigned long	last_lag;		/* Last computed lag */
	
	char *challenge;                /* challenge sent for auth */
	unsigned int auth_asked;        /* we asked him to auth */
	unsigned int auth_fails;        /* number of auth failures */
	unsigned int authentified;      /* the user has authentified */

	unsigned int login_complete;    /* the user has logged in successfully */

	/**** client informations ****/
	char *user_agent;				/* client program and version */
	char *referer;					/* website from which the user comes */
	char *user_lang;				/* user preferred language */

	/**** Storage ****/
	char *recv_buffer;				/* buffer of received data */
	unsigned int recv_pos;			/* pointer in the buffer to see where we are */
	unsigned int recv_size;			/* size alloc'ed for the receive buffer */

	/*** gag *******/				/* global ignore for annoying users */
	gag_t       *gag;               /* set to NULL if user is not gagged */
};
typedef struct chat_client client_t;

/* Functions declarations */
extern struct chat_client *new_client(void);
extern struct chat_client *get_client_by_nick(char *);
extern char *ip_of(struct chat_client *);
extern void user_deconnect(struct chat_client *, const char *);
extern void add_client(struct chat_client *);
extern int count_users(void);
extern void reset_recv_buffer(client_t *);
extern unsigned int is_gagged(client_t *);

extern void set_user_agent(client_t *, char *);
extern void set_referer   (client_t *, char *);
extern void set_user_lang (client_t *, char *);



/* Global variables */
struct chat_client* first_client;       /* pointer to the first user */

#endif /* USERS_H */
