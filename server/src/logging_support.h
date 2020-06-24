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
 * This is the header file for the logging module
 */


#ifndef _LOGGING_SUPPORT_H_
#define _LOGGING_SUPPORT_H_

/* Types */
typedef enum {
	LOG_OFF,
	LOG_ON,
	LOG_UNDEFINED
} log_state_t;

/* Functions declarations */
extern int init_logging(char*);
extern void log(char*, ...);
extern log_state_t init_log_to_talk(void);
extern void log_to_talk(const char *);

/* Global variables */
int log_handle;

#endif
