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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>         /* memset, strlen, strcpy */

#include "i18n.h"

#include "mem_utils.h"

static size_t used_mem = 0;

void *emalloc(size_t size)
{
	void *result;
	
	if((result = malloc(size + sizeof(size_t))) == NULL) {
		fprintf(stderr, _("Error : no more memory\n"));
		exit(EXIT_FAILURE);
	}
	
	(void) memset(result, 0, size + sizeof(size_t));
	*((size_t*) result) = size;
	used_mem += size;

#ifdef MEMDEBUG
	printf(_("Successfully allocated %u bytes (total is now : %u)\n"), size,
	       used_mem);
#endif

	return (result + sizeof(size_t));
}


void efree(void *ptr)
{
	size_t size;
	void *orig_p;

	if(ptr == NULL) {
#ifdef MEMDEBUG
		printf(_("Warning : efree(NULL) called"));
#endif
		return;
	}
	
	orig_p = ptr - sizeof(size_t);
	size = *((size_t *) orig_p);
	used_mem -= size;
	(void) memset(orig_p, 0, size + sizeof(size_t));
	
	free(ptr - sizeof(size_t));
#ifdef MEMDEBUG
	printf(_("Successfully freed %u bytes (total is now : %u)\n"), size, used_mem);
#endif /* MEMDEBUG */
}



void *erealloc(void *ptr, size_t size)
{
	void *result, *orig_p;
	size_t orig_s;

	if(ptr == NULL) {
		orig_p = NULL;
		orig_s = 0;
	} else {
		orig_p = ptr - sizeof(size_t);
		orig_s = *((size_t *) orig_p);
	}

	result = realloc(orig_p, size + sizeof(size_t));

	if(size > orig_s)	
		(void) memset(result + sizeof(size_t) + orig_s, 0, size - orig_s);
	
	*((size_t *) result) = size;
	used_mem += size - orig_s;

#ifdef MEMDEBUG
	printf(_("Successfully reallocated %u to %u bytes (total is now : %u)"), orig_s, size,
	       used_mem);
#endif

	return (result + sizeof(size_t));
}


/* estrdup : strdup extended to use efree/emalloc
 */
char *estrdup(const char *s)
{
	char *result = NULL;
	
	/* strlen(NULL) -> SIGSEGV (thanks to Jordan) */
	if(!s)
		return result;
	
	result = emalloc((strlen(s) + 1) * sizeof(*result));
	
	strncpy(result, s, strlen(s));
	
	return result;
}
