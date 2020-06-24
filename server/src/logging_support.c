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
/* Supports the login file */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>  /* strlen */

#include "config_file.h"
#include "mem_utils.h"
#include "server_config.h"

#include "logging_support.h"


/* Statically used variables */
static int log_talk_fd;

/* Static variables */
static log_state_t log_talk = LOG_UNDEFINED;

/*
 * Initialize the logging facility (as of now : just
 * open the logging file.
 *
 */
int init_logging(char* logging_path)
{
	if((log_handle = open(logging_path, O_WRONLY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE)) == -1) {
		perror("init_logging");
		return(-1);
	}
	
	return(log_handle);
}


/*
 * Add a log entry (same usage as printf()).
 *
 */
void log(char* format, ...)
{
	struct tm *ac_time;
	time_t *timet = emalloc(sizeof(time_t));
	char* date = emalloc(30 * sizeof(char));
	char* buffer;
	char* entry = emalloc(BUF_SIZE);
	va_list ap;
	
	time(timet);
	ac_time = localtime(timet);
	sprintf(date,"[%02d/%02d/%04d %02d:%02d:%02d]",ac_time->tm_mday,ac_time->tm_mon+1,ac_time->tm_year+1900,ac_time->tm_hour,ac_time->tm_min,ac_time->tm_sec);

	if(entry) {
		va_start(ap, format);
		vsnprintf(entry, BUF_SIZE, format, ap);
	}
	
	buffer = emalloc((strlen(date) + strlen(entry) + 3) * sizeof(char));	
	sprintf(buffer,"%s %s\n", date, entry);
	write(log_handle, buffer, strlen(buffer));
	
	efree(buffer);
	efree(entry);
	efree(date);
	efree(timet);
}


/* init_log_to_talk : check if we must log the conversations,
 * and open accordingly a log file
 */
log_state_t init_log_to_talk(void)
{
	const char *log_path;

	log_path = get_config_value("LogTalk");

	if(log_path[0] == '\0')
		return LOG_OFF;

	if((log_talk_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, S_IREAD | S_IWRITE)) == -1) {
		perror("init_log_to_talk");
		return LOG_UNDEFINED;
	}

	return LOG_ON;
}

/* log_to_talk : log a talk message to the logtalk file
 */
void log_to_talk(const char *msg)
{
	char *log_msg;
	struct tm *ac_time;
	time_t timet;
	char date[30];    /* Real length is 20 but we'd better be safe ! */

	/* Activate the talk log if it has not been done */
	if(log_talk == LOG_UNDEFINED)
		log_talk = init_log_to_talk();

	/* just return if we do not actually log the chat */
	if(log_talk != LOG_ON)
		return;

	time(&timet);
	ac_time = localtime(&timet);
	sprintf(date,"[%02d/%02d/%04d %02d:%02d:%02d]", ac_time->tm_mday,
	 ac_time->tm_mon+1, ac_time->tm_year+1900, ac_time->tm_hour,
	 ac_time->tm_min, ac_time->tm_sec);

	log_msg = emalloc((strlen(date) + strlen(msg) + 6) * sizeof(*log_msg));
	sprintf(log_msg, "%s %s\n", date, msg);
	if(write(log_talk_fd, log_msg, strlen(log_msg)) == -1)
		perror("log_to_talk");

	efree(log_msg);
}
