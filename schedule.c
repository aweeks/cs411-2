/*
 *  Project 2
 *    A shortest job remaining scheduler.
 *   
 *  Authors:
 *    Alex Weeks    
 *    Josh Jordahl  
 *    Kevin McIntosh
 *    Tyler McClung
 */

/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEWTASKSLICE (NS_TO_JIFFIES(100000000))

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *			 There are HZ jiffies in a second, (HZ is 
 *			 declared in macros.h), and is usually
 *			 1 or 10 milliseconds.
 * 
 * Singular - jiffy.  A tick is simply and simply represents the min 
 * amount of time
 */
extern long long jiffies;

void printqueue() 
{
	#ifdef DEBUG
		printf("ACTIVE QUEUE:\n");

		struct sched_array *tmp;
		list_for_each_entry(tmp, &rq->active->list, list) {
			printf("    %x timeslice: %u\n", (unsigned int) tmp->task, tmp->task->time_slice);
		}
	#endif
}

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
 /* initscheduler
  * Sets up and allocates memory for the scheduler, as well
  * as sets initial values. This function should also
  * set the initial effective priority for the "seed" task 
  * and enqueu it in the scheduler.
  * INPUT:
  * newrq - A pointer to an allocated rq to assign to your
  *			local rq.
  * seedTask - A pointer to a task to seed the scheduler and start
  * the simulation.
  */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask)
{
	rq = newrq;
	rq -> active  = (struct sched_array *) malloc( sizeof(struct sched_array) );
	INIT_LIST_HEAD( &rq->active->list );

	rq -> expired = (struct sched_array *) malloc( sizeof(struct sched_array) );
	INIT_LIST_HEAD( &rq->expired->list );

	enqueue_task(seedTask, rq->active);

	seedTask->time_slice = seedTask->first_time_slice = NEWTASKSLICE;

	rq->nr_running++;
	//schedule();
}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule()
{
	//struct list_head *pos = rq->active->list; 	  
	struct sched_array *tmp,*next;	
	list_for_each_entry_safe(tmp,next, &(rq->active->list),list)
	{
		free(&tmp->list); //not sure about freeing task_structs
	}	 
	list_for_each_entry_safe(tmp,next, &(rq->expired->list), list)
	{
		free(&tmp->list);
	}
	free(rq->active);
	free(rq->expired);
}

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task with the shortest runtime(time slice) remaining
 */
void schedule()
{
	if (rq->nr_running > 0)
	{
		struct sched_array *tmp, *new = list_entry(rq->active->list.next, struct sched_array, list);

		// The process didn't finish yet, but its time slice is expired; Reset the time slice.
		if (rq->curr != NULL)
		{
			if( rq->curr->time_slice == 0 )
			{
				rq->curr->time_slice = rq->curr->first_time_slice;
			}
			rq->curr->need_reschedule = 0;
			
			#ifdef DEBUG
				printf("MOVING TO BACK\n");
			#endif

			printqueue();
			list_del( &rq->curr->array->list);
			printqueue();
			list_add_tail(&rq->curr->array->list, &rq->active->list);	
			//printqueue();

			// Find the shortest job remaining and run it.
			list_for_each_entry(tmp, &(rq->active->list), list)
			{
				if (new->task->time_slice > tmp->task->time_slice)
				{
					new = tmp;
				}
			}
		}
        	#ifdef DEBUG
			printf("SCHEDULE %x\n", (unsigned int) new->task);
		#endif
		if(rq->curr != new->task )
		{
			context_switch(new->task);
			rq->curr = new->task;
			rq->nr_switches++;
		}
	}
}

/* enqueue_task
 * Enqueues a task in the passed sched_array
 */
void enqueue_task(struct task_struct *p, struct sched_array *array)
{
	#ifdef DEBUG
		printf("ENQUEUE %x\n", (unsigned int)  p);
	#endif
	struct sched_array *new = (struct sched_array *) malloc( sizeof(struct sched_array) );

	p->array = new;
	new->task = p;
	list_add( &new->list, &array->list );
	
	printqueue();
}

/* dequeue_task
 * Removes a task from the passed sched_array
 */
//You will likely not call this because you are using only one array.
//Call when the process is dead
void dequeue_task(struct task_struct *p, struct sched_array *array)
{
	#ifdef DEBUG
		printf("DEQUEUE %x\n", (unsigned int) p);
	#endif
	
	// If the process we're removing is the current process, make sure to remove the reference.
	if (rq->curr = p)
	{
		rq->curr = NULL;
	}
	
	list_del( &(p->array->list) );
	//rq->nr_running--;
	printqueue();
}

/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void sched_fork(struct task_struct *p)
{
	#ifdef DEBUG
		printf("FORK %x\n", (unsigned int) p);
	#endif

	// To prevent loss of odd timeslices on fork, add one to child (before bitshift)
	p->first_time_slice = rq->curr->first_time_slice; 
	p->time_slice = ( rq->curr->time_slice + 1 ) >> 1;
	rq->curr->time_slice >>= 1;
	if (rq->curr->time_slice <= 0)
	{
		rq->curr->time_slice = rq->curr->first_time_slice;
	}
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{	
	#ifdef DEBUG	
		printf("TICK %x\n", (unsigned int) p);
	#endif
	p->time_slice--;
	if ( p->time_slice <= 0 )
	{
		p->need_reschedule = 1;
	}
}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 * Also handles preemption, e.g. decides 
 * whether or not the current task should
 * call scheduler to allow for this one to run
 */
void wake_up_new_task(struct task_struct *p)
{
	#ifdef DEBUG
		printf("WAKE %x\n", (unsigned int) p );
	#endif

	enqueue_task(p, rq->active);
	rq->nr_running++;		
}

/* __activate_task
 * Activates the task in the scheduler
 * by adding it to the active array.
 * 
 * Do bookkeeping here
 */
void __activate_task(struct task_struct *p)
{
	#ifdef DEBUG	
		printf("ACTIVATE %x\n", (unsigned int) p);
	#endif
    
	enqueue_task(p,rq->active);
	rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{	
    __activate_task(p);	
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{
	#ifdef DEBUG	
		printf("DEACTIVATE %x\n", (unsigned int) p);
	#endif
	
	dequeue_task(p,NULL);
	rq->nr_running--;
}

long long unsigned get_timestamp()
{
	time_t ltime; /* calendar time */
	ltime = time(NULL); /* get current cal time */
	return ltime;
}
