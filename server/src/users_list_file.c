/* users_file.c
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
 * This module manages a simple text file containing the
 * list of the connected users. It may be useful to display
 * on the web server.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "server_config.h"
#include "config_file.h"
#include "users.h"

#include "users_list_file.h"


/*
 * Refresh the list of connected clients
 */
void refresh_users_file()
{
	int fd;
	const char *file_path;
	client_t *tmp_client = first_client;

	/* Open the UsersFile specified file */
	file_path = get_config_value("UsersFile");
	 
	if(file_path[0] == '\0')
		return;
	
	if((fd = creat(file_path, S_IREAD | S_IWRITE | S_IRGRP | S_IROTH)) < 0) {
		perror("users_file:open");
		return;
	}


	/* Write the nick of all the logged clients */
	while(tmp_client) {
		if(tmp_client->login_complete) {
			write(fd, tmp_client->nick, strlen(tmp_client->nick));
			write(fd, "\n", 1);
		}
		tmp_client = tmp_client->next;
	}

	close(fd);
}


/*
 * Remove the users file (when we shut down the server)
 */
void remove_users_file()
{
	const char *file_path;

	/* Get the UsersFile name */
	file_path = get_config_value("UsersFile");
	 
	if(file_path[0] == '\0' || !strcmp(file_path, "/dev/null"))
		return;

	/* Remove it */
	unlink(file_path);
}
