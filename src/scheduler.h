/**
 * @file
 *
 * This file contains declarations for the MourOS scheduler.
 *
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#include <mouros/tasks.h>

/**
 * The total number of priority levels implemented by MourOS. Tasks with higher
 * priority have a lower priority level number.
 */
#define NUM_PRIO_LEVELS 16

/**
 * Pointer to the struct representing the task currently being executed.
 */
extern struct tcb* current_task;

/**
 * The total number of system ticks since os_tasks_start() was run. I.e. since
 * the system was started.
 */
extern uint64_t os_tick_count;

/**
 * Function initializing the scheduler. This function must be run before any
 * other sched_* function.
 */
void sched_init(void);

/**
 * This function will start the scheduler.
 *
 * @note This function does not return.
 */
void sched_start_tasks(void);

/**
 * Adds task to the head of the runnable queue with the priority of task.
 *
 * @param task The task to be added.
 */
void sched_add_to_runqueue_head(struct tcb *task);

/**
 * Adds task to the tail of the runnable queue with the priority of task.
 *
 * @param task The task to be added.
 */
void sched_add_to_runqueue_tail(struct tcb *task);

/**
 * Adds task to the sleepqueue. Tasks in the sleepqueue will get woken up after
 * their wakeup_time has been exceeded by os_tick_count.
 * @param task
 */
void sched_add_to_sleepqueue(struct tcb *task);

#endif /* SCHEDULER_H_ */
