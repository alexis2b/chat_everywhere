/* header file for functions in io_comm.c */


#ifndef _IO_COMM_H_
#define _IO_COMM_H_


/* Some defines */

#define LOGIN_SUCCESS 0
#define LOGIN_BADNICK 1
#define LOGIN_FULL 2
#define LOGIN_NICK_IN_USE 3
#define LOGIN_BANNED 4
#define LOGIN_ERROR 255

#ifndef BUF_SIZE
#define BUF_SIZE 2048
#endif


/* Types declarations */

struct chat_message
{
	int number;
	char **parts;
};



/* Functions declarations */

int login_to_chat_server(char *, int, char *);
int send_chat_message(int, ...);
int send_message(char *);
struct chat_message * recv_chat_message(void);
int refresh_users_list();

/* Global variables */

int server_fd;

char **users_name;
int users_number;


#endif
