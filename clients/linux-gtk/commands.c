/* Server commands - related functions */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "io_comm.h"
#include "screen_utils.h"
#include "commands.h"

/*
 * treat a server command
 */
int treat_server_command()
{
	struct chat_message *reply;
	int command;
	
	if((reply = recv_chat_message()) == NULL)
		return -1;
	
	if(reply->number < 1)
		return -1;
	
	command = whatis_command(reply->parts[0]);
	
	switch(command)
	{
		case COM_UNKNOWN :
			print_on_screen("** Error : received unknown command : %s\n", reply->parts[0]);
			break;

		case COM_TALK:
			dispatch_talk_command(reply);
			break;

		case COM_MSG:
			dispatch_msg_command(reply);
			break;
			
		case COM_PMSG:
			dispatch_pmsg_command(reply);
			break;

		case COM_ERROR:
			dispatch_error_command(reply);
			break;			
	}

	return(0);
}


/* 
 * look for a command
 */
int whatis_command(char *command)
{
	if(!strcasecmp(command, "TALK"))
		return COM_TALK;

	if(!strcasecmp(command, "PMSG"))
		return COM_PMSG;
		
	if(!strcasecmp(command, "MSG"))
		return COM_MSG;
	
	if(!strcasecmp(command, "ERROR"))
		return COM_ERROR;
	
	return COM_UNKNOWN;
}


/*
 * Dispatch a user talk line
 */
int dispatch_talk_command(struct chat_message *command)
{
	if(command->number != 3)
		return(-1);
	
	print_on_screen("<%s> %s\n", command->parts[1], command->parts[2]);

	return 1;
}


/*
 * Dispatch a server message
 */
int dispatch_msg_command(struct chat_message *command)
{
	if(command->number != 2)
		return(-1);
	
	print_on_screen("*** %s\n", command->parts[1]);

	return 1;
}


/*
 * Dispatch a server error message
 */
int dispatch_error_command(struct chat_message *command)
{
	if(command->number != 2)
		return(-1);
	
	print_on_screen("*** ERROR : %s\n", command->parts[1]);

	return 1;
}


/*
 * Dispatch a private message
 */
int dispatch_pmsg_command(struct chat_message *command)
{
	if(command->number != 3)
		return(-1);
	
	print_on_screen("=> <%s> %s\n", command->parts[1], command->parts[2]);

	return 1;
}
