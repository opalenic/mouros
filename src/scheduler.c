/*
 * scheduler.c
 *
 *  Created on: 28. 4. 2015
 *      Author: ondra
 */

#include <libopencm3/cm3/scb.h>  // The system control block defines
#include <libopencm3/cm3/nvic.h> // nvic_* functions & defines

#include "scheduler.h"


#include "stack_m0.h" // Stack popping and pushing functions.

#define NUM_PRIO_LEVELS 16


static struct task_group task_prio_groups[NUM_PRIO_LEVELS];

static struct tcb *sleepqueue_head = NULL;


struct tcb *current_task = NULL;

uint64_t os_tick_count = 0;

static uint8_t highest_prio_level = 0;


static struct tcb *take_highest_prio_task(void)
{
	for (uint8_t i = highest_prio_level; i < NUM_PRIO_LEVELS; i++) {
		highest_prio_level = i;
		struct tcb *task = task_prio_groups[i].first;

		if (task != NULL) {
			task_prio_groups[i].first = task->next_task;
			if (task->next_task == NULL) {
				task_prio_groups[i].last = NULL;
			}

			return task;
		}
	}

	// if we got here, something went very wrong
	while (true);
	return NULL;
}

static void wakeup_tasks(void)
{
	struct tcb *sleeping = sleepqueue_head;

	while (sleeping != NULL) {
		if (sleeping->wakeup_time > os_tick_count) {
			return;
		}

		sleepqueue_head = sleeping->next_task;
		sleeping->next_task = NULL;

		sleeping->state = RUNNABLE;
		sched_add_to_runqueue_head(sleeping);

		sleeping = sleepqueue_head;
	}
}


void sched_init(void)
{
	for (uint8_t i = 0; i < NUM_PRIO_LEVELS; i++) {
		task_prio_groups[i].first = NULL;
		task_prio_groups[i].last = NULL;
	}
}

void sched_start_tasks(void)
{
	current_task = take_highest_prio_task();

	int task_struct = current_task->stack[8];
	int task_runner = current_task->stack[14];
	int psr_setting = current_task->stack[15];

	current_task->stack += 16;

	// Set EPSR value
	// Set PSP value
	// Switch to using PSP (in CONTROL)
	// Set argument for task_runner
	// Instruction barrier
	// Branch to task runner
	asm volatile (
	    "msr psr_nzcvq, %[psr_setting]\n\t"
	    "msr psp, %[stack]\n\t"
	    "mov r1, #2\n\t"
	    "msr control, r1\n\t"
	    "mov r0, %[task]\n\t"
	    "isb\n\t"
	    "bx %[runner]"
	    :: [runner] "r" (task_runner),
	       [task] "r" (task_struct),
	       [stack] "r" (current_task->stack),
	       [psr_setting] "r" (psr_setting)
	    : "r0", "r1", "cc", "memory");
}


void sched_add_to_runqueue_head(struct tcb *task)
{
	uint8_t prio = task->priority;

	if (prio < highest_prio_level) {
		highest_prio_level = prio;
	}


	if (task_prio_groups[prio].first == NULL) {
		task_prio_groups[prio].first = task;
		task_prio_groups[prio].last = task;

	} else {
		task->next_task = task_prio_groups[prio].first;
		task_prio_groups[prio].first = task;
	}
}

void sched_add_to_runqueue_tail(struct tcb *task)
{
	uint8_t prio = task->priority;

	if (prio < highest_prio_level) {
		highest_prio_level = prio;
	}


	if (task_prio_groups[prio].first == NULL) {
		task_prio_groups[prio].first = task;
		task_prio_groups[prio].last = task;

	} else {
		task_prio_groups[prio].last->next_task = task;
		task_prio_groups[prio].last = task;
	}

	task->next_task = NULL;
}

void sched_add_to_sleepqueue(struct tcb *task)
{
	struct tcb *sleeping = sleepqueue_head;

	if (sleeping == NULL) {
		sleepqueue_head = task;
		task->next_task = NULL;

	} else if (sleeping->wakeup_time > task->wakeup_time) {
		sleepqueue_head = task;
		task->next_task = sleeping;

	} else {
		while (sleeping != NULL) {
			struct tcb *next_sleeping = sleeping->next_task;

			if (next_sleeping == NULL ||
			    next_sleeping->wakeup_time > task->wakeup_time) {
				sleeping->next_task = task;
				task->next_task = next_sleeping;

				break;
			}

			sleeping = next_sleeping;
		}
	}
}



__attribute__((naked, used))
void pend_sv_handler(void)
{
	SCHED_PUSH_STACK();

	SCB_ICSR |= SCB_ICSR_PENDSVCLR;

	if (current_task->state == RUNNING) {
		current_task->state = RUNNABLE;
		sched_add_to_runqueue_tail(current_task);
	}

	current_task = take_highest_prio_task();
	_impure_ptr = &current_task->reent;

	current_task->state = RUNNING;

	SCHED_POP_STACK_AND_BRANCH();
}

__attribute__((naked, used))
void sys_tick_handler(void)
{
	SCHED_PUSH_STACK();

	os_tick_count++;

	wakeup_tasks();

	sched_add_to_runqueue_tail(current_task);

	current_task->state = RUNNABLE;

	current_task = take_highest_prio_task();
	_impure_ptr = &current_task->reent;

	current_task->state = RUNNING;

	SCHED_POP_STACK_AND_BRANCH();
}

