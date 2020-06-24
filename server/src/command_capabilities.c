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
/* Command file for client login */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>              /* putc */
#include <string.h>             /* strcascmp */
#include <sys/types.h>			/* needed before netinet/in.h for FreeBSD */
#include <netinet/in.h>         /* sin_addr on FreeBSD */


#include "server_config.h"
#include "i18n.h"

#include "chat_server.h"
#include "commands.h"
#include "communication.h"
#include "logging_support.h"
#include "mem_utils.h"
#include "str_utils.h"
#include "users.h"


/* dispatch_capabilities_message
 * called upon the reception of a CAPABILITIES client description
 * fill the client structure with these additional informations
 */
int dispatch_capabilities_message(client_t *from_client, char *command)
{
	char **com_parsed;
	
	com_parsed = emalloc(4 * sizeof(char *));
	if(parse_chat_message(command, com_parsed, 4) != 4) {
		log(_("Invalid CAPABILITIES message received from %s: %s"),
		 ip_of(from_client), command);
		// MSG_TO_CLIENT
		send_generic_to(from_client, SEND_ERROR, "Invalid CAPABILITIES command");
		efree(com_parsed);
		return -1;
	}

	/* Add the informations to the client structure */
	set_user_agent(from_client, com_parsed[1]);
	set_referer(from_client, com_parsed[2]);
	set_user_lang(from_client, com_parsed[3]);

	free_string_array(com_parsed, 4);
	return 1;
}
