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
 * This is the header file for the string management
 * module.
 */


#ifndef _STR_UTILS_H_
#define _STR_UTILS_H_

/* Definitions */
#define ESCAPE_CHAR        '%'
#define ESCAPE_LENGTH      2     /* "%' + [a-zA-Z] */
#define ITOA_BUF_LENGTH    6

/* Structures */
typedef struct {
	char escape;
	char *sequence;
} escape_t;

/* Functions declarations */
extern char * cut_to(const char*, const char*);
extern char * cut_from(const char*, const char*);
extern char * str_clean(const char*);
extern char * str_upper(char*);
extern char ** tokenize(const char *, const char *, int *);
extern void free_string_array(char **, int);
extern char * str_pack(const char *);
extern int str_to_bool(const char *);
extern char * str_replace(const char *, const char *, const char *);
extern char * str_replace_escapes(const char *, const escape_t *);
extern char *itoa(const int);
extern char *cut_last_char(char *);

#endif
