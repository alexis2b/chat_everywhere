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
/* Users management */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>           /* snprintf */
#include <time.h>            /* time */
#include <sys/types.h>       /* needed */
#include <sys/stat.h>        /*   for  */
#include <fcntl.h>           /*  open  */
#include <string.h>          /* strchr */
#include <unistd.h>          /* close  */

#include "i18n.h"

#include "server_config.h"
#include "communication.h"
#include "config_file.h"
#include "fd_utils.h"
#include "password_auth.h"
#include "md5.h"
#include "mem_utils.h"
#include "str_utils.h"

/* private vars */
static protected_nick_t *first_protected = NULL;

/* private functions */
static void generate_challenge(char *);
static void password_read_file(void);
static char *get_nick_hash(const char *);
static unsigned int get_nick_level0(const char *);
static void send_challenge(struct chat_client *);
static void add_protected_nick(char *, char *, unsigned int);
static void free_protected_nicks(void);

/* various defines */
#define FIELD_SEPARATOR ":"


/* password_auth_init
 * Initialize the module password_auth
 */
void password_auth_init()
{
	/* Seed the random number generator */
	srand(time(NULL));

	/* Read the password file */
	password_read_file();
}


/* password_auth_reinit
 * re-read the password file (asked by the user)
 * on a HUP signal
 */
void password_auth_reinit()
{
	/* removes all the passwords */
	free_protected_nicks();

	/* add them all again */
	password_read_file();
}


/* generate_challenge
 * print a new challenge in the buffer
 * Buffer must be at least 33 bytes long !
 */
static void generate_challenge(char *buffer)
{
	char to_hash[BUF_SIZE];
	
	snprintf(to_hash, BUF_SIZE - 1, "%u%u%u%u", rand(), rand(), rand(), rand());
	MD5_hash_string(to_hash, buffer);
}


/* add_protected_nick
 * add the given nick/hash combination to the list of protected nicks
 */
static void add_protected_nick(char *nick, char *hash, unsigned int level0)
{
	protected_nick_t *new_token;
	protected_nick_t *current_token = first_protected;
	
	new_token = emalloc(sizeof(*new_token));
	new_token->next = NULL;
	new_token->nick = estrdup(nick);
	new_token->hash = estrdup(hash);
	new_token->level0 = level0;

	if(first_protected == NULL) {
		first_protected = new_token;
	} else {
		while(current_token->next)
			current_token = current_token->next;
		current_token->next = new_token;
	}
}

/* free_protected_nick
 * removes all the tree of protected nicks
 */
static void free_protected_nicks()
{
	protected_nick_t *next;

	while(first_protected) {
		next = first_protected->next;
		
		/* Free the protected nick struct */
		efree(first_protected->nick);
		efree(first_protected->hash);
		efree(first_protected);
		
		/* Next one ! */
		first_protected = next;
	}
}


/* password_read_file : read the password file
 */
static void password_read_file()
{
	const char *file_path;
	int fd;
	char buffer[BUF_SIZE];
	char **tokens;
	int token_num;
	unsigned int lineno = 0;

	file_path = get_config_value("PasswordFile");

	if((fd = open(file_path, O_RDONLY)) == -1) {
		perror(file_path);
		return;
	}
	
	for(;;) {
		if(read_line(fd, buffer, BUF_SIZE - 1) < 0)
			break;

		/* Increment the line number */
		lineno++;

		/* Empty lines and comments are ignored */
		if(buffer[0] == '\0' || buffer[0] == '#')
			continue;

		/* tokenize but ignore blank line */
		if((tokens = tokenize(buffer, FIELD_SEPARATOR, &token_num)) == NULL)
			continue;

		/* 1st part: the nick to protect */
		switch(token_num) {
			case 2:
				fprintf(stderr,
				 _("Warning: no initial level specified for %s in password file (line %d)\n")
				 , tokens[0], lineno);
				add_protected_nick(tokens[0], tokens[1], 0);
				break;

			case 3:
				add_protected_nick(tokens[0], tokens[2], atoi(tokens[1]));
				break;
			
			default:
				fprintf(stderr,
				 _("Warning: syntax error in password file line %d\n"), lineno);
				break;
		}
		free_string_array(tokens, token_num);
	}

	close(fd);
}


/* authentify_user
 * performs all the step needed to authentify an user
 * if p_hash = NULL -> try to auth as a guest (no auth)
 * else try a password auth
 */
auth_t authentify_user(struct chat_client *the_client, const char *p_hash)
{
	char to_hash[BUF_SIZE];
	char s_hash[BUF_SIZE];
	

	/* Guest auth */
	if(p_hash == NULL || the_client->challenge == NULL) {
		if(get_nick_hash(the_client->nick) == NULL)
			return AUTH_FREE;

		send_challenge(the_client);
		return AUTH_ASKED;
	}

	/* Auth verification */
	snprintf(to_hash, BUF_SIZE, "%s%s", get_nick_hash(the_client->nick),
	 the_client->challenge);
	MD5_hash_string(to_hash, s_hash);

	/* Clean the useless stuff */
	efree(the_client->challenge);
	the_client->challenge = NULL;
	the_client->auth_asked = 0;

	/* Auth failed */
	if(strcmp(s_hash, p_hash)) {
		the_client->auth_fails += 1;
		return AUTH_FAILED;
	}
	
	/* Auth successful */
	the_client->authentified = 1;
	the_client->level = get_nick_level0(the_client->nick);
	return AUTH_SUCCESS;
}

/* get_nick_hash
 * return the hashed password if the user nick is protected
 * NULL otherwise
 *
 * FIXME : need to add MySQL support here to get the password
 * from a DB if needed
 */
static char *get_nick_hash(const char *nick)
{
	protected_nick_t *current_token = first_protected;
	
	while(current_token) {
		if(!strcasecmp(nick, current_token->nick))
			return current_token->hash;
		
		current_token = current_token->next;
	}
	
	return NULL;
}


/* get_nick_level0
 * return the initial level if the user nick is protected
 * or 0 otherwise
 */
static unsigned int get_nick_level0(const char *nick)
{
	protected_nick_t *current_token = first_protected;
	
	while(current_token) {
		if(!strcasecmp(nick, current_token->nick))
			return current_token->level0;
		
		current_token = current_token->next;
	}
	
	return 0;
}


/* send_challenge
 * send a challenge auth request to the client and update its info
 * accordingly
 */
static void send_challenge(struct chat_client *the_client)
{
	char challenge[BUF_SIZE];

	generate_challenge(challenge);
	send_generic_to(the_client, SEND_AUTH, "%s", challenge);
	the_client->challenge = estrdup(challenge);
	the_client->auth_asked = 1;
}


