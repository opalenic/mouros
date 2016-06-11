/**
 * @file
 *
 * Override header for libopencm3 interrupt & fault control functions.
 * To be used with cmocka unit tests.
 */

#include <stdbool.h>
#include <stdint.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#ifndef CORTEX_H_
#define CORTEX_H_

static inline void cm_enable_interrupts(void)
{
}

static inline void cm_disable_interrupts(void)
{
}

static inline void cm_enable_faults(void)
{
}

static inline void cm_disable_faults(void)
{
}

static inline bool cm_is_masked_interrupts(void)
{
	return (bool) mock();
}

__attribute__((always_inline))
static inline bool cm_is_masked_faults(void)
{
	return (bool) mock();
}

__attribute__((always_inline))
static inline uint32_t cm_mask_interrupts(uint32_t mask)
{
	check_expected(mask);

	return (uint32_t) mock();
}

__attribute__((always_inline))
static inline uint32_t cm_mask_faults(uint32_t mask)
{
	check_expected(mask);

	return (uint32_t) mock();
}

#define CM_ATOMIC_BLOCK()
#define CM_ATOMIC_CONTEXT()

#endif /* CORTEX_H_ */


