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
 * This is the header file for the commands module
 */

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "users.h"          /* struct chat_client */

/* Structures declarations */
typedef struct banishment
{
	struct banishment  *next;
	struct banishment  *prev;
	
	char               *banned_nick;
	char               *banner_nick;
	
	int                 banner_level;

	struct sockaddr_in *banned_address;
	
	unsigned long       expiration;
} ban_t;


/* Functions declarations */
extern void commands_init(void);
extern int whatis_command(char *);
extern int treat_command(char *, struct chat_client *);

extern int dispatch_talk_message(struct chat_client *, char *);
extern int dispatch_ignore_message(struct chat_client *, char *);
extern int dispatch_users_message(struct chat_client *);
extern int dispatch_op_message(struct chat_client *,char *);
extern int dispatch_usersinfo_message(struct chat_client *);
extern int dispatch_msg_message(struct chat_client *, char *);
extern int dispatch_setlevel_message(struct chat_client *, char *);
extern int dispatch_kick_message(struct chat_client *, char *);
extern int dispatch_unignore_message(struct chat_client *, char *);
extern int dispatch_ban_message(struct chat_client *, char *);
extern int dispatch_unban_message(struct chat_client *, char *);
extern int dispatch_help_message(struct chat_client *, char *);
extern int dispatch_seen_message(struct chat_client *, char *);
extern int dispatch_stats_message(struct chat_client *, char *);
extern int dispatch_quit_message(struct chat_client *, char *);
extern int dispatch_action_message(struct chat_client *, char *);
extern int dispatch_login_message(client_t *, char *);
extern int dispatch_auth_message(client_t *, char *);
extern int dispatch_gag_message(client_t *, char *);
extern int dispatch_ungag_message(client_t *, char *);
extern int dispatch_capabilities_message(client_t *, char *);

extern void accept_login(client_t *);
extern int check_client_is_logged(client_t *);
extern void ban_timer(void *);
extern unsigned int is_new_client_gagged(client_t *client);

/* Symbolic returned values by whatis_command */
#define COM_UNKNOWN    0
#define COM_LOGIN      1
#define COM_TALK       2
#define COM_IGNORE     3
#define COM_USERS      4
#define COM_OP         5
#define COM_USERSINFO  6
#define COM_MSG        7
#define COM_SETLEVEL   8
#define COM_KICK       9
#define COM_UNIGNORE  10
#define COM_BAN       11
#define COM_UNBAN     12
#define COM_HELP      13
#define COM_SEEN      14
#define COM_STATS     15
#define COM_QUIT      16
#define COM_ME        17
#define COM_PONG      18
#define COM_AUTH      29
#define COM_GAG       30
#define COM_UNGAG     31
#define COM_CAPS      32
#define COM_MAX_ID    32    /* Just the current number of commands */

/* Global variables */
extern struct banishment* first_ban;   /* node of various linked */

#endif
