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
/*
 * net_utils.c : various network related functions
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "i18n.h"

#include "server_config.h"
#include "net_utils.h"
#include "config_file.h"
#include "fd_utils.h"			/* RDERR_* stuff */

/* Global variables */
extern struct config_node *first_config;

/* Static fields */
static char localhostname[MAX_INET_NAME_LENGTH];

/* 
 * Open a passive socket on port server_port
 * Returns : fd of the socket
 */
int open_passive_socket(const int server_port)
{
	int passive_socket, yes;
	struct sockaddr_in passive_addr;
	
	if((passive_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

#ifdef SO_REUSEADDR
	/* Set the SO_REUSEADDR flag on the socket if the OS supports it
	 * this allows us to avoid the 2 minutes delay if there is a crash */
	yes = 1;
	if(setsockopt(passive_socket, SOL_SOCKET, SO_REUSEADDR,
	 (char *) &yes, sizeof(yes)) < 0) {
		perror("setsockopt");
		close(passive_socket);
		return -1;
	}
#endif /* SO_REUSEADDR */

	passive_addr.sin_family = AF_INET;
	passive_addr.sin_addr.s_addr = inet_addr(get_config_value("ListenIP"));
	passive_addr.sin_port = htons(server_port);
	
	if(bind(passive_socket, (struct sockaddr*) &passive_addr,
	 sizeof(passive_addr)) == -1) {
		perror("bind");
		close(passive_socket);
		return -1;
	}
	
	if(listen(passive_socket, SERVER_BACK_LOGS) == -1) {
		perror("listen");
		close(passive_socket);
		return -1;
	}
	
	return passive_socket;
}



/* 
 * Converts the numerical IP into a typical "xxx.xxx.xxx.xxx" string
 * returns a pointer on the string
 */
char* sin_addr2ip_char(const struct sockaddr_in* the_addr)
{
	return(inet_ntoa((struct in_addr) the_addr->sin_addr));
}


/*
 * ip_match : check if two IPs are in the same subnet
 * Returns : 1 if the two IPs matches the given subnet
 *           0 else
 */
int ip_match(const int ip1, const int ip2, const int sub)
{
	unsigned int mask = 0, inc;
	int i, j;


	/* Very Lame code... */

	for(i = 0; i < sub; i++)
	{
		inc = 1;
		
		for(j = 0; j < i; j++)
			inc *= 2;

		mask += inc;
	}
	return(((ip1 & mask) == (ip2 & mask)) ? 1 : 0);
}


/* get_client_hostname : returns a string containing the
 * reverse-resolved name of the client (or the quad-dotted
 * ip if we were unable to resolve it).
 */
char *get_client_hostname(const struct sockaddr_in *the_addr)
{
	struct hostent *res;
	struct in_addr addr = the_addr->sin_addr;
	
	if((res = gethostbyaddr((char *) &addr, sizeof(struct in_addr), AF_INET)) == NULL)
		return sin_addr2ip_char(the_addr);
	
	return res->h_name;
}


/* get_local_hostname : returns a pointer to a string
 * containing the resolved local host name.
 *
 * Note : the pointer is toward a statically allocated
 * buffer, so it will be overwrited by any subsequent call.
 */
char *get_local_hostname(void)
{
	char hostname[MAX_INET_NAME_LENGTH];
	char domainname[MAX_INET_NAME_LENGTH];
	
	if(gethostname(hostname, MAX_INET_NAME_LENGTH) == -1) {
		perror("gethostname");
		exit(EXIT_FAILURE);
	}
	
	if(getdomainname(domainname, MAX_INET_NAME_LENGTH) == -1) {
		perror("getdomainname");
		exit(EXIT_FAILURE);
	}
	
	/* Value may be truncated if MAX_INET_NAME_LENGTH is
	* too short (a warning is displayed).
	*/
	if(snprintf(localhostname, MAX_INET_NAME_LENGTH,
	             "%s.%s", hostname, domainname) == -1)
		fprintf(stderr, _("Error : local host name is too long, increase the "
		                "value of MAX_INET_NAME_LENGTH in server_config.h"));

	return localhostname;
}
