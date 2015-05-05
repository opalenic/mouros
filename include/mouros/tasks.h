/*
 * tasks.h
 *
 *  Created on: 27. 4. 2015
 *      Author: ondra
 */

#ifndef MOUROS_TASKS_H_
#define MOUROS_TASKS_H_


#include <stdint.h>
#include <stdbool.h>
#include <reent.h>


#define ___os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params, stack_num) \
	uint8_t stack_##stack_num[stack_size]; \
	os_task_init(task, name, stack_##stack_num, stack_size, priority, task_func, task_params)

#define __os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params, stack_num) \
	___os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params, stack_num)

#define os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params) \
	__os_task_init_with_stack((task), (name), (stack_size), (priority), (task_func), (task_params), __COUNTER__)


typedef struct tcb {
	struct tcb *tasklist_prev;
	struct tcb *tasklist_next;

	struct tcb *next_task;

	uint8_t id;
	const char *name;

	void (*task_func)(void *);
	void *task_params;

	uint8_t priority;

	enum {
		RUNNABLE = 0,
		WAITING_FOR_RESOURCE,
		SLEEPING,
		SUSPENDED,
		RUNNING,
		STOPPED
	} state;

	uint64_t wakeup_time;

	int *stack_base;
	int *stack;
	uint32_t stack_size;

	struct _reent reent;
} task_t;

struct task_group {
	struct tcb* first;
	struct tcb* last;
};



extern bool os_is_initialized;


void os_init(void);

bool os_task_init(task_t *task,
                  const char *name,
                  uint8_t *stack_base,
                  uint32_t stack_size,
                  uint8_t priority,
                  void (*task_func)(void *),
                  void *task_params);

bool os_task_add(task_t *task);

void os_tasks_start(void);

void os_task_yield(void);


#endif /* MOUROS_TASKS_H_ */
