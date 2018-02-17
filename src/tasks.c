/**
 * @file
 *
 * This file contains the implementation of the main MourOS task manipulating
 * functions.
 *
 */

#include <stddef.h>  // For NULL
#include <string.h>  // For memset (used internally in _REENT_INIT_PTR())
#include <stdbool.h> // For true, false

#include <libopencm3/cm3/cortex.h>  // CM3_ATOMIC_* macros
#include <libopencm3/cm3/assert.h>  // assert macros
#include <libopencm3/cm3/systick.h> // systick_* functions
#include <libopencm3/cm3/nvic.h>    // nvic_* functions
#include <libopencm3/cm3/scb.h>     // System control block (SCB_*) defines
#include <libopencm3/stm32/rcc.h>   // rcc_ahb_frequency value

#include <mouros/tasks.h>
#include "scheduler.h"

#include "diag/diag.h"


/**
 * The default exception return vector to start new tasks with. Used when
 * returning from context switching interrupts.
 *
 * @details 0xfffffffd means, that the task should return to privileged thread
 *          mode, use the program stack pointer, and do not use the FPU. If the
 *          processor has an FPU and the task starts using it, the task's
 *          exc_ret changes to 0xffffffed, which means return to privileged
 *          thread mode, use the program stack pointer, and use the FPU.
 */
#define DEFAULT_EXC_RET 0xfffffffd


// __ARM_FP is defined by the compiler. According to http://infocenter.arm.com/
// help/topic/com.arm.doc.ihi0053b/IHI0053B_arm_c_language_extensions_2013.pdf
#if defined(__ARM_FP)
// The stack needs to fit the core MCU state (16 registers * 4 bytes), the
// FPU state (33 * 4 bytes), and possibly some alignment bytes.
#define IDLE_TASK_STACK_SIZE 256
#else
// When the FPU isn't used, only the core MCU state needs to be saved.
#define IDLE_TASK_STACK_SIZE 128
#endif

static uint32_t us_per_tick = 0;
static uint32_t systicks_per_us = 0;

bool os_is_initialized = false;

/**
 * Struct holding pointers to the first and last tasks in the linked list of all
 * tasks.
 */
static struct task_group all_tasks = {
	.first = NULL,
	.last = NULL
};


/**
 * Function implementing the idle (do-nothing) task.
 *
 * @param params Parameters of the idle task. Not used.
 */
static void __idle_task(void *params)
{
	(void) params;

	while(true) {
	}
}

/**
 * Function used to start task execution and to remove the task from the
 * all-tasks linked list.
 *
 * @param task Pointer to the task to be run.
 */
static void __task_runner(struct tcb *task)
{
	task->task_func(task->task_params);

	CM_ATOMIC_BLOCK() {
		struct tcb *prev = task->tasklist_prev;
		struct tcb *next = task->tasklist_next;

		if (next == NULL) {
			all_tasks.last = prev;
		}

		if (prev == NULL) {
			all_tasks.first = next;
		}

		if (next != NULL) {
			next->tasklist_prev = prev;
		}

		if (prev != NULL) {
			prev->tasklist_next = next;
		}

		task->tasklist_next = NULL;
		task->tasklist_prev = NULL;

		task->state = TASK_STOPPED;

		os_task_yield();
	}
}


void os_init(void)
{
	cm3_assert(!os_is_initialized);

	sched_init();

	os_is_initialized = true;

	static struct tcb idle_task;
	static uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE];

	os_task_init(&idle_task, "idle", idle_task_stack, IDLE_TASK_STACK_SIZE,
			NUM_PRIO_LEVELS - 1, __idle_task, NULL);


	os_task_add(&idle_task);

}


bool os_task_init(task_t *task,
                  const char *name,
                  uint8_t *stack_base,
                  uint32_t stack_size,
                  uint8_t priority,
                  void (*task_func)(void *),
                  void *task_params)
{
	if (priority > (NUM_PRIO_LEVELS - 1) || stack_size < 64) {
		return false;
	}

	static uint8_t task_id = 0;

	CM_ATOMIC_BLOCK() {
		task->id = task_id++;
	}

	task->tasklist_next = NULL;
	task->tasklist_prev = NULL;

	task->next_task = NULL;

	task->priority = priority;
	task->task_func = task_func;
	task->task_params = task_params;
	task->state = TASK_STOPPED;

	task->name = name;

	// Make sure the stack is 8 byte aligned, even if it means not using all
	// the provided stack memory.
	uint32_t aligned_stack_top = ((uint32_t) stack_base + stack_size) & (uint32_t) ~0b111;

	task->stack_base = (int *) stack_base;
	task->stack_size = aligned_stack_top - (uint32_t) stack_base;

#ifdef ENABLE_STACK_PAINTING
	for (uint32_t i = 0; i < task_stack_size; i++) {
		stack_base[i] = 0xa5;
	}
#endif

	// The top of the empty stack will contain the frame that gets popped
	// once the task is first scheduled.
	task->stack = (int *) (aligned_stack_top - 64);

	for (uint8_t i = 0; i < 16; i++) {
		task->stack[i] = 0;
	}
	task->stack[8] = (int) task;
	task->stack[14] = (int) __task_runner;
	task->stack[15] = 1 << 24; // write to PSR (EPSR), set the Thumb bit

	task->exc_ret = DEFAULT_EXC_RET;

	_REENT_INIT_PTR(&task->reent);

	return true;
}


bool os_task_add(task_t *new_task)
{
	cm3_assert(os_is_initialized);

	CM_ATOMIC_CONTEXT();


	if (new_task->state != TASK_STOPPED) {
		return false;
	}

	new_task->state = TASK_RUNNABLE;


	if (all_tasks.first == NULL) {
		all_tasks.first = new_task;
		all_tasks.last = new_task;

		new_task->tasklist_next = NULL;
		new_task->tasklist_prev = NULL;
	} else {
		all_tasks.last->tasklist_next = new_task;

		new_task->tasklist_prev = all_tasks.last;
		new_task->tasklist_next = NULL;

		all_tasks.last = new_task;
	}


	sched_add_to_runqueue_tail(new_task);

	return true;
}

void os_tasks_start(uint32_t tick_freq)
{
	cm3_assert(os_is_initialized);

	systick_set_frequency(tick_freq, rcc_ahb_frequency);

	us_per_tick = 1000000 / tick_freq;
	systicks_per_us = (systick_get_reload() + 1) / us_per_tick;

// Silence warning because of a hack libopencm3 did. (NVIC_SYSTICK_IRQ &
// NVIC_PENDSV_IRQ are negative, and nvic_set_priority() expects an unsigned)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
	nvic_set_priority(NVIC_SYSTICK_IRQ, 0xff);
	nvic_set_priority(NVIC_PENDSV_IRQ, 0xff);
#pragma GCC diagnostic pop

	systick_clear();
	systick_interrupt_enable();
	systick_counter_enable();

	sched_start_tasks();
}

void os_task_yield(void)
{
	SCB_ICSR |= SCB_ICSR_PENDSVSET;
}


void os_task_suspend_self(void)
{
	current_task->state = TASK_SUSPENDED;

	os_task_yield();
}

bool os_task_unsuspend(task_t *task)
{
	CM_ATOMIC_CONTEXT();

	if (task->state != TASK_SUSPENDED) {
		return false;
	}

	task->state = TASK_RUNNABLE;

	sched_add_to_runqueue_tail(task);

	if (current_task->priority > task->priority) {
		os_task_yield();
	}

	return true;
}

void os_task_sleep(uint32_t num_ticks)
{
	CM_ATOMIC_CONTEXT();

	current_task->wakeup_time = os_tick_count + num_ticks;
	current_task->state = TASK_SLEEPING;

	sched_add_to_sleepqueue(current_task);

	os_task_yield();
}

void os_task_wait_us(uint64_t wait_time_us)
{
	uint64_t whole_os_ticks = wait_time_us / us_per_tick;
	uint32_t remainder_systicks = (uint32_t) (wait_time_us % us_per_tick) * systicks_per_us;

	uint64_t wait_until_os_ticks = 0;
	uint32_t wait_until_systicks = 0;

	CM_ATOMIC_BLOCK() {
		wait_until_systicks = systick_get_value() - remainder_systicks;
		wait_until_os_ticks = os_tick_count + whole_os_ticks;
	}

	uint32_t reload_val = systick_get_reload();
	if (wait_until_systicks > reload_val) {
		wait_until_os_ticks += 1;
		wait_until_systicks += reload_val;
	}

	while ((os_tick_count < wait_until_os_ticks) ||
	       ((os_tick_count == wait_until_os_ticks) &&
	        (systick_get_value() > wait_until_systicks)));
}

uint32_t os_get_stack_max_size(task_t *task)
{
	return task->stack_size;
}

uint32_t os_get_stack_curr_size(task_t *task)
{
	return (uint32_t) (task->stack - task->stack_base);
}

uint32_t os_get_stack_max_usage(task_t *task)
{
#ifdef ENABLE_STACK_PAINTING
	uint8_t *stack_ptr = NULL;

	for (stack_ptr = ((uint8_t *) task->stack_base) + task->stack_size;
	     stack_ptr >= (uint8_t *) task->stack_base;
	     stack_ptr--) {

		if (*stack_ptr != 0xa5) {
			break;
		}
	}

	return (uint32_t) ((((uint8_t *) task->stack_base) + task->stack_size) - stack_ptr);
#else
	(void) sizeof(task);
	return 0;
#endif
}

void os_set_diagnostics(uint8_t (*diag_send_func)(uint8_t *msg_buf,
                                                  uint8_t msg_buf_len),
                        void (*diag_error_func)(void))
{
	diag_init(diag_send_func, diag_error_func);
}

uint64_t os_get_tick_count(void)
{
	CM_ATOMIC_CONTEXT();

	return os_tick_count;
}


