/* NCurses client for chat_everywhere
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <gtk/gtk.h>


#include "nc_chat.h"
#include "screen_utils.h"
#include "config.h"
#include "io_comm.h"
#include "commands.h"
#include "user_input.h"

int main(int argc, char* argv[])
{
	int res;

	if(argc != 4)
		return(1);


	users_name = NULL;
	users_number = 0;


	printf("Logging to %s\n", argv[1]);
	res = login_to_chat_server(argv[1] , atoi(argv[2]), argv[3]);

	if(res != LOGIN_SUCCESS)
	{
		printf("Login failed\n");
		exit(1);
	}


	gtk_init(&argc, &argv);
	init_screen();

	refresh_users_list();
	refresh_users_window(users_number, users_name);

	enter_main_loop();
	
	return(0);
}


int enter_main_loop()
{
	int fd_num;
	fd_set current_fds, saved_fds;
	
	FD_ZERO(&current_fds);
	FD_ZERO(&saved_fds);
	FD_SET(server_fd, &current_fds);
	FD_SET(STDIN_FILENO, &current_fds);
	fd_num = getdtablesize();
	
	while(1)
	{
		memcpy(&saved_fds, &current_fds, sizeof(current_fds));
		
		if((select(fd_num, &saved_fds, 0, 0, 0)) == -1)
		{
			if(errno == EINTR)
				continue;
			
			perror("select");
			exit(1);
		}
		
		if(FD_ISSET(server_fd, &saved_fds))
			treat_server_command();
		
		if(FD_ISSET(STDIN_FILENO, &saved_fds))
			treat_user_input();

	}
	
	return(0);
}
