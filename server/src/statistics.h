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
/* This is the include file for the statistic module */

#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include "users.h"             /* struct chat_client */

/* Structures declarations */
struct user_stats
{
	char *user_nick;
	int user_address;
	
	unsigned int kick_number;
	unsigned long spent_time;
	unsigned long login_number;
	time_t last_login;
	time_t last_quit;
	unsigned int max_level;

	struct user_stats *next;
};

struct global_stats
{
	unsigned int kick_number;
	time_t launch_time;              /* Used to compute the uptime */
	unsigned int max_users;
	
	struct user_stats *first_user_stats;
};


/* Functions declarations */
int init_statistics_module(void);
int stats_append_user(struct chat_client *);
struct user_stats *get_user_stats(struct chat_client *);
char *seconds2ascii(unsigned long);
char *seconds2ascii_compact(unsigned long);

/* Global values */
struct global_stats *stats;
unsigned short use_global_stats;
unsigned short use_user_stats;


/* Various defines */
#define STATS_VAL_UNKNOWN    0
#define STATS_VAL_ON         1
#define STATS_VAL_OFF        2
#define STATS_VAL_GLOBALONLY 3

#endif
