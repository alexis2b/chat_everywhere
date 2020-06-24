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
/* Command file for password auth */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "server_config.h"
#include "i18n.h"

#include "chat_server.h"
#include "commands.h"
#include "communication.h"
#include "logging_support.h"
#include "mem_utils.h"
#include "password_auth.h"

int dispatch_auth_message(client_t *from_client, char *command)
{
	char **com_parsed;
	auth_t auth_res;

	com_parsed = emalloc(2 * sizeof(char *));
	if(parse_chat_message(command, com_parsed, 2) != 2) {
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "invalid AUTH command");
		efree(com_parsed);
		return -1;
	}

	auth_res = authentify_user(from_client, com_parsed[1]);

	switch(auth_res) {
		case AUTH_SUCCESS:
			from_client->authentified = 1;
			efree(from_client->challenge);
			accept_login(from_client);
			break;

		case AUTH_FAILED:
			from_client->auth_fails++;
			/* FIXME : config value pour max number de failures */
			log(_("user %s tried to authenticate with a bad password"), ip_of(from_client));
			send_generic_to(from_client, SEND_ERROR, "Password incorrect");
			break;
		
		default:
			send_generic_to(from_client, SEND_ERROR, "invalid AUTH command received");
			break;
	}


	free_string_array(com_parsed, 2);
	return 1;
}

