/**
 * @file
 *
 * This file contains the stub implementation of the MourOS task functions.
 */


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <mouros/tasks.h>


void os_init(void)
{
}


bool os_task_init(task_t *task,
                  const char *name,
                  uint8_t *stack_base,
                  uint32_t stack_size,
                  uint8_t priority,
                  void (*task_func)(void *),
                  void *task_params)
{
	check_expected_ptr(task);
	check_expected_ptr(name);
	check_expected_ptr(stack_base);
	check_expected(stack_size);
	check_expected(priority);
	check_expected_ptr(task_func);
	check_expected_ptr(task_params);

	return mock_type(bool);
}


bool os_task_add(task_t *new_task)
{
	check_expected_ptr(new_task);

	return mock_type(bool);
}

void os_tasks_start(uint32_t tick_freq)
{
	check_expected(tick_freq);
}

void os_task_yield(void)
{
}


void os_task_suspend_self(void)
{
}

bool os_task_unsuspend(task_t *task)
{
	check_expected_ptr(task);

	return mock_type(bool);
}

void os_task_sleep(uint32_t num_ticks)
{
	check_expected(num_ticks);
}

void os_set_diagnostics(uint8_t (*diag_send_func)(uint8_t *msg_buf,
                                                  uint8_t msg_buf_len),
                        void (*diag_error_func)(void))
{
	check_expected_ptr(diag_send_func);
	check_expected_ptr(diag_error_func);
}




