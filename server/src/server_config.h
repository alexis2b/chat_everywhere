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
 * Compile-time config file for chat_server
 * You may change some default values, then recompile
 * all the package with 'make'
 */

#ifndef CONFIG_H
#define CONFIG_H


/*
 * Most of the default values can be overriden in
 * the config file
 */


/* Default command restrictions */
#define DEFAULT_BAN_RES       9
#define DEFAULT_GAG_RES       9
#define DEFAULT_USERSINFO_RES 9


/* These are not configurable but does not really matter */
#define SERVER_BACK_LOGS        5
#define BUF_SIZE                2048
#define MAX_NICK_LENGTH         14
#define CLIENT_PREFIX           "CLIENT"
#define MSG_SEPARATOR           " <|> "
#define MAX_INET_NAME_LENGTH    256
#define MIN_USER_LEVEL          0
#define MAX_USER_LEVEL          9

/* memory management of a client's reception buffer */
#define CLIENT_RCVBUF_START_SIZE 10
#define CLIENT_RCVBUF_INCREMENT  20
#define CLIENT_RCVBUF_MAX_SIZE   2048

/* Scheduler configuration */
#define SCHEDULER_GRANULARITY 5
#define BAN_TIMER_DELAY       60
#define GAG_TIMER_DELAY       60
#define TASK_PING_TIMEOUT     101
#define TASK_BAN_TIMER        102
#define TASK_GAG_TIMER        103


#endif  /* CONFIG_H */
