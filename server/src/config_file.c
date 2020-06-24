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
/* Manages the configuration file */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "i18n.h"

#include "server_config.h"

#include "mem_utils.h"
#include "config_file.h"
#include "str_utils.h"
#include "fd_utils.h"

/* Internal Structures declarations */
typedef struct config_node {
	struct config_node*		next;
	
	char*					name;
	char*					value;
} config_node_t;


/* Static global variable : head of the config linked list */
static char *config_file_path = NULL;

/* Array of all the configuration directives */
static config_value_t server_config[] = {
	{ "AdminEmail",          "root",             NULL },
	{ "AllowIP",             "0.0.0.0/0",        NULL },
	{ "AllowGuestLogin",     "yes",              NULL },
	{ "AllowNickChar",       "",                 NULL },
	{ "AutoLevel",           "",                 NULL },
	{ "BeepOnLogin",         "no",               NULL },
	{ "DenyIP",              "0.0.0.0/32",       NULL },
	{ "DenyNickChar",        "",                 NULL },
	{ "ListenIP",            "0.0.0.0",          NULL },
	{ "LogFile",             "./chat.log",       NULL },
	{ "LogTalk",             "",                 NULL },
	{ "MaxUserConnections",  "-1",               NULL },
	{ "MaxUsers",            "-1",               NULL },
	{ "MotdFile",            "",                 NULL },
	{ "OpPass",              "password",         NULL },
	{ "Order",               "Allow, Deny",      NULL },
	{ "PasswordFile",        "./chat.passwords", NULL },
	{ "PidFile",             "/dev/null",        NULL },
	{ "PingTimeout",         "60",               NULL },
	{ "ReserveNick",         "",                 NULL },
	{ "RestrictCommand",     "",                 NULL },
	{ "ServerPort",          "5656",             NULL },
	{ "ServerStatsMinLevel", "9",                NULL },
	{ "Statistics",          "On",               NULL },
	{ "UserStatsMinLevel",   "9",                NULL },
	{ "UsersFile",           "",                 NULL },
	{ "UsersInfoMinLevel",   "0",                NULL },
	{ NULL,                  NULL,               NULL }   /* Ending token */
};



/* Private functions */
static void config_read(void);
static void add_config_node(const char *, const char *);
static void free_config(void);
static int  get_config_index(const char *);
static void dump_config(void);


/* config_file_init
 * Initialize the config file module by setting the path
 * of the config file and by reading it.
 * It needs to be called before any function of the config
 * file module.
 */
void config_file_init(char *config_path)
{
	config_file_path = config_path;
	config_file_reread();
}


/* config_file_reread
 * called when you need to re-read the config file
 * (typically on a SIGHUP from the user)
 */
void config_file_reread()
{
	free_config();
	config_read();
}



/*
 * config_read : opens the config file
 * and parses it to a vector of structs
 *
 * config_path is specified by the caller as it may defined on command
 * line. It needs to be set with set_config_path() before calling
 * config_read()
 */
static void config_read()
{
	int config_handle;
	char *buffer;
	unsigned int buffer_size;	
	unsigned int lineno = 0;
	char *d_name, *d_value;
	int dir_index;

	if(config_file_path == NULL)
		return;

	/* Step 1: open the config file */
	if((config_handle = open(config_file_path, O_RDONLY)) == -1) {
		perror(config_file_path);
		fprintf(stderr, _("Error : no config file found, default values will be used.\n"));
		return;
	}


	/* Step 2: allocate memory for a buffer */
	buffer = emalloc(BUF_SIZE);
	buffer_size = BUF_SIZE;

	/* Step 2: parse it */	
	for(;;) {
		if(read_line(config_handle, buffer, BUF_SIZE) < 0)
			break;

		/* Increment the line number */
		lineno++;

		/* Is it an empty line or a comment ? */
		if(buffer[0] == '\0' || buffer[0] == '#')
			continue;

		/* Is it a multi-line directive ? */
		while(buffer[strlen(buffer) - 1] == '\\') {
			char tmp[BUF_SIZE + 1];
			size_t size;

			if(read_line(config_handle, tmp, BUF_SIZE) < 0) {
				fprintf(stderr,
				 _("Error line %u : Last line of the config file is finishing by '\\'\n"),
				 lineno);
				return;
			}
			lineno++;

			/* Increase buffer size if needed */
			size = strlen(buffer) + strlen(tmp);
			if(size >= buffer_size) {
				buffer = erealloc(buffer, size + 1);
				buffer_size = size + 1;
			}

			/* Concatenate the lines */
			strcpy(buffer + strlen(buffer) - 1, tmp);
			buffer[size - 1] = 0;
		}

		/* Seems like a directive */
		/* Is it a valid definition or an empty one? */
		if(strstr(buffer, " ") == NULL) {
			d_name = str_clean(buffer);
			d_value = estrdup("");
		} else {
			d_name = str_clean(cut_to(buffer, " "));
			d_value = str_clean(cut_from(buffer, " "));
		}

		/* Check that the directive exists */
		dir_index = get_config_index(d_name);
		if(dir_index == -1) {
			fprintf(stderr, _("Error: directive \"%s\" unknown (line %d)\n"),
			 d_name, lineno);
		} else {

			/* Ok, adding it */
			add_config_node(d_name, d_value);
		}
		
		efree(d_name);
		efree(d_value);
	}

	close(config_handle);
	efree(buffer);
	/* FIXME : this is for debugging purposes */
	/* dump_config(); */
}


/* add_config_node : add a new config node as read by
 * config_read.
 * if a config_node with this name already exists, we
 * just concatenate the new value to the old one.
 */
static void add_config_node(const char *name, const char *value)
{
	int dir_index;
	char *p_value;

	/* check if there is already a node with this name */
	if((dir_index = get_config_index(name)) == -1)
		return;

	/* if it is the first time we see this directive, we
	 * add it to the beginning of the list, else we add the
	 * value to the previous one.
	 */
	p_value = server_config[dir_index].value;
	if(p_value != NULL) {
		p_value = erealloc(p_value, strlen(p_value) + strlen(value) + 2);
		strcat(p_value, ",");
		strcat(p_value, value);
		server_config[dir_index].value = p_value;
		
	} else {
		server_config[dir_index].value = estrdup(value);
	}
}


/*
 * efree() a config tree
 *
 */
static void free_config()
{
	int i;
	
	/* Parse all the config array to the ending token */
	for(i = 0; server_config[i].directive != NULL; i++) {
		efree(server_config[i].value);
		server_config[i].value = NULL;
	}
}


/* For debugging purposes, dump a given config tree */
void dump_config()
{
	int i;
	const char *name;
	const char *value;
	int def;
	
	/* Parse all the config array to the ending token */
	for(i = 0; server_config[i].directive != NULL; i++) {

		name = server_config[i].directive;
		value = get_config_value(name);
		def = (server_config[i].value == NULL) ? 1 : 0;

		printf("#%20s# = #%s#\t\t\t%s\n", name, value,
		 (def ? "default":""));
	}
}


/* Returns the value for the given token or a default if
 * not found */
const char *get_config_value(const char *name)
{
	int i;
	
	/* Parse all the config array to the ending token */
	for(i = 0; server_config[i].directive != NULL; i++) {

		if(!strcasecmp(server_config[i].directive, name))
			return (server_config[i].value == NULL) ?
			 server_config[i].default_value :
			 server_config[i].value;
	}

	fprintf(stderr, "Internal error: requested directive \"%s\" unknown\n",
	 name);

	return NULL;
}


/* get_config_node
 * returns the index of the config node
 * or -1 if this directive is unknown
 */
static int get_config_index(const char *directive)
{
	int i;
	
	/* Parse all the config array to the ending token */
	for(i = 0; server_config[i].directive != NULL; i++) {

		if(!strcasecmp(server_config[i].directive, directive))
			return i;
	}

	/* Not found */
	return -1;
}
