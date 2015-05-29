/**
 * @file
 *
 * Definitions of functions and structures implementing resources.
 *
 */

#ifndef MOUROS_SYNC_H_
#define MOUROS_SYNC_H_

#include <mouros/tasks.h>

/**
 * Struct holding information about a resource.
 */
typedef struct resource {
	/**
	 * Pointer to the first task waiting for the resource. This is the first
	 * task in a linked list.
	 */
	struct tcb *first_waiting;
	/**
	 * Pointer to the task currently owning the resource.
	 */
	struct tcb *acquired_by;
} resource_t;

/**
 * Acquires ownership of the resource pointed to by res.
 *
 * @note The call will block until the resource becomes available.
 *
 * @param res The resource to be acquired.
 */
void os_resource_acquire(resource_t *res);

/**
 * Releases the ownership of the resource pointed to by res.
 *
 * @note It is currently possible for a task to release a resource belonging to
 *       another task.
 *
 * @param res The resource to be released.
 */
void os_resource_release(resource_t *res);

#endif /* MOUROS_SYNC_H_ */
