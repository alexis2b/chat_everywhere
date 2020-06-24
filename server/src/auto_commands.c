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
/* auto_commands.c : manage config time autocommands */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "server_config.h"
#include "i18n.h"

#include "mem_utils.h"
#include "config_file.h"
#include "str_utils.h"
#include "net_utils.h"
#include "auto_commands.h"

/* Static global variables */
static struct auto_level *first_autolevel = NULL;
static struct reserved_nick *first_reserved_nick = NULL;



/* ================== Auto Level Directive ============*/

int read_auto_level_config()
{
	int i,n;
	char **auto_levels;
	const char *config;

	/* free a previous config if there is one */	
	if(first_autolevel)
		free_auto_level_list();

	config = get_config_value("AutoLevel");
	auto_levels = tokenize(config, ",", &n);
	
	for(i = 0; i < n; i++) {
		char *nick;
		char *tmp, *tmp2;
		int ip;
		int mask;
		int level;
		struct auto_level *current;
	

		/* Check if the parameter is valid */
		if(strchr(auto_levels[i], ':') == NULL) {
			fprintf(stderr, _("Error: invalid AutoLevel parameter (%s)\n"),
			 auto_levels[i]);
			continue;
		}
		
		/* Check if an IP has been specified */
		if(strchr(auto_levels[i], '@') == NULL) {
			nick = cut_to(auto_levels[i], ":");

			tmp = cut_from(auto_levels[i], ":");
			level = atoi(tmp);
			efree(tmp);

			ip = 0;		/* valid for any IP : 0.0.0.0/0 */
			mask = 0;
		} else {

			nick = cut_to(auto_levels[i], "@");
		
			tmp = cut_from(auto_levels[i], ":");
			level = atoi(tmp);
			efree(tmp);

			tmp = cut_from(auto_levels[i], "@");
			tmp2 = cut_to(tmp, ":");
			efree(tmp);

			if(strchr(tmp2, '/') != NULL) {
				tmp = cut_from(tmp2, "/");
				mask = atoi(tmp);
				efree(tmp);
				tmp = cut_to(tmp2, "/");
				efree(tmp2);
				tmp2 = tmp;
			} else {
				mask = 32;
			}
		
			ip = inet_addr(tmp2);
			efree(tmp2);
		}


		/* Fill in the new struct */
		current = emalloc(sizeof(*current));
		current->ip = ip;
		current->mask = mask;
		current->level = level;
		current->nick = nick;
		
		/* Add it to the tree */
		current->next = first_autolevel;
		first_autolevel = current;
	}

	free_string_array(auto_levels, n);
		
	return n;
}


/*
 * original_level_of : returns 0 normally or the given
 * autolevel as specified in the config file
 */
int original_level_of(client_t *client)
{
	struct auto_level *current = first_autolevel;
	
	while(current) {
		if(!strcasecmp(current->nick, client->nick)) {
			if(ip_match(current->ip, client->address->sin_addr.s_addr, current->mask))
				return current->level;
		}
		current = current->next;
	}

	return 0;  /* should this be configurable ? */
}



/* efree the auto_level linked list */
void free_auto_level_list()
{
	struct auto_level *current = first_autolevel;
	struct auto_level *next;
	
	while(current) {
		next = current->next;
		
		efree(current->nick);
		efree(current);
		current = next;
	}
	
	first_autolevel = NULL;
}


/* ============= Reserved Nick Directive ==============*/
int read_reserved_nick_config()
{
	int i,n;
	char **reserved_nicks;
	const char *config;

	/* efree a previous config if there is one */	
	if(first_reserved_nick)
		free_reserved_nick_list();

	config = get_config_value("ReserveNick");
	reserved_nicks = tokenize(config, ",", &n);
	
	for(i = 0; i < n; i++) {
		char *nick;
		char *tmp, *tmp2;
		int ip;
		int mask;
		struct reserved_nick *current;

		/* Check if the parameter is valid */
		if(strchr(reserved_nicks[i], '@') == NULL) {
			fprintf(stderr, _("Error: invalid ReserveNick parameter (%s)\n"),
			 reserved_nicks[i]);
			continue;
		}

		nick = cut_to(reserved_nicks[i], "@");
		
		tmp = cut_from(reserved_nicks[i], "@");
		if(strchr(tmp, '/') != NULL) {
			tmp2 = cut_from(tmp, "/");
			mask = atoi(tmp2);
			efree(tmp2);
			tmp2 = cut_to(tmp, "/");
			efree(tmp);
			tmp = tmp2;
		} else
			mask = 32;
		
		ip = inet_addr(tmp);
		efree(tmp);
		
		/* Fill in the new struct */
		current = emalloc(sizeof(struct reserved_nick));
		current->ip = ip;
		current->mask = mask;
		current->nick = nick;
		
		/* Add it to the tree */
		current->next = first_reserved_nick;
		first_reserved_nick = current;
	}
	free_string_array(reserved_nicks, n);
		
	return n;
}

/* Check if the user may get the nick
 * Returns 1 if this all is ok or 0 if the nick is reserved
 */
int may_use_nick(client_t *the_client)
{
	struct reserved_nick *current = first_reserved_nick;
	
	while(current) {
		if(!strcasecmp(the_client->nick, current->nick)) {
			if(!ip_match(the_client->address->sin_addr.s_addr, current->ip, current->mask))
				return 0;
		}		
		current = current->next;
	}

	return 1;
}

/* efree the reserved nick linked list (reimplement as recursive?) */
void free_reserved_nick_list()
{
	struct reserved_nick *current = first_reserved_nick;
	struct reserved_nick *next;
	
	while(current) {
		next = current->next;
		
		efree(current->nick);
		efree(current);
		current = next;
	}
	
	first_reserved_nick = NULL;
}
