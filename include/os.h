/*
 * os.h
 *
 *  Created on: 19. 12. 2014
 *      Author: ondra
 */

#ifndef OS_H_
#define OS_H_

#include <stdint.h>
#include <sys/queue.h>

#define __os_init_task_with_stack(task, name, stack_size, stack_id, priority, task_func, task_params) \
	uint8_t stack_##stack_id[(stack_size)]; \
	os_init_task((task), (name), stack_##stack_id, (stack_size), (priority), (task_func), (task_params))

#define __os_init_task_with_stack_int(task, name, stack_size, stack_id, priority, task_func, task_params) \
	__os_init_task_with_stack((task), (name), (stack_size), stack_id, (priority), (task_func), (task_params))

#define os_init_task_with_stack(task, name, stack_size, priority, task_func, task_params) \
	__os_init_task_with_stack_int((task), (name), (stack_size), __COUNTER__, (priority), (task_func), (task_params))


typedef struct tcb {
	struct tcb *next_task;
	struct tcb *next_delayed_task;

	uint8_t id;
	const uint8_t *name;

	void (*task_func)(void *);
	void *task_params;

	uint8_t priority;

	enum {
		RUNNABLE = 0,
		DELAYED,
		RUNNING,
		STOPPED
	} state;

	uint64_t wakeup_time;

	int *stack_base;
	int *stack;
	uint32_t stack_size;
} task_t;


void os_init_task(task_t *task, const uint8_t *name, uint8_t *stack_base,
		uint32_t stack_size, uint8_t priority,
		void (*task_func)(void *), void *task_params);

void os_task_delay(uint32_t ticks);

void os_add_task(task_t *task);
void os_yield(void);
void os_init(void);
void os_start_tasks(void);


#endif /* OS_H_ */
