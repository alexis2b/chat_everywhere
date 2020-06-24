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
/* Various files-related functions
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "fd_utils.h"

/* Reads a line ending by '\n' or after max_bytes */
int read_line(const int fd, char* buffer, const unsigned int max_bytes)
{
	unsigned int n = 0;
	char c[2];
	int tmp;
	
	while(n != max_bytes) {
		if((tmp = read(fd, c, 1)) != 1) {
			if(!tmp)
				return RDERR_EOF;
			return RDERR_ERROR;
		}

		if(*c == '\n') {
			buffer[n] = 0;
			return n;
		}
		buffer[n++] = *c;
	}
	return RDERR_NOSPACE;
}
