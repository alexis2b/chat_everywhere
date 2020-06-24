/* Various files-related functions */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "fd_utils.h"

/* Reads a line ending by '\n' or after max_bytes */
int read_line(int fd, char* buffer, int max_bytes)
{
	int n=0;
	char* c;
	int tmp;
	
	c=malloc(2);
	while(n!=max_bytes)
	{
		if((tmp=read(fd,c,1))!=1)
		{
			free(c);
			if(!tmp)
				return(-3);
			return(-1);
		}

		if(*c=='\n')
		{
			buffer[n]=0;
			free(c);
			return(n);
		}
		
		buffer[n++]=*c;
	}
	free(c);
	return(-2);
}
