/**
 * @file
 *
 * This file contains tests for the MourOS pool allocator.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdint.h>
#include <stddef.h>

#include <mouros/pool_alloc.h>

struct test_struct {
	uint64_t member1;
	uint32_t member2;
	uint8_t member3;
};

static void alloc_dealloc_test(void **state) {

	(void) state;

	pool_alloc_t pool;

	uint8_t num_elements = 120;
	struct test_struct backing_memory[num_elements];

	os_pool_alloc_init(&pool,
	                   backing_memory,
	                   sizeof(struct test_struct),
	                   num_elements);


	struct test_struct* block_ptrs[num_elements];

	// Empty the pool and see the blocks are returned in the correct order.
	for (uint8_t i = 0; i < num_elements; i++) {
		block_ptrs[i] = os_pool_alloc_take(&pool);
		assert_ptr_equal(block_ptrs[i], &backing_memory[i]);
	}

	// No blocks should be left in the pool now.
	assert_ptr_equal(os_pool_alloc_take(&pool), NULL);

	// Return the blocks in reverse order.
	for (uint8_t i = 0; i < num_elements; i++) {
		os_pool_alloc_give(&pool, block_ptrs[i]);
	}

	// Empty the pool again and make sure it behaves like a LIFO.
	for (uint8_t i = 0; i < num_elements; i++) {
		block_ptrs[i] = os_pool_alloc_take(&pool);

		assert_ptr_equal(block_ptrs[i],
		                 &backing_memory[num_elements - 1 - i]);
	}

	// No blocks should be left in the pool now.
	assert_ptr_equal(os_pool_alloc_take(&pool), NULL);
}

static void small_block_test(void **state) {

	(void) state;

	uint16_t backing_memory[10];

	pool_alloc_t pool;
	expect_assert_failure(os_pool_alloc_init(&pool, backing_memory, 2, 10));
}

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(alloc_dealloc_test),
		cmocka_unit_test(small_block_test)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
