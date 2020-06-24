/* scheduler.c
 * copyright(c)2004, A. de Bernis, <alexis@bernis.org>
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
/* scheduler
 * general module to trigger alarms at specific time
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdlib.h>
#include <unistd.h>   /* alarm */
#include <signal.h>   /* signal, SIGALRM */
#include <time.h>     /* time */

#include "scheduler.h"
#include "mem_utils.h"

/* Internal defines */
#ifndef SCHEDULER_GRANULARITY
# define SCHEDULER_GRANULARITY 5
#endif


/* internal structures */
typedef struct sched_task {
	struct sched_task  *next;      /* next struct */

	unsigned int task_id;          /* task id */

	unsigned long alarm_time;      /* when to trigger it */
	void (*alarm_handler)(void *); /* function to call */
	void *alarm_data;              /* data to pass to the function */

	unsigned long delay;           /* delay between alarms */
	unsigned int repeat;           /* is it to be repeated ? */
} sched_task_t;


/* Set some global variables */
static sched_task_t *first_sched_task = NULL;


/* Idem for static functions */
static void sched_alarm_handler(int);
static void remove_sched_task(sched_task_t *);


/* scheduler_init
 * to be called at the beginning to set the alarms
 */
void scheduler_init()
{
	/* initiate the alarms */
	if(SCHEDULER_GRANULARITY != 0) { /* scheduler can be disabled */
		signal(SIGALRM, sched_alarm_handler);
		alarm(SCHEDULER_GRANULARITY);
	}
}


/* sched_alarm_handler
 * called every SCHEDULER_GRANULARITY seconds with a SIGALRM signal
 */
static void sched_alarm_handler(int sig)
{
	execute_scheduled_tasks();

	/* Reset the alarm */
	if(SCHEDULER_GRANULARITY != 0) { /* scheduler can be disabled */
		signal(SIGALRM, sched_alarm_handler);
		alarm(SCHEDULER_GRANULARITY);
	}
}



/* execute_scheduled_tasks
 * execute the tasks that needs to be done by the time
 */
void execute_scheduled_tasks()
{
	sched_task_t *current_task = first_sched_task;
	sched_task_t *next_task;
	unsigned long now = time(NULL);
	
	while(current_task != NULL) {
		/* we save it because current can be removed */
		next_task = current_task->next;

		/* do we have to execute it */
		if(current_task->alarm_time <= now) {
			(current_task->alarm_handler)(current_task->alarm_data);

			/* remove it or reschedule it */
			if(current_task->repeat != 0) {
				current_task->alarm_time = now + current_task->delay;
			} else {
				remove_sched_task(current_task);
			}
		}
		
		/* iterate */
		current_task = next_task;
	}
}



/* remove_sched_task
 * remove the given taks from the linked list
 */
static void remove_sched_task(sched_task_t *task)
{
	sched_task_t *cur = first_sched_task;

	if(task == first_sched_task) {
		first_sched_task = task->next;
	} else {
		while(cur != NULL) {
			if(task == cur->next) {
				cur->next = task->next;
				break;
			}
		}
	}

	efree(task);
}

/* register_alarm
 * called by an external module to register a task that must be executed
 * at a given time.
 *    id      task id (to cancel or remove it)
 *    delay   how many seconds before we execute it
 *    handler function to call when the alarm sets off
 *    data    a data pointer that we pass to the handler
 *    repeat  must the alarm be repeated every "delay" seconds ? (0 or 1)
 */
void register_alarm(
 unsigned int id,
 unsigned long delay,
 void (*handler)(void *),
 void *data,
 unsigned int repeat)
{
	sched_task_t *alrm;
	
	/* creates the structure */
	alrm = emalloc(sizeof(*alrm));
	alrm->task_id = id;
	alrm->alarm_time = time(NULL) + delay;
	alrm->alarm_handler = handler;
	alrm->alarm_data = data;
	alrm->delay = delay;
	alrm->repeat = repeat;


	/* add it to the tree */
	alrm->next = first_sched_task;
	first_sched_task = alrm;
}


/* cancel_alarm
 * cancel all the tasks defined by the given task_id
 */
void cancel_alarm(unsigned int id)
{
	sched_task_t *cur, *next;
	
	cur = first_sched_task;
	while(cur != NULL) {
		next = cur->next;
		
		if(cur->task_id == id)
			remove_sched_task(cur);

		cur = next;
	}
}
