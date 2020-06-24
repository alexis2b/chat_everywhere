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


int dispatch_usersinfo_message(struct chat_client* from_client)
{
	struct chat_client* tmp_client=first_client;
	int min_level;

	if(!check_client_is_logged(from_client))
		return -1;

	min_level = atoi(get_config_value("UsersInfoMinLevel"));
	if(min_level < 0 || min_level > 9)
		min_level = 9;

	if(from_client->level < min_level) {
		log(_("%s tried to use USERSINFO with a bad level (%d < %d)"), from_client->nick, from_client->level, min_level);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "You need at least level %d to use this command!", min_level);
		return -1;
	}

	// MSG_TO_CLIENT
	send_generic_to(from_client, SEND_MSG, "Users Infos");
	send_generic_to(from_client, SEND_MSG, "Nick                 From            Level");

	while(tmp_client) {
		if(tmp_client->login_complete)
			send_generic_to(from_client, SEND_MSG, "%-20s %-15s   %d",
			 tmp_client->nick, ip_of(tmp_client), tmp_client->level);

		tmp_client = tmp_client->next;
	}

	return 1;
}		
