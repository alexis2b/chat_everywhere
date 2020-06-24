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

#include "server_config.h"

#include "mem_utils.h"
#include "commands.h"
#include "communication.h"
#include "chat_server.h"
#include "communication.h"
#include "str_utils.h"

int dispatch_users_message(struct chat_client* from_client)
{
	char *msg;
	struct chat_client* cur_client = first_client;
	unsigned int n = 0;
	
	msg = emalloc((2 * BUF_SIZE + 1) * sizeof(*msg));
	msg[0] = '\0';

	while(cur_client) {
		if(cur_client->login_complete) {
			if(n > 0)
				msg = (char *) strcat(msg, " <|> ");

			n++;
			msg = (char *) strcat(msg, cur_client->nick);

		}
		cur_client = cur_client->next;
	}

	if(n == 0)
		send_generic_to_nf(from_client, SEND_USERS);
	else
		send_generic_to(from_client, SEND_USERS, msg);
	
	efree(msg);

	return 1;
}
