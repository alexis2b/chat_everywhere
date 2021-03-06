/* Chat Everywhere
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
 * This is the include file for the config file management
 * module.
 */


#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

/* Types declaration */
typedef struct {
	const char *directive;          /* name of the directive */
	const char *default_value;      /* default value */
	char       *value;              /* value set by user */
} config_value_t;


/* Functions declarations */
extern void  config_file_init(char *);
extern void  config_file_reread(void);
extern const char *get_config_value(const char *);

#endif
