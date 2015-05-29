/**
 * @file
 *
 * Main MourOS header. Contains declarations of functions and structs
 * implementing tasks.
 *
 */

#ifndef MOUROS_TASKS_H_
#define MOUROS_TASKS_H_

#include <stdint.h>  // For uint32_t, etc.
#include <stdbool.h> // For bool.
#include <reent.h>   // For the reent struct.


/** @cond */
#define ___os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params, stack_num) \
	uint8_t stack_##stack_num[stack_size]; \
	os_task_init(task, name, stack_##stack_num, stack_size, priority, task_func, task_params)

#define __os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params, stack_num) \
	___os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params, stack_num)

/** @endcond */
/**
 * Helper macro used for initializing a task structure and declaring an uint8_t
 * array to be used as the task stack.
 */
#define os_task_init_with_stack(task, name, stack_size, priority, task_func, task_params) \
	__os_task_init_with_stack((task), (name), (stack_size), (priority), (task_func), (task_params), __COUNTER__)


/**
 * The Task Control Block. The main structure holding information about tasks.
 */
typedef struct tcb {
	/** Pointer to the previous task in the linked list of all tasks. */
	struct tcb *tasklist_prev;
	/** Pointer to the next task in the linked list of all tasks. */
	struct tcb *tasklist_next;

	/**
	 * Pointer to the next task in a singly linked list. Used in the
	 * sleepqueue (SLEEPING state) and in the waitqueue
	 * (WAITING_FOR_RESOURCE).
	 */
	struct tcb *next_task;

	/** The ID number of the task. */
	uint8_t id;
	/** Pointer to a string holding the name of the task. */
	const char *name;

	/**
	 * Pointer to the task function.
	 * @param params Pointer to parameters passed to task_func.
	 */
	void (*task_func)(void *params);
	/** Pointer to the parameters passed to task_func. */
	void *task_params;

	/** The task priority. */
	uint8_t priority;

	/** The task state. */
	enum {
		/**
		 * The task is ready to be run.
		 */
		RUNNABLE = 0,
		/**
		 * The task is waiting for a resource to become available.
		 */
		WAITING_FOR_RESOURCE,
		/**
		 * The task is sleeping and will again be scheduled once the
		 * set sleep duration has elapsed.
		 */
		SLEEPING,
		/**
		 * The task is suspended and won't be scheduled.
		 */
		SUSPENDED,
		/**
		 *  The task is currently being executed.
		 */
		RUNNING,
		/**
		 * The task has returned and is done executing.
		 */
		STOPPED
	} state;

	/**
	 * The value of the tick timer when the task should be woken up from the
	 * SLEEPING state.
	 */
	uint64_t wakeup_time;

	/** Pointer pointer to the stack allocated to the task. */
	int *stack_base;
	/** Pointer to the current top of the stack. */
	int *stack;
	/** The total size of the stack allocated to the task. */
	uint32_t stack_size;

	/** Struct used for newlib task reentrancy. */
	struct _reent reent;
} task_t;

/**
 * Struct used to remember the first and last member in a task linked list.
 */
struct task_group {
	/** The first task in the group. */
	struct tcb *first;
	/** The last task in the group. */
	struct tcb *last;
};


/**
 * Variable stating that basic MourOS initialization has been carried out. (By
 * calling os_init())
 */
extern bool os_is_initialized;

/**
 * Initializes the MourOS operating system.
 *
 * @note This function must be run before any other MourOS function.
 */
void os_init(void);

/**
 * Initializes the supplied task_t struct. Must be run before the task is
 * "added" by os_task_add().
 *
 * @param task        Pointer to the task_t struct to be initialized.
 * @param name        The name of the initialized task.
 * @param stack_base  Pointer to the beginning of the stack used for this task.
 * @param stack_size  Size of the task stack.
 * @param priority    The task priority. Must be between 0 and MAX_PRIORITY. A
 *                    lower number means higher priority.
 * @param task_func   The function to execute in the task.
 * @param task_params Pointer to parameters passed to task_func.
 * @return True on successful struct initialization, false otherwise.
 */
bool os_task_init(task_t *task,
                  const char *name,
                  uint8_t *stack_base,
                  uint32_t stack_size,
                  uint8_t priority,
                  void (*task_func)(void *),
                  void *task_params);

/**
 * Adds a newly initialized task to the global task list and to the RUNNABLE
 * queue.
 *
 * @param task The task to be added.
 * @return True on success, false otherwise.
 */
bool os_task_add(task_t *task);

/**
 * Starts executing all added tasks.
 *
 * @note This function does not return.
 */
void os_tasks_start(void);

/**
 * This function will force a call to the scheduler. Normal scheduling rules
 * apply.
 *
 * TODO Add more detail about scheduling rules.
 */
void os_task_yield(void);

/**
 * This function will suspend the current task and force a call to the
 * scheduler. A suspended task will not be scheduled until it is unsuspended
 * again.
 */
void os_task_suspend_self(void);

/**
 * This function will unsuspend the task pointed to by task.
 *
 * @param task Pointer to the task to be unsuspended.
 * @return True on success, false otherwise.
 */
bool os_task_unsuspend(task_t *task);

/**
 * This function will set the current task state to SLEEPING and force a call to
 * the scheduler. A SLEEPING task will not be scheduled by the scheduler. The
 * task state will automatically be set to RUNNABLE and the task will again
 * start to be scheduled after num_ticks system ticks have elapsed.
 *
 * @param num_ticks The number of system ticks the current task should sleep
 *                  for.
 */
void os_task_sleep(uint32_t num_ticks);

#endif /* MOUROS_TASKS_H_ */
