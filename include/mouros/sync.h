/*
 * sync.h
 *
 *  Created on: 28. 5. 2015
 *      Author: ondra
 */

#ifndef MOUROS_SYNC_H_
#define MOUROS_SYNC_H_

#include <mouros/tasks.h>

typedef struct resource {
	struct tcb *first_waiting;
	struct tcb *acquired_by;
} resource_t;


void os_resource_acquire(resource_t *res);
void os_resource_release(resource_t *res);

#endif /* MOUROS_SYNC_H_ */
