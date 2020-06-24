/* Chat_server
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


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>   /* struct in_addr for FreeBSD */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "i18n.h"

#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#else
	#include "getopt.h"
#endif /* HAVE_GETOPT_H */

#include "server_config.h"
#include "mem_utils.h"
#include "chat_server.h"
#include "commands.h"
#include "command_gag.h"
#include "communication.h"
#include "str_utils.h"
#include "config_file.h"
#include "logging_support.h"
#include "fd_utils.h"
#include "net_utils.h"
#include "auto_commands.h"
#include "statistics.h"
#include "ping_timeout.h"		/* All the PING/PONG stuff */
#include "password_auth.h"      /* for nick password protection */
#include "users_list_file.h"
#include "restrict_command.h"
#include "scheduler.h"

/* ===== Quirks for some systems ==== */
/* MSG_DONTWAIT not defined on HP-UX */
#ifndef MSG_DONTWAIT
# define MSG_DONTWAIT 0
#endif /* MSG_DONTWAIT */

/* socklen_t not defined on MacOS X */
#ifndef HAVE_SOCKLEN_T
 typedef unsigned int socklen_t;
#endif /* HAVE_SOCKLEN_T */



/* Internal defines */
typedef enum {
 RD_CONT,			/* Waiting for more data */
 RD_ERR,			/* Error : connection should be cut */
 RD_FULL,			/* Buffer reached its max size */
 RD_CMDREADY,		/* We received a \n -> command is in buffer */
 RD_EOF				/* connection cut by the other end */
} read_state_t;


/* Set some global variables */
static fd_set current_fds, saved_fds, login_fds;  /* used by select()*/
static unsigned int fd_num;						  /* max fd value */


/* Idem for static functions */
static void usage(char *, int);
static read_state_t get_client_data(client_t *);
static void zombie(int);
static void hup_handler(int);
static void int_handler(int);
static void term_handler(int);
static void pipe_handler(int);
static void usr1_handler(int);
static int is_valid_ip(int);
static struct chat_client *register_login_in_progress(int);
static void write_pid(void);

/* Command-line options support for getopt */
static struct option long_options[] = {
	{"background",	0,	0,	'b'},
	{"config",		1,	0,	'c'},
	{"help",		0,	0,	'h'},
	{"log",			1,	0,	'l'},
	{"port",		1,	0,	'p'},
	{"version",		0,	0,	'v'},
	{0,             0,  0,  0}
};


/*
 * Displays version and usage informations
 */
static void usage(char* prog_path, int help)
{
	printf(_("Chat Everywhere, copyright(C)2000-2004, A. de Bernis <alexis@bernis.org>\n"));
	printf(_("Version %s, released %s\n"), CHAT_SERVER_VERSION, CHAT_SERVER_RELEASE);

	if(!help)
		return;

	printf(_("Usage : %s [Options]\n\n"), prog_path);

	printf(_("-b --background\t run the server into the background\n"));

	printf(_("-c --config file specifies an alternate config file\n"));
#ifdef DEFAULT_CONFIG_FILE
	printf(_("\t\t default is %s\n"), DEFAULT_CONFIG_FILE);
#endif /* DEFAULT_CONFIG_FILE */

	printf(_("-h --help\t print this help\n"));

	printf(_("-l --log file\t specifies an alternate logging file (overrides config file)\n"));
	printf(_("\t\t default is %s\n"), get_config_value("LogFile"));

	printf(_("-p --port num\t specifies the port to bind to (overrides config file)\n"));

	printf(_("-v --version\t print version informations\n"));
}


/*
 * All starts here...
 */
int main(int argc, char* argv[])
{
	int server_port = 0;
	int server_socket;	
	struct chat_client* tmp_client;
	char *buffer;
	char c;
	char *log_file = NULL;
	read_state_t r;
	char *config_path;
	
#ifdef ENABLE_NLS
	/* L10n and i18n... */
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif /* ENABLE_NLS */

	/* Chat server initialisation */
	signal(SIGCHLD, zombie);
	signal(SIGHUP, hup_handler);
	signal(SIGINT, int_handler);
	signal(SIGTERM, term_handler);
	signal(SIGPIPE, pipe_handler);
	signal(SIGUSR1, usr1_handler);
	config_path = NULL;

	/* parse command-line arguments */
	while((c = getopt_long(argc, argv, "bc:hl:p:v", long_options, NULL)) 
	 != EOF) {
		switch(c) {
			case 'b':
				if(fork())		/* The father exit() thus effectively      */
					exit(0);	/* getting the program into the background */
				break;

			case 'c':
				config_path = estrdup(optarg);
				break;

			case 'h':
				usage(argv[0],1);
				exit(0);
			
			case 'l':
				log_file = estrdup(optarg);
				break;
				
			case 'p':
				server_port = atoi(optarg);
				break;

			case 'v':
				usage(argv[0],0);
				exit(0);
			
			default:
				break;
		}
	}

	if(!config_path) { /* If not specified on the command-line */
		config_path = emalloc(strlen(DEFAULT_CONFIG_FILE) + 1);
		strcpy(config_path, DEFAULT_CONFIG_FILE);
	}
	config_file_init(config_path);
	read_auto_level_config();
	read_reserved_nick_config();
	init_statistics_module();
	restrict_command_init();

	/* Opening logging file */
	if(!log_file)  /* If not specified on the command-line */
		log_file = estrdup(get_config_value("LogFile"));

	if(init_logging(log_file) == -1) {
		fprintf(stderr, _("Error: failed to open or to create logging file,"
		  "logging directed to stderr\n"));
		log_handle = STDERR_FILENO;
	}
	efree(log_file);


	/* = Starting to listen on the port = */

    /* has it been given by user through command-line? */
	if(server_port == 0)
		server_port = atoi(get_config_value("ServerPort"));

	/*
	* Open a passive socket on port 'server_port' and wait
	* for connections
	*/
	if((server_socket = open_passive_socket(server_port)) == -1) {
		log(_("Fatal error : cannot open passive socket on port %d"), server_port);
		exit(1);
	}

	log(_("Starting chat server on port %d"), server_port);

	/* Global vars initialisation */
	FD_ZERO(&current_fds);
	FD_ZERO(&saved_fds);
	FD_ZERO(&login_fds);
	FD_SET(server_socket, &current_fds);
	fd_num = server_socket;
	first_client = NULL;
	first_ban = NULL;
	buffer = emalloc(BUF_SIZE + 1);

	/* Modules intialisation */
	write_pid();
	scheduler_init();
	password_auth_init();
	pingtimeout_init();
	refresh_users_file();
	commands_init();

	for(;;) {
		memcpy(&saved_fds, &current_fds, sizeof(current_fds)); 
		
		/* Main loop */
		if((select(fd_num + 1, &saved_fds, 0, 0, 0)) == -1) {
			if(errno == EINTR)
				continue;
						
			perror("Select");
			exit(1);
		}
		
		/* Is it a new connection ? */
		if(FD_ISSET(server_socket, &saved_fds))
			register_login_in_progress(server_socket);


		/* Now let us see if anybody sends us a message */
		tmp_client = first_client;
		while(tmp_client) {
			/* We save the next client now because it can be freed if the */
			/* client issued a QUIT command, or has been deconnected      */
			client_t *next_client = tmp_client->next;

			/* Has this client talked ? */
			if(!FD_ISSET(tmp_client->fd, &saved_fds)) {
				tmp_client = next_client;
				continue;
			}

			/* Yes! treat his command */
			r = get_client_data(tmp_client);
			if(r == RD_EOF || r == RD_ERR) {
				user_deconnect(tmp_client, "Read error : connection reset by peer");
			} else if(r == RD_FULL) {
				/* Buffer overflow attempt ? Reset its buffer */
				reset_recv_buffer(tmp_client);
				log(_("Error : received buffer overflowed by %s\n"), ip_of(tmp_client));
			} else if(r == RD_CMDREADY) {
				char *client_cmd = estrdup(tmp_client->recv_buffer);
			
				/* Treat the command */
				treat_command(client_cmd, tmp_client);
				
				/* and clean */
				reset_recv_buffer(tmp_client);
				efree(client_cmd);
			}

			/* Finished for this one, let's see the next */
			tmp_client = next_client;
		}
	}

	/* Never reached */
	return 0;
}

/* Read data until :
 *   - it blocks
 *   - we have a command ('\n')
 *   - an error occurs
 */
static read_state_t get_client_data(client_t *the_client) {
	int tmp;
	char c[2];
	read_state_t res;

	for(;;) {
		tmp = recv(the_client->fd, c, 1, MSG_DONTWAIT);
		
		if(tmp == 0) {
			res = RD_EOF;
		} else if(tmp == -1 && errno == EAGAIN) {
			res = RD_CONT;
		} else if(tmp == -1) {
			res = RD_ERR;
		} else if(the_client->recv_pos >= CLIENT_RCVBUF_MAX_SIZE) {
			res = RD_FULL;
		} else {
			/* We have to add a byte */
			/* do we need to increase the buffer size ? */
			if(the_client->recv_pos == the_client->recv_size - 1) {
				the_client->recv_buffer = erealloc(the_client->recv_buffer,
				 (the_client->recv_size + CLIENT_RCVBUF_INCREMENT) *
				 sizeof(*(the_client->recv_buffer)));
				the_client->recv_size += CLIENT_RCVBUF_INCREMENT;
			}

			/* Do we have a command ready ? */
			if(*c == '\n') {
				the_client->recv_buffer[the_client->recv_pos] = '\0';
				res = RD_CMDREADY;
			} else {
				the_client->recv_buffer[the_client->recv_pos++] = *c;
				continue;
			}
		}
		break;
	}

	return res;
}


/*
 * Close any zombie child
 */
static void zombie(int sig)
{
	signal(SIGCHLD, zombie);

	while(wait3(NULL, WNOHANG, NULL) > 0);
}


/*
 * Re-read the config file on SIGHUP
 */
static void hup_handler(int sig)
{
	signal(SIGHUP, hup_handler);
	
	log("%s", _("SIGHUP received, reading config file again"));
	config_file_reread();
	read_auto_level_config();
	read_reserved_nick_config();
	pingtimeout_config_changed();
	restrict_command_init();
	password_auth_reinit(); /* re-read password file */
}


/*
 * Try to quit nicely on SIGINT
 */
static void int_handler(int sig)
{
	// FIXME : dirty... access to a stat global variable
	if(use_global_stats)
		log(_("Terminating at user request (SIGINT), uptime %s"), seconds2ascii_compact(time(NULL) - stats->launch_time));
	else
		log(_("Terminating at user request (SIGINT)"));


	// MSG_TO_CLIENT
	send_generic_to_all(SEND_MSG, "Server has been shut down, deconnecting.");

	while(first_client)
		user_deconnect(first_client, "server shutdown");

	/* remove the users file list */
	remove_users_file();

	exit(0);
}


/*
 * Try to quit nicely on SIGTERM
 */
static void term_handler(int sig)
{
	// FIXME : dirty... access to a stat global variable
	if(use_global_stats)
		log(_("Terminating at system request (SIGTERM), uptime %s"), seconds2ascii_compact(time(NULL) - stats->launch_time));
	else
		log(_("Terminating at system request (SIGTERM)"));


	// MSG_TO_CLIENT
	send_generic_to_all(SEND_MSG, "Server has been shut down, deconnecting.");
	
	while(first_client)
		user_deconnect(first_client, "server shutdown");

	/* remove the users file list */
	remove_users_file();

	exit(0);
}


/*
 * Handles SIGPIPE message
 * a SIGPIPE message can occur if we send() to a client which has
 * already disconnected. This can occur if two clients cut their
 * connection in the same time, as it is processed sequentially
 * a DISCONNECTION message can be sent to a client which has already
 * closed its connection.
 * We juste make sure that the server does not stop on this.
 */
static void pipe_handler(int sig)
{
	signal(SIGPIPE, pipe_handler);
}

/*
 * Handles USR1 signals for debugging purposes
 */
static void usr1_handler(int sig)
{
	client_t *tmp_client = first_client;
	int n = 0;
	int l;
	
	signal(SIGUSR1, usr1_handler);

	while(tmp_client) {
		n++;
		l = tmp_client->login_complete;
		printf("Client %03d : %16s, %slogged, %s\n", n, ip_of(tmp_client),
		(l?"":"not "), (l?tmp_client->nick:""));

		tmp_client = tmp_client->next;
	}
	
	if(n == 0)
		printf("***  no client ***\n");
}


/* 
 * Parses a typical chat message: CLIENT <|> Comm <|> arg1 <|> Arg2 <|> CLIENT
 * and returns an array of char* containing 'expected' elements
 * or -1 if there is an error
 */
int parse_chat_message(char* command, char** parsed, int expected)
{
	int n = 0;
	char* tmps;
	char* pos;

	/* Gets number of elements in this message */
	pos = command-1;
	while((pos = strstr((char*) (pos+1), MSG_SEPARATOR)) != 0)
		n++;

	if(--n != expected)
		return(-1);

	tmps = estrdup(command);
	tmps[sizeof(CLIENT_PREFIX)-1] = 0;
	
	/* Is message starting by CLIENT ? */
	if(strcmp(tmps, CLIENT_PREFIX)) {
		efree(tmps);
 		return -1;
	}
	efree(tmps);

	command += sizeof(CLIENT_PREFIX) + sizeof(MSG_SEPARATOR) - 2;

	/* Ok, splitting the message in an array */
	n = 0;
	while((pos = strstr(command,MSG_SEPARATOR)) != 0) {
		int offset;
		
		offset=pos-command;
		tmps=emalloc(offset+1);
		tmps[offset]=0;

		memcpy(tmps,command,offset);
  		parsed[n++]=tmps;
		command+=offset+sizeof(MSG_SEPARATOR)-1;
	}

	/* Returns the number of elements in the array */
	return(n);	
}


/*
 * Returns the ban struct of the given nick or NULL
 * if not found
 *
 */
struct banishment* get_ban_by_nick(char* nick)
{
	struct banishment* tmp_ban = first_ban;
	
	while(tmp_ban) {
		if(!strcasecmp(tmp_ban->banned_nick, nick))
			return tmp_ban;

		tmp_ban = tmp_ban->next;
	}
	
	return NULL;
}



/*
 * is_valid_ip : verify if the connection is acceptable given
 * the user specification.
 * Uses directives : Order, AllowIP, DenyIP.
 * Returns :   1 if ip match
 *             0 else
 */
static int is_valid_ip(int ip)
{
	int i, n;
	char **tmpstr;
	int allowed_num, denied_num;
	int *allowed, *denied;
	int *allowed_sub, *denied_sub;
	int allow_first = 0;
	int match_allow = 0, match_deny = 0;

	/* Put all the allowed IPs into an array of int */

	tmpstr = tokenize(get_config_value("AllowIP"), ",", &n);
	allowed = emalloc(n * sizeof(int));
	allowed_sub = emalloc(n * sizeof(int));
	for(i = 0; i < n; i++) {
		char **parts;
		int j;

		parts = tokenize(tmpstr[i], "/", &j);
		allowed[i] = inet_addr(parts[0]);
		allowed_sub[i] = (j == 1) ? 32 : atoi(parts[1]);

		free_string_array(parts, j);
	}
	allowed_num = n;
	free_string_array(tmpstr, n);
	

	/* Do exactly the same for for denied IPs */

	tmpstr = tokenize(get_config_value("DenyIP"), ",", &n);
	denied = emalloc(n * sizeof(int));
	denied_sub = emalloc(n * sizeof(int));
	for(i = 0; i < n; i++) {
		char **parts;
		int j;		

		parts = tokenize(tmpstr[i], "/", &j);
		denied[i] = inet_addr(parts[0]);
		denied_sub[i] = (j == 1) ? 32 : atoi(parts[1]);

		free_string_array(parts, j);
	}
	denied_num = n;
	free_string_array(tmpstr, n);

	/* Check if allow is first in Order (else it must be deny
	 * no need to test it) */
	tmpstr = tokenize(get_config_value("Order"), ",", &n);
	if(!strcasecmp(tmpstr[0], "allow"))
		allow_first = 1;
	free_string_array(tmpstr, n);

	/* Check if the IP matches the rules */
	for(i = 0; i < allowed_num; i++)
		match_allow |= ip_match(allowed[i], ip, allowed_sub[i]);

	for(i = 0; i < denied_num; i++)
		match_deny |= ip_match(denied[i], ip, denied_sub[i]);

	efree(allowed);
	efree(allowed_sub);
	efree(denied);
	efree(denied_sub);

	if(allow_first)
		return(match_allow ? 1 : !match_deny);
	else
		return(match_deny ? 0 : match_allow);
}


/*
 * Returns 1 if the user has reached his maximum number
 * of allowed connection (based on his ip) and 0 else
 * This limit is defined in the config file by the
 * directive MaxUserConnection
 */
int too_many_connections(struct chat_client *client)
{
	struct chat_client *current = first_client;
	int n = 0;
	int limit = atoi(get_config_value("MaxUserConnections"));
	
	while(current) {
		if(current->login_complete &&
		   current->address->sin_addr.s_addr == client->address->sin_addr.s_addr)
			n++;
	
		current = current->next;
	}
	
	return (n == limit)? 1 : 0;
}


/* register_login_in_progress
 * register a new client who just connected.
 * returns a pointer on the client struct if it was ok
 * or NULL if an error happened
 */
static struct chat_client *register_login_in_progress(int socket_fd)
{
	struct chat_client *the_client;
	socklen_t in_addr;

	/* Fill a new client structure */
	the_client = new_client();
	in_addr = sizeof(*(the_client->address));
	the_client->address = emalloc(in_addr);

	/* Accept the connection */
	if((the_client->fd = accept(socket_fd,
	 (struct sockaddr*) the_client->address, &in_addr)) == -1) {
		log(_("Error in accept(), maybe a SYN scan"));
		
		efree(the_client->address);
		efree(the_client);
		return NULL;
	}

	/* Log the connection attempt */
	log(_("Connection from %s"), ip_of(the_client));

	/* Is it to be denied ? */
	if(!is_valid_ip(the_client->address->sin_addr.s_addr)) {
		log(_("Connection attempt from %s denied"), ip_of(the_client));
		close(the_client->fd);
		efree(the_client->address);
		efree(the_client);
		return(NULL);
	}

	/* Ok, we are now waiting for data */
	FD_SET(the_client->fd, &current_fds);
	if(the_client->fd > fd_num)
		fd_num = the_client->fd;

	/* And it is officially a client (but not yet logged in) */
	add_client(the_client);

	return the_client;
}



/* get_escape_table : returns the array of Escape structures
 * filled with the current values. See the escapes text file
 * in the doc subdirectory (or the man page).
 *
 * Note : the returned pointer must be freed!
 */
escape_t *get_escape_table(struct chat_client *the_client)
{
	escape_t *result;
	time_t now = time(NULL);

	result = emalloc(sizeof(escape_t) * 11);    /* Beware of the number! */

	result[0].escape = '%';
	result[0].sequence = estrdup("%");

	result[1].escape = 'n';
/* this inserts a \n is a message sent by the server... */
/*	result[1].sequence = estrdup("\n");          // BUG !!!! */
	result[1].sequence = estrdup(" ");

	result[2].escape = 't';
	result[2].sequence = estrdup("\t");
	
	result[3].escape = 'T';
	result[3].sequence = estrdup(cut_last_char(asctime(localtime(&now))));
	
	result[4].escape = 'E';
	result[4].sequence = estrdup(get_config_value("AdminEmail"));

	result[5].escape = 'R';
	result[5].sequence = estrdup(get_client_hostname(the_client->address));

	result[6].escape = 'L';
	result[6].sequence = estrdup(get_local_hostname());

	result[7].escape = 'U';
	result[7].sequence = estrdup(the_client->nick);

	result[8].escape = 'M';
	result[8].sequence = estrdup(get_config_value("MaxUsers"));

	result[9].escape = 'N';
	result[9].sequence = estrdup(itoa(count_users()));
	
	result[10].escape = '\0';
	result[10].sequence = NULL;

	return result;
}


/* send_motd_to : send the content of the Message Of The Day
 * file to the client
 */
void send_motd_to(struct chat_client *the_client)
{
	int motd_fd;
	const char *motd_file = get_config_value("MotdFile");
	char line[BUF_SIZE];
	escape_t *escape_table;
	char *escaped_message;
	
	if(strlen(motd_file) == 0)
		return;

	if((motd_fd = open(motd_file, O_RDONLY)) == -1) {
		perror("motd open");
		return;
	}
	
	escape_table = get_escape_table(the_client);
	while(read_line(motd_fd, line, BUF_SIZE) >= 0) {
		escaped_message = str_replace_escapes(line, escape_table);
/*		send_msg_to(the_client, escaped_message); */
		send_generic_to(the_client, SEND_MSG, escaped_message);
		efree(escaped_message);
	}
	free_escape_table(escape_table);

	close(motd_fd);
}



/* free_escape_table : free a table returned by get_escape_table
 */
void free_escape_table(escape_t *table)
{
	unsigned int i = 0;

	for(i = 0; table[i].escape != '\0'; i++)
		efree(table[i].sequence);

	efree(table);
}


/* write_pid_value : write the value of the PID into
 * the file given by the user
 */
void write_pid()
{
	const char *filepath;
	FILE *fd;
	pid_t my_pid = getpid();

	filepath = get_config_value("PidFile");
	if((fd = fopen(filepath, "w")) == NULL) {
		perror(filepath);
		return;
	}

	fprintf(fd, "%d\n", my_pid);
	fclose(fd);	
	log(_("Main process started with pid %d"), my_pid);
}


/* remove_fd : remove the given fd from the list of server
 * controlled file descriptors
 */
void remove_fd(int fd)
{
	FD_CLR(fd, &current_fds);
}
