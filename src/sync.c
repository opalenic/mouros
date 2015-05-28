/*
 * sync.c
 *
 *  Created on: 28. 5. 2015
 *      Author: ondra
 */

#include <libopencm3/cm3/cortex.h>


#include <mouros/sync.h>
#include "scheduler.h"   // current_task & sched_add_to_runqueue_head


static void insert_waiting_task(struct resource *res) {

	struct tcb *task = res->first_waiting;

	if (task == NULL) {
		res->first_waiting = current_task;
		current_task->next_task = NULL;

	} else if (task->priority > current_task->priority) {

		res->first_waiting = current_task;
		current_task->next_task = task;

	} else {
		while (task != NULL) {
			struct tcb *next = task->next_task;

			if (next == NULL ||
			    next->priority > current_task->priority) {

				task->next_task = current_task;
				current_task->next_task = next;

				break;
			}

			task = next;
		}
	}
}


void os_resource_acquire(resource_t *res)
{
	while (true) {
		CM_ATOMIC_CONTEXT();

		if (res->acquired_by == NULL) {
			res->acquired_by = current_task;
			return;

		} else if (res->acquired_by == current_task) {
			return;

		} else {
			current_task->state = WAITING_FOR_RESOURCE;

			insert_waiting_task(res);

			os_task_yield();
		}
	}
}

void os_resource_release(resource_t *res)
{
	CM_ATOMIC_CONTEXT();

	if (current_task != res->acquired_by) {
		return;
	}

	res->acquired_by = NULL;

	struct tcb *first = res->first_waiting;

	if (first != NULL) {
		res->first_waiting = first->next_task;

		first->next_task = NULL;
		first->state = RUNNABLE;

		sched_add_to_runqueue_head(first);

		if (current_task->priority > first->priority) {
			os_task_yield();
		}

	}
}

