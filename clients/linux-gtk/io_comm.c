/* Input/Output functions for the client */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "io_comm.h"
#include "str_utils.h"
#include "sock_utils.h"
#include "fd_utils.h"

/* 
 * Logins to the given chat_server
 * returns a status code defined in io_comm.h
 * or 0 if all went ok (LOGIN_SUCCESS)
 */

int login_to_chat_server(char* server_name, int port, char* login)
{
	struct chat_message *reply;
	int i;

	if((server_fd = open_tcp_socket(address_resolve(server_name), port)) == -1)
		return(LOGIN_ERROR);
	
	send_chat_message(2, "LOGIN", login);
	
	if(!(reply = recv_chat_message()))
		return(LOGIN_ERROR);
		
	printf("Done\n");
	
	printf("Reply from server :\n");
	printf("Number of elements : %d\n", reply->number);
	for(i = 0; i < reply->number; i++)
		printf("parts[%d] = #%s#\n", i, reply->parts[i]);

	return(LOGIN_SUCCESS);
}


/*
 * Send a specially formated message to the server
 * Returns -1 if failed
 */
int send_chat_message(int number, ...)
{
	char *f_string;
	char *args_string;
	char *message;
	va_list pp;
	int i;
	
	if(number <= 0)
		return -1;

	va_start(pp, number);

	if(!(args_string = malloc(BUF_SIZE)))
	{
		fprintf(stderr, "Error : not enough memory\n");
		return -1;
	}
	

	for(i = 0; i < number; i++)
		strcpy(args_string + strlen("%s <|> ") * i, "%s <|> ");
	
	if(!(f_string = malloc(BUF_SIZE)))
	{
		fprintf(stderr, "Error : not enough memory\n");
		return(-1);
	}
	
	sprintf(f_string, "CLIENT <|> %sCLIENT\n", args_string);
	free(args_string);
	
	if(!(message = malloc(BUF_SIZE)))
	{
		printf("Error : not enough memory");
		return(-1);
	}
	
	vsnprintf(message, BUF_SIZE, f_string, pp);

	return(send_message(message));
}



/* 
 * Sends a message to the server
 */
int send_message(char *message)
{
	return(send_string(server_fd, message));
}


/*
 * Receive a chat message and parse it to a struct
 */
struct chat_message * recv_chat_message()
{
	struct chat_message *res;
	char *buffer, *line, *saved;
	int n = 0;
	char* tmps;
	char* pos;
	
	if(!(buffer = malloc(BUF_SIZE)) || !(res = malloc(sizeof(struct chat_message))))
	{
		fprintf(stderr, "Error : not enough memory\n");
		return NULL;
	}

	if(read_line(server_fd, buffer, BUF_SIZE) < 0)
	{
		fprintf(stderr, "Error in readline\n");
		return NULL;
	}

	/* Gets the number of elements in this message */
	line = str_clean(buffer);
	free(buffer);
	pos = line - 1;
	while((pos = strstr((char*) (pos+1), "<|>")) != 0)
		n++;

	/* A valid message has at least 2 separators */
	if(n < 2)
		return NULL;

	res->number = n - 1;

	tmps = strdup(line);
	tmps[strlen("SERVER")] = 0;

	/* Is message starting by SERVER ? */
	if(strcmp(tmps, "SERVER"))
	{
		free(tmps);
		free(line);
		free(res);
		return NULL;
	}
	free(tmps);

	buffer = str_clean(line + strlen("SERVER <|>"));
	saved = buffer;
	res->parts = malloc(res->number * sizeof(char*));

	/* Ok, splitting the message into an array */
	n = 0;
	while((pos = strstr(buffer, "<|>")) != 0)
	{
		int offset;
		
		offset = pos - buffer;
		tmps = malloc(offset + 1);
		strncpy(tmps, buffer, offset);
		tmps[offset] = 0;
  		res->parts[n++] = str_clean(tmps);
		free(tmps);
		buffer += offset + strlen("<|>");
	}


	free(saved);
	free(line);

	/* Returns the number of elements in the array */
	return(res);
}


/*
 * refresh the users list
 * returns -1 if failed or the number of users if it succeeded
 */
int refresh_users_list()
{
	struct chat_message * reply;
	int i;

	send_chat_message(1, "USERS");
	if((reply = recv_chat_message()) == NULL)
		return -1;
		
	for(i = 0; i < users_number; i++)
	{
		free(users_name[i]);
	}
	
	if(users_name)
		free(users_name);
	
	users_number = reply->number - 1;
	users_name = malloc(users_number * sizeof(char *));
	
	for(i = 0; i < users_number; i++)
		users_name[i] = strdup(reply->parts[i+1]);
	
	return(reply->number);
}
