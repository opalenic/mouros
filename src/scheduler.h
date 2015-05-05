/*
 * scheduler.h
 *
 *  Created on: 28. 4. 2015
 *      Author: ondra
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#include <mouros/tasks.h>

#define NUM_PRIO_LEVELS 16


extern struct tcb* current_task;

extern uint64_t os_tick_count;

void sched_init(void);

void sched_start_tasks(void);

void sched_add_to_runqueue_head(struct tcb *task);
void sched_add_to_runqueue_tail(struct tcb *task);

void sched_add_to_sleepqueue(struct tcb *task);

#endif /* SCHEDULER_H_ */
