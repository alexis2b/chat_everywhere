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
/* Command file for chat_server */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"
#include "logging_support.h"
#include "config_file.h"
#include "statistics.h"

/* FIXME : change the numerical 9 value to op level (in config) */

int dispatch_op_message(struct chat_client* from_client, char* command)
{
	char** com_parsed;
	struct user_stats *the_stats;
	

	if(!check_client_is_logged(from_client))
		return -1;

	if(from_client->level == 9) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "You are already op!");
		return 1;
	}
	
	com_parsed = emalloc(2 * sizeof(char *));
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_MSG, "Usage : /op password");
		efree(com_parsed);
		return -1;
	}
	
	if(strcmp(com_parsed[1], get_config_value("OpPass"))) {
		log(_("%s tried to op with a bad password"), from_client->nick);

		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Bad password : Your try has been logged and notified to ops");
		send_generic_to_level(9, SEND_MSG, "%s tried to op with a bad password", from_client->nick);
		free_string_array(com_parsed, 2);
		return -1;
	}

	log(_("%s gained level 9"), from_client->nick);
	// MSG_TO_CLIENT
	send_generic_to(from_client, SEND_MSG, "Congratulations : access to level 9 granted !");
	send_generic_to_level(9, SEND_MSG, "%s gained level 9", from_client->nick);
	from_client->level = 9;

	/* Check the stats */
	if((the_stats = get_user_stats(from_client)) != NULL) {
		if(from_client->level > the_stats->max_level)
			the_stats->max_level = from_client->level;
	}

	free_string_array(com_parsed, 2);
	return 1;
}
