/* Various net-utils functions */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sock_utils.h"



/*
 * Resolves an hostname
 */
struct in_addr address_resolve(char *name)
{
        static struct in_addr in;
        unsigned long l;
        struct hostent *ent;

        if((l = inet_addr(name)) != INADDR_NONE)
        {
                in.s_addr = l;
                return in;
        }

        if(!(ent = gethostbyname(name)))
        {
                in.s_addr = INADDR_NONE;
                return in;
        }

        return *(struct in_addr *) ent->h_addr;
}


/*
 * Opens a TCP socket on the server:port
 * Returns : socket fd if succeeded or -1 if failed
 */
int open_tcp_socket(struct in_addr server, int port)
{
	int s_rem;
	struct sockaddr_in rem_addr;

	rem_addr.sin_addr = server;	
	if(rem_addr.sin_addr.s_addr == INADDR_NONE)
	{
		fprintf(stderr, "Error : server unknown\n");
		return(-1);
	}
	
	if((s_rem = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return(-1);
	}
	
	rem_addr.sin_port = htons(port);
	rem_addr.sin_family = AF_INET;
	
	if((connect(s_rem, (struct sockaddr *) &rem_addr, sizeof(rem_addr))) == -1)
	{
		perror("Connect");
		return(-1);
	}

	return(s_rem);
}



int send_string(int socket_fd, char* string)
{
	return(send(socket_fd, string, strlen(string), 0));
}
