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
/* str_utils : tools to manipulate strings */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mem_utils.h"
#include "str_utils.h"

/* Static buffers */
static char itoa_buf[ITOA_BUF_LENGTH];


/*
 * Cut a string to the next delimiter
 * The new string is emalloc'ed
 */
char * cut_to(const char* the_string, const char* delimiter)
{
	char* w_string = estrdup(the_string);
	char* del_pos = strstr(w_string, delimiter);
	
	if(del_pos == NULL)
		return NULL;
		
	*del_pos = 0;
	w_string = erealloc(w_string, (strlen(w_string) + 1) * sizeof(char));
	
	return(w_string);
}


/*
 * Cut a string from the next delimiter to the end
 * The new string is emalloc'ed
 */
char * cut_from(const char* the_string, const char* delimiter)
{
	char* del_pos = strstr(the_string, delimiter);
	char* w_string = emalloc((strlen(the_string) + 1) * sizeof(char));
	
	if(del_pos == NULL)
		return NULL;
	
	strcpy(w_string, del_pos + strlen(delimiter));
	
	return(w_string);
}


/*
 * Clean a string by removing beginnings and endings
 * spaces and tabs
 * The new string is emalloc'ed
 */
char * str_clean(const char* the_string)
{
	const char* str_start = the_string;
	const char* str_end = the_string + (strlen(the_string) - 1);
	char* w_string;
	
	while(*str_start == ' ' || *str_start == '\t') str_start++;
	while(*str_end == ' ' || *str_end == '\t') str_end--;
	
	w_string = estrdup(str_start);
	w_string[str_end - str_start + 1] = 0;

	return(w_string);
}


/*
 * Convert a string into uppercase
 */
char * str_upper(char* the_string)
{
	unsigned int i;
	
	for(i = 0; i < strlen(the_string); i++)
		the_string[i] = (char) toupper(the_string[i]);
	
	return(the_string);
}



/*
 * tokenize : split the given string into an array of string
 * separated by the delimiter. Returns a pointer on this array
 * or NULL if missed. n will contain the number of elements in
 * the array
 */
char ** tokenize(const char *string, const char *delim, int *n)
{
	char **res = NULL;
	const char *nxt, *prev;
	char *tmp;
	int step = strlen(delim);
	int i;

	/* Check is there is something to tokenize */
	tmp = str_clean(string);
	if(!strcmp(tmp, ""))
	{
		*n = 0;
		efree(tmp);
		return NULL;
	}
	efree(tmp);
	
	/* Ok, go on */
	nxt = string - step;
	for(i = 0; strstr(nxt + step, delim) != NULL; i++)
	{
		char *tmpstr;

		res = erealloc(res, (i + 1) * sizeof(char *));
		prev = nxt + step;
		nxt = strstr(prev, delim);
		tmpstr = emalloc((nxt - prev + 1) * sizeof(char));
		memcpy(tmpstr, prev, nxt - prev);
		tmpstr[nxt - prev] = 0;
		res[i] = str_clean(tmpstr);
		efree(tmpstr);
	}

	res = erealloc(res, (i + 1) * sizeof(char *));
	res[i] = str_clean(nxt + step);

	*n = i + 1;

	return res;
}


/*
 * free_string_array : efree an array of string
 */
void free_string_array(char **array, int n)
{
	int i;

	for(i = 0; i < n; i++)
		efree(array[i]);

	efree(array);
}


/*
 * condensate : condensate a string by replacing multiple
 * tabs or space by only one space
 * The returned string is emalloc'ed
 */
char * str_pack(const char * string)
{
	int isspace = 0;
	int i, p = 0;
	int l = strlen(string);
	char * res = estrdup(string);
	
	for(i = 0; i < l; i++)
	{
		if(string[i] == ' ' || string[i] == '\t')
		{
			if(isspace)
				continue;
			else
				isspace = 1;
		} else
			isspace = 0;

		res[p++] = string[i];
	}

	res[p] = 0;
	res = erealloc(res, (p + 1) * sizeof(char));

	return res;
}



/*
 * str_to_bool : converts [Yy][Ee][Ss]/[Nn][Oo] into boolean
 */
int str_to_bool(const char *str)
{
	if(!strcasecmp(str, "yes"))
		return 1;
		
	return 0;
}


/*
 * str_replace : recursively replace a string by another
 * into a given string.
 * The returned string is emalloc'ed
 */
char * str_replace(const char *the_string, const char *from,
 const char *to)
{
	char *pos, *res, *prev_part, *next_part;

	if((pos = strstr(the_string, from)) == NULL)
		return estrdup(the_string);
	
	prev_part = cut_to(the_string, from);
	next_part = str_replace(pos + strlen(from), from, to);
	
	res = (char *) emalloc((strlen(prev_part) + strlen(to) + strlen(next_part) +
	 1) * sizeof(char));
	strcpy(res, prev_part);
	strcpy(res + strlen(prev_part), to);
	strcpy(res + strlen(prev_part) + strlen(to), next_part);
	
	efree(prev_part);
	efree(next_part);
	return res;
}

/* str_replace_escapes : Replaces escape sequences, starting
 * by '%' by its equivalent in the array given in parameter.
 *
 * Note : the last element of the array must be a 0/NULL
 * escape struct
 *
 * Warning : the returned pointer must be freed!
 */
char *str_replace_escapes(const char *from, const escape_t *escapes)
{
	char *result = emalloc(1);
	char *pos_in_from, *new_pos_in_from;
	unsigned int i;
	unsigned int res_size = 1;

	pos_in_from = (char *) from;
	
	while((new_pos_in_from = strchr(pos_in_from, ESCAPE_CHAR)) != NULL) {
		result = erealloc(result, res_size + new_pos_in_from - pos_in_from);
		(void) memcpy(result + res_size - 1, pos_in_from,
		               new_pos_in_from - pos_in_from);
		res_size += new_pos_in_from - pos_in_from;

		for(i = 0; escapes[i].escape != '\0'; i++) {
			if(new_pos_in_from[1] == escapes[i].escape) {
				result = erealloc(result, res_size
				                          + strlen(escapes[i].sequence));
				memcpy(result + res_size - 1, escapes[i].sequence,
				       strlen(escapes[i].sequence));
				res_size += strlen(escapes[i].sequence);
				break;
			}
		}
		if(escapes[i].escape == '\0')
			fprintf(stderr, "Error, invalid escape character %c\n",
			        new_pos_in_from[1]);

		pos_in_from = new_pos_in_from + ESCAPE_LENGTH;
	}
	new_pos_in_from = (char *) from + strlen(from);
	result = erealloc(result, res_size + new_pos_in_from - pos_in_from);
	(void) memcpy(result + res_size - 1, pos_in_from,
	               new_pos_in_from - pos_in_from);

	return result;
}


/* itoa : returns a pointer toward a statically allocated
 * buffer containing the ascii representation of the given
 * integer.
 */
char *itoa(const int i)
{
	if(snprintf(itoa_buf, ITOA_BUF_LENGTH, "%d", i) == -1) {
		fprintf(stderr, "Error : value of ITOA_BUF_LENGTH is too small, "
		                "increase it in str_utils.h\n");
	}
	
	return itoa_buf;
}


/* cut_last_char : drop the last character of the given
 * string.
 *
 * Note : the string is directly modified!
 */
char *cut_last_char(char *s)
{
	if(strlen(s) < 1)
		return s;

	s[strlen(s) - 1] = '\0';
	
	return s;
}
