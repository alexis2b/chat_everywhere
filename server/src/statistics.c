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
/* Manage the global and per user statistics */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "config_file.h"
#include "statistics.h"

/* static declarations */
static struct user_stats *create_user_stats(struct chat_client *);


/* extern variables */
extern struct config_node *first_config;

/*
 * Read the config parameters and initialize various
 * values.
 */
int init_statistics_module()
{
	const char *use_stats;
	int stats_value = STATS_VAL_UNKNOWN;
	
	use_user_stats = 0;
	use_global_stats = 0;
	stats = NULL;
	
	use_stats = get_config_value("Statistics");

	if(!strcasecmp(use_stats, "On"))
		stats_value = STATS_VAL_ON;
	else if(!strcasecmp(use_stats, "Off"))
		stats_value = STATS_VAL_OFF;
	else if(!strcasecmp(use_stats, "GlobalOnly"))
		stats_value = STATS_VAL_GLOBALONLY;
	
	switch(stats_value) {
		case STATS_VAL_ON :
			use_user_stats = 1;
		case STATS_VAL_GLOBALONLY :
			use_global_stats = 1;
			break;
		
		case STATS_VAL_UNKNOWN :
			fprintf(stderr, _("Parse error in config file, Statistics value is not recognized\n"));
			fprintf(stderr, _("Statistics are disabled\n"));
			break;
		
		default :
			break;
	}

	if(use_global_stats)
	{
		 stats = emalloc(sizeof(struct global_stats));
		 stats->kick_number = 0;
		 stats->launch_time = time(NULL);
		 stats->max_users = 0;
		 stats->first_user_stats = NULL;
	}

	return 1;
}


/*
 * Append user : creates or updates the login stats for
 * a given client
 */
int stats_append_user(struct chat_client *the_client)
{
	struct user_stats *the_stats;
	
	if((the_stats = get_user_stats(the_client)) == NULL) {
		if((the_stats = create_user_stats(the_client)) == NULL)
			return 0; 			/* No user stats */
	}

	
	the_stats->last_login = time(NULL);
	the_stats->last_quit = the_stats->last_login;
	the_stats->login_number++;
	if(the_client->level > the_stats->max_level)
		the_stats->max_level = the_client->level;

	return 1;
}


/*
 * creates a new user and append it to the linked list
 * Returns the struct if all was ok or NULL if an error
 * ocurred (no user stats for example).
 */
static struct user_stats *create_user_stats(struct chat_client *the_client)
{
	struct user_stats *result;

	if(!use_user_stats)
		return NULL;		/* Not allowed */

	result = emalloc(sizeof(struct user_stats));
	result->user_nick = estrdup(the_client->nick);
	result->user_address = (int) the_client->address->sin_addr.s_addr;
	result->kick_number = 0;
	result->spent_time = 0;
	result->login_number = 0;
	result->last_login = 0;
	result->last_quit = 0;
	result->max_level = the_client->level;
	result->next = NULL;

	if(use_global_stats) {
		struct user_stats *last = stats->first_user_stats;

		if(!last)
			stats->first_user_stats = result;
		else {
			while(last->next)
				last = last->next;

			last->next = result;
		}
	}

	return result;
}


/*
 * Returns the user stats structure or NULL if none has
 * been recorded.
 */
struct user_stats *get_user_stats(struct chat_client *the_client)
{
	struct user_stats *current;

	if(!use_user_stats)
		return NULL;

	current = stats->first_user_stats;
	while(current) {
		if(!strcasecmp(the_client->nick, current->user_nick) &&
		    the_client->address->sin_addr.s_addr == current->user_address)
			return current;
	
		current = current->next;
	}

	return NULL;
}


/*
 * seconds2ascii : converts a number of seconds into an
 * ascii string splitted in days, hours, minutes and seconds
 * WARNING : you must efree the res string by yourself
 */
char *seconds2ascii(unsigned long seconds)
{
	unsigned int days, hours, minutes;
	char *res = emalloc(BUF_SIZE + 1);
	
	if(!res)
		return NULL;
	
	days = (int) (seconds / 86400L);
	seconds %= 86400L;
	hours = (int) (seconds / 3600L);
	seconds %= 3600L;
	minutes = (int) (seconds / 60L);
	seconds %= 60L;
	
	res[BUF_SIZE] = '\0';
	if(days)
		snprintf(res, BUF_SIZE, "%u days, %u hours, %u minutes, %lu seconds",
		days, hours, minutes, seconds);
	else if(hours)
		snprintf(res, BUF_SIZE, "%u hours, %u minutes, %lu seconds",
		hours, minutes, seconds);
	else if(minutes)
		snprintf(res, BUF_SIZE, "%u minutes, %lu seconds", minutes, seconds);
	else
		snprintf(res, BUF_SIZE, "%lu seconds", seconds);
	
	res = erealloc(res, strlen(res) + 1);

	return res;
}

/*
 * seconds2ascii_compact : converts a number of seconds into
 * an ascii string splitted in hours, minutes and seconds in
 * the format : hhh:mm:ss
 * WARNING : you must free the res string by yourself
 */
char *seconds2ascii_compact(unsigned long seconds)
{
	unsigned int hours, minutes;
	char *res = emalloc(BUF_SIZE + 1);
	
	if(!res)
		return NULL;
	
	hours = (int) (seconds / 3600L);
	seconds %= 3600L;
	minutes = (int) (seconds / 60L);
	seconds %= 60L;
	
	res[BUF_SIZE] = '\0';
	snprintf(res, BUF_SIZE, "%u:%02u:%02lu", hours, minutes, seconds);
	res = erealloc(res, strlen(res) + 1);

	return res;
}
