/**
 * @file
 *
 * This file contains the MourOS implementation of a pool allocator.
 */

#include <stddef.h> // For NULL

#include <libopencm3/cm3/cortex.h> // For CM_ATOMIC_CONTEXT().
#include <libopencm3/cm3/assert.h> // For assert().

#include <mouros/pool_alloc.h> // Pool alloc function definitions.



void os_pool_alloc_init(pool_alloc_t *alloc,
                        void *backing_mem,
                        uint32_t block_size,
                        uint32_t num_blocks) {

	cm3_assert(block_size >= sizeof(uintptr_t));

	CM_ATOMIC_CONTEXT();

	alloc->block_size = block_size;
	alloc->first_block = backing_mem;

	uintptr_t *curr_block = backing_mem;

	for (uint32_t i = 0; i < num_blocks - 1; i++) {

		uintptr_t next = (uintptr_t) (((uint8_t *) curr_block) + block_size);

		*curr_block = next;
		curr_block = (uintptr_t *) next;
	}

	*curr_block = (uintptr_t) NULL;
}

void *os_pool_alloc_take(pool_alloc_t *alloc) {
	CM_ATOMIC_CONTEXT();

	uintptr_t *ret = alloc->first_block;

	if (ret != NULL) {
		alloc->first_block = (uintptr_t *) *ret;
	}

	return ret;
}

void os_pool_alloc_give(pool_alloc_t *alloc, void *block) {
	CM_ATOMIC_CONTEXT();

	*((uintptr_t *) block) = (uintptr_t) alloc->first_block;
	alloc->first_block = block;
}
