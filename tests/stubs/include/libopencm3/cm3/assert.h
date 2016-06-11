/**
 * @file
 *
 * Override header for libopencm3 asserts. To be used with cmocka unit tests.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#ifndef ASSERT_H_
#define ASSERT_H_

#define cm3_assert(expr) mock_assert((int) (expr), #expr, __FILE__, __LINE__)

#define cm3_assert_not_reached() fail()

#endif /* ASSERT_H_ */


