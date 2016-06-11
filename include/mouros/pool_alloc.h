/**
 * @file
 *
 * Header file for the MourOS pool allocator.
 *
 * This is an allocator which returns blocks of a predefined size.
 *
 * Internally, the blocks are organized into a singly linked list of free
 * blocks. Every time an "allocation" is made (via os_pool_alloc_take()), the
 * first block in the list is returned, and the second block becomes the new
 * first block. Every time a block is freed (via os_pool_alloc_give()), the
 * newly freed block is prepended to the list of free blocks.
 */

#ifndef MOUROS_POOL_ALLOC_H_
#define MOUROS_POOL_ALLOC_H_

#include <stdint.h> // For uint32_t, ...

/**
 * Struct holding information about the pool of allocatable blocks.
 */
typedef struct pool_alloc {
	/** Pointer to the first block in the list of free blocks. */
	void *first_block;
	/** The size (in bytes) of blocks in this pool. */
	uint32_t block_size;
} pool_alloc_t;


/**
 * Initialization function for pool_alloc_t. This must be called before a pool
 * is used.
 *
 * @note The size of the individual blocks must be at least the size of a
 *       pointer type on the architecture used. E.g. at least four bytes on a
 *       32-bit ARM.
 *
 * @param alloc       Pointer to the pool struct to be initialized.
 * @param backing_mem Pointer to the beginning of a memory area that should hold
 *                    the individual blocks. Obviously this needs to be at least
 *                    block_size * num_blocks bytes large.
 * @param block_size  The size (in bytes) of an individual block.
 * @param num_blocks  The number of blocks to initialize.
 */
void os_pool_alloc_init(pool_alloc_t *alloc,
                        void *backing_mem,
                        uint32_t block_size,
                        uint32_t num_blocks);

/**
 * Returns a pointer to an area of memory of block_size (see
 * os_pool_alloc_init()).
 *
 * @param alloc Pointer to the pool struct holding information about the block
 *              pool.
 *
 * @return Pointer to the newly allocated block. Returns NULL if no free blocks
 *         are available.
 */
void *os_pool_alloc_take(pool_alloc_t *alloc);

/**
 * Returns a memory block of block_size (see os_pool_alloc_init()) to the block
 * pool.
 *
 * @param alloc Pointer to the pool struct holding information about the block
 *              pool.

 * @param block Pointer to the block that should be returned.
 */
void os_pool_alloc_give(pool_alloc_t *alloc, void *block);


#endif /* MOUROS_POOL_ALLOC_H_ */


