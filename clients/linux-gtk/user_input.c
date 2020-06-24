/* Manage user input */


#include <stdlib.h>
#include <stdio.h>
#include <ncurses/ncurses.h>
#include <string.h>

#include "user_input.h"
#include "screen_utils.h"
#include "io_comm.h"
#include "str_utils.h"

/*
 * Thuis function is called when the user pressed
 * the enter key
 * it will then dispatch the input to the right function
 */
int treat_user_input()
{
	char* in;

	if(!(in = malloc(BUF_SIZE)))
		return -1;
		
	get_user_string(in);
	
	/* Is there some datas ? */
	if(*in == 0)
		return 0;
	
	/* Is it a command ? */
	if(*in == '/')
		send_user_command(in);
	else
		send_user_text(in);

	free(in);
	return 1;
}



/*
 * Sends a user command
 * returns the number of arguments
 */
int send_user_command(char *text)
{
	char *line;
	char *command;
	char *arg1;
	char *pos;
	char *tmp;

	line = str_clean(text);
	
	if((pos = strchr(line, ' ')) == NULL)
	{
		send_chat_message(1, str_upper(line + 1));
		free(line);
		return(0);
	}
	
	command = malloc(pos - line);
	strncpy(command, line + 1, pos - line - 1);
	command[pos - line - 1] = 0;
	str_upper(command);
	
	tmp = str_clean(pos + 1);
	if((pos = strchr(tmp, ' ')) == NULL)
	{
		send_chat_message(2, command, tmp);

		free(tmp);
		free(command);
		free(line);
		return(1);
	}
	
	arg1 = malloc(pos - tmp);
	strncpy(arg1, tmp, pos - tmp - 1);
	arg1[pos - tmp - 1] = 0;

	send_chat_message(3, command, arg1, pos + 1);

	free(tmp);
	free(command);
	free(line);
	free(arg1);
	return 2;
}

/* 
 * Send a talk message with the user text
 */
int send_user_text(char *text)
{
	return(send_chat_message(2, "TALK", text));
}
