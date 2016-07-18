/**
 * @file
 *
 * This file contains a stub implementation of the MourOS pool allocator.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <mouros/pool_alloc.h> // Pool alloc function definitions.



void os_pool_alloc_init(pool_alloc_t *alloc,
                        void *backing_mem,
                        uint32_t block_size,
                        uint32_t num_blocks)
{
	check_expected_ptr(alloc);
	check_expected_ptr(backing_mem);
	check_expected(block_size);
	check_expected(num_blocks);
}

void *os_pool_alloc_take(pool_alloc_t *alloc)
{
	check_expected_ptr(alloc);

	return mock_ptr_type(void *);
}

void os_pool_alloc_give(pool_alloc_t *alloc, void *block)
{
	check_expected_ptr(alloc);
	check_expected_ptr(block);
}
