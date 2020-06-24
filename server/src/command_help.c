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
#include "chat_server.h"
#include "communication.h"


int dispatch_help_message(client_t *the_client, char *command)
{
	char **com_parsed;

	if(!check_client_is_logged(the_client))
		return -1;

	com_parsed = emalloc(2 * sizeof(char *));
	if(parse_chat_message(command, com_parsed, 1) == 1) {
		// MSG_TO_CLIENT
		send_generic_to(the_client, SEND_MSG, "Available commands :");
		send_generic_to(the_client, SEND_MSG, "BAN          GAG");
		send_generic_to(the_client, SEND_MSG, "HELP         IGNORE");
		send_generic_to(the_client, SEND_MSG, "KICK         QUIT");
		send_generic_to(the_client, SEND_MSG, "ME           OP");
		send_generic_to(the_client, SEND_MSG, "MSG          SETLEVEL");
		send_generic_to(the_client, SEND_MSG, "SEEN         UNBAN");
		send_generic_to(the_client, SEND_MSG, "STATS        USERSINFO");
		send_generic_to(the_client, SEND_MSG, "UNGAG        UNIGNORE");
		send_generic_to(the_client, SEND_MSG, "Use /help [command] to see more informations");

		free_string_array(com_parsed, 1);
		return 1;
	}
	
	
	if(parse_chat_message(command, com_parsed, 2) == 2) {
		switch(whatis_command(str_upper(com_parsed[1]))) {
			case COM_BAN :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, " BAN : ban an user from the channel");
				send_generic_to(the_client, SEND_MSG, " Usage : /ban [nick [expiration]]");
				send_generic_to(the_client, SEND_MSG, " If nick is not specified, /ban returns the ban list for this channel");
				send_generic_to(the_client, SEND_MSG, " otherwise, 'nick' is banned with the level of the user who ban him.");
				send_generic_to(the_client, SEND_MSG, " You can specify an expiration time in minute after the nick.");
				send_generic_to(the_client, SEND_MSG, "   - You can upgrade the level of a ban by retyping the /ban command with a");
				send_generic_to(the_client, SEND_MSG, "   better level,");
				send_generic_to(the_client, SEND_MSG, "   - You can only ban a user with a level smaller than your,");
				break;

			case COM_UNBAN :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "UNBAN : unban a previously banned user");
				send_generic_to(the_client, SEND_MSG, "Usage : /unban nick");
				send_generic_to(the_client, SEND_MSG, "You must specify the nick of an user previously banned with the /ban command.");
				send_generic_to(the_client, SEND_MSG, "  - You can only unban a people which has been banned with a level smaller or");
				send_generic_to(the_client, SEND_MSG, "  equal than your.");
				break;

			case COM_OP :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, " OP : tries to get op (level 9)");
				send_generic_to(the_client, SEND_MSG, " Usage : /op password");
				send_generic_to(the_client, SEND_MSG, " Pasword must be specified, it is specified in the server config file.");
				send_generic_to(the_client, SEND_MSG, " This gives you the maximum level : level 9");
				send_generic_to(the_client, SEND_MSG, " All invalid tries of this command will be logged and existing ops will be");
				send_generic_to(the_client, SEND_MSG, " notified of your try.");
				break;

			case COM_KICK :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, " KICK : kick an user from the channel");
				send_generic_to(the_client, SEND_MSG, " Usage : /kick nick [message]");
				send_generic_to(the_client, SEND_MSG, " Attempts to kick an user with the given reason : message.");
				send_generic_to(the_client, SEND_MSG, "  - You can only kick an user with a level smaller than your,");
				send_generic_to(the_client, SEND_MSG, "  - invalid kick attempts are notified to the aimed user.");
				break;
		
			case COM_IGNORE :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "IGNORE : ignore all messages from an user");
				send_generic_to(the_client, SEND_MSG, "Usage : /ignore [nick]");
				send_generic_to(the_client, SEND_MSG, "If nick is not specified, /ignore will return your ignore list.");
				send_generic_to(the_client, SEND_MSG, "You may ignore any user.");
				break;
		
			case COM_UNIGNORE :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "UNIGNORE : remove an user from your ignore list");
				send_generic_to(the_client, SEND_MSG, "Usage : /unignore nick");
				send_generic_to(the_client, SEND_MSG, "Removes 'nick' from your ignore list.");
				break;
			
			case COM_USERSINFO :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "USERSINFO : displays the level of each user");
				send_generic_to(the_client, SEND_MSG, "Usage :  /usersinfo");
				send_generic_to(the_client, SEND_MSG, "Returns a list of all the existing users with their level.");
				send_generic_to(the_client, SEND_MSG, "You must be op (level 9) to use this command.");
				break;
		
			case	COM_SETLEVEL :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "SETLEVEL : set an user level");
				send_generic_to(the_client, SEND_MSG, "Usage : /setlevel [nick] level");
				send_generic_to(the_client, SEND_MSG, "If nick is not specified, your level is modified.");
				send_generic_to(the_client, SEND_MSG, "You can only decrease your level");
				send_generic_to(the_client, SEND_MSG, "You can only increase somebody to your level or smaller");
				send_generic_to(the_client, SEND_MSG, "You can't modify the level of a user which has a greater level than your");
				send_generic_to(the_client, SEND_MSG, "The aimed user is notified of his level change");
				break;
				
			case COM_MSG :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "MSG : sends a private message");
				send_generic_to(the_client, SEND_MSG, "Usage : /msg nick_or_level message");
				send_generic_to(the_client, SEND_MSG, "This command sends a private message to nick");
				send_generic_to(the_client, SEND_MSG, "You may also specify a level number (0-9) instead of a");
				send_generic_to(the_client, SEND_MSG, "nick to send the message to all the users >= level");
				break;
			
			case COM_HELP :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "HELP : get help an a command");
				send_generic_to(the_client, SEND_MSG, "Usage : /help [command]");
				send_generic_to(the_client, SEND_MSG, "If 'command' is not specified, /help returns a list of all the available");
				send_generic_to(the_client, SEND_MSG, "commands, else, the description of 'command' usage is returned.");
				break;

			case COM_SEEN :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "SEEN : return how long since a user has quit");
				send_generic_to(the_client, SEND_MSG, "Usage : /seen nick");
				send_generic_to(the_client, SEND_MSG, "'nick' is the nick of a user that came on the channel");
				send_generic_to(the_client, SEND_MSG, "Note : there may be several answers if 'nick' was used");
				send_generic_to(the_client, SEND_MSG, "by different IPs.");
				break;

			case COM_STATS :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "STATS : returns server or user stats");
				send_generic_to(the_client, SEND_MSG, "Usage : /stats [nick]");
				send_generic_to(the_client, SEND_MSG, "'nick' is the nick of a user that came on the channel");
				send_generic_to(the_client, SEND_MSG, "If nick is not specified, and if you have a correct level, you may");
				send_generic_to(the_client, SEND_MSG, "view the server global statistics.");
				send_generic_to(the_client, SEND_MSG, "Note : there may be several answers if 'nick' was used");
				send_generic_to(the_client, SEND_MSG, "by different IPs.");
				break;

			case COM_QUIT :
				//MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "QUIT : leave the chat nicely");
				send_generic_to(the_client, SEND_MSG, "Usage : /quit [message]");
				send_generic_to(the_client, SEND_MSG, "You may specify a message that will explain your leaving.");
				break;

			case COM_ME :
				//MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "ME : displays an action message");
				send_generic_to(the_client, SEND_MSG, "Usage : /me message");
				send_generic_to(the_client, SEND_MSG, "Displays message as an action.");
				send_generic_to(the_client, SEND_MSG, "i.e :  your_nick message");
				break;

			case COM_GAG :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "GAG : globally ignore a user");
				send_generic_to(the_client, SEND_MSG, "Usage : /gag [nick]");
				send_generic_to(the_client, SEND_MSG, "Asks nick's message to be ignored by the server.");
				send_generic_to(the_client, SEND_MSG, "If nick is not specified, /gag returns the gag list for this channel");
				send_generic_to(the_client, SEND_MSG, "otherwise, 'nick' is gagged with the level of the user who gagged him.");
				send_generic_to(the_client, SEND_MSG, "  - You can upgrade the level of a gag by retyping the /gag command with a");
				send_generic_to(the_client, SEND_MSG, "  better level,");
				send_generic_to(the_client, SEND_MSG, "  - You can only gag a user with a level smaller than your,");
				break;

			case COM_UNGAG :
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_MSG, "UNGAG : removes a gag put on a user");
				send_generic_to(the_client, SEND_MSG, "Usage : /ungag nick");
				send_generic_to(the_client, SEND_MSG, "You must specify the nick of an user previously gagged with the /gag command.");
				send_generic_to(the_client, SEND_MSG, "  - You can only unban a people which has been banned with a level smaller or");
				send_generic_to(the_client, SEND_MSG, "  equal than your (i.e : you can see him by typing /gag).");
				break;

			default : 
				// MSG_TO_CLIENT
				send_generic_to(the_client, SEND_ERROR, "No help on that topic");
		}		
	
		free_string_array(com_parsed, 2);
		return 1;
	}
	
	// MSG_TO_CLIENT
	send_generic_to(the_client, SEND_MSG, "Usage : /help [command]");
	efree(com_parsed);

	return 1;
}

