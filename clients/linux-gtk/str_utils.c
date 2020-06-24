/* str_utils : tools to manipulate strings */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "str_utils.h"


/*
 * Cut a string to the next delimiter
 */
char* cut_to(char* the_string, const char* delimiter)
{
	char* w_string = strdup(the_string);
	char* del_pos = strstr(w_string, delimiter);
	
	if(del_pos == NULL)
		return NULL;
		
	*del_pos = 0;
	w_string = realloc(w_string, (strlen(w_string) + 1) * sizeof(char));
	
	return(w_string);
}


/*
 * Cut a string from the next delimiter to the end
 */
char* cut_from(char* the_string, const char* delimiter)
{
	char* del_pos = strstr(the_string, delimiter);
	char* w_string = malloc((strlen(the_string) + 1) * sizeof(char));
	
	if(del_pos == NULL)
		return NULL;
	
	strcpy(w_string, del_pos + strlen(delimiter));
	
	return(w_string);
}


/*
 * Clean a string by removing beginnings and endings
 * spaces and tabs
 */
char* str_clean(char* the_string)
{
	char* str_start = the_string;
	char* str_end = the_string + (strlen(the_string) - 1);
	char* w_string;
	
	while(*str_start == ' ' || *str_start == '\t') str_start++;
	while(*str_end == ' ' || *str_end == '\t') str_end--;
	
	w_string = strdup(str_start);
	w_string[str_end - str_start + 1] = 0;

	return(w_string);
}


/*
 * Convert a string into uppercase
 */
char* str_upper(char* the_string)
{
	int i;
	
	for(i = 0; i < strlen(the_string); i++)
		the_string[i] = (char) toupper(the_string[i]);
	
	return(the_string);
}
