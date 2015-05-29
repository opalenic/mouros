/**
 * @file
 *
 * This file holds the implementation of the MourOS resource facility.
 *
 */

#include <libopencm3/cm3/cortex.h> // CM_ATOMIC_*

#include <mouros/sync.h> // Function and struct declarations.
#include "scheduler.h"   // current_task & sched_add_to_runqueue_head


/**
 * Adds the current task to the linked list of task waiting for res to be
 * available. The task is inserted into the list based on its priority. A higher
 * priority task will be placed before a lower priority task.
 *
 * @param res Pointer to the resource the current task should wait for.
 */
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

