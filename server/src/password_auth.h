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
 * This is the header file for the password authentication module
 */

#ifndef _PASSWORD_AUTH_H
#define _PASSWORD_AUTH_H

#include "users.h"

/* enums */
typedef enum _auth {
	AUTH_FAILED,      /* wrong password */
	AUTH_SUCCESS,     /* good password */
	AUTH_FREE,        /* nick not protected */
	AUTH_ASKED        /* we have asked for a password */
} auth_t;


/* structs */
typedef struct _protected_nick {
	struct _protected_nick *next;

	char *nick;
	char *hash;
	unsigned int level0;
} protected_nick_t;


/* functions */
extern void password_auth_init(void);
extern void password_auth_reinit(void);
extern auth_t authentify_user(struct chat_client *, const char *);


#endif /* _PASSWORD_AUTH_H */
