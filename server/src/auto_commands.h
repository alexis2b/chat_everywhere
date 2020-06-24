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
 * This is the include for the auto commands module
 */

#ifndef AUTO_COMMANDS_H
#define AUTO_COMMANDS_H

#include "users.h"        /* struct chat_client */

/* Global variables */

/* Structures declarations */

struct auto_level {
	int ip;
	int mask;

	char *nick;
	int level;
	
	struct auto_level *next;
};

struct reserved_nick {
	int ip;
	int mask;
	
	char *nick;
	
	struct reserved_nick *next;
};


/* Functions declarations */
extern int read_auto_level_config(void);
extern int original_level_of(client_t *);
extern void free_auto_level_list(void);

extern int read_reserved_nick_config(void);
extern int may_use_nick(client_t *);
extern void free_reserved_nick_list(void);

#endif /* AUTO_COMMANDS_H */
