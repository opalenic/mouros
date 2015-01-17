/*
 * os.c
 *
 *  Created on: 19. 12. 2014
 *      Author: ondra
 */

#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

#include <bsp.h>

#include <os.h>

#define NUM_PRIO_GROUPS 16
#define IDLE_TASK_PRIO 0xF


STAILQ_HEAD(tcb_queue, tcb);
static struct tcb_queue task_groups[NUM_PRIO_GROUPS];
static uint8_t highest_prio_group;

SLIST_HEAD(tcb_list, tcb);
static struct tcb_list delayed_tasks = SLIST_HEAD_INITIALIZER(delayed_tasks);

static struct tcb *current_task = NULL;

static uint64_t os_tick_count = 0;


static uint8_t idle_task_stack[128];
static task_t idle_task;



// should never run delay in the idle task!
static void __os_idle_task(void *params)
{
	while(true) {
		for (uint32_t i = 0; i < 10000; i++) {
			asm("nop");
		}
		bsp_led_toggle(LEDO);
	}
}


static void __os_blocking_handler(void)
{
	while(true);
}


static void __os_task_runner(struct tcb *task)
{
	task->task_func(task->task_params);
	task->state = STOPPED;
	os_yield();
}


__attribute__((always_inline))
static inline void push_prg_stack(void)
{
	asm ("mrs %[new_sp], psp\n\t"
	     "sub %[new_sp], #32\n\t"
	     "mov r1, %[new_sp]\n\t"
	     "stm r1!, {r4-r7}\n\t"
	     "mov r4, r8\n\t"
	     "mov r5, r9\n\t"
	     "mov r6, r10\n\t"
	     "mov r7, r11\n\t"
	     "stm r1!, {r4-r7}"
	     : [new_sp] "=r" (current_task->stack)
	     :: "cc", "memory");

}

__attribute__((always_inline))
static inline void pop_prg_stack(void)
{
	asm ("mov r2, #16\n\t"
	     "add r1, %[old_sp], r2\n\t"
	     "ldm r1!, {r4-r7}\n\t"
	     "mov r8, r4\n\t"
	     "mov r9, r5\n\t"
	     "mov r10, r6\n\t"
	     "mov r11, r7\n\t"
	     "ldm %[old_sp]!, {r4-r7}\n\t"
	     "msr psp, r1"
	     :: [old_sp] "r" (current_task->stack)
	     : "cc");
}

__attribute__((always_inline))
static inline void wakeup_tasks(void)
{
	struct tcb *task = SLIST_FIRST(&delayed_tasks);

	while (task != NULL) {
		if (task->wakeup_time > os_tick_count) {
			return;
		}

		task->state = RUNNABLE;
		if (task->priority < highest_prio_group) {
			highest_prio_group = task->priority;
		}

		SLIST_REMOVE_HEAD(&delayed_tasks, dtl_el);
		task = SLIST_FIRST(&delayed_tasks);
	}
}

__attribute__((always_inline))
static inline struct tcb *get_highest_prio_task(void)
{

	STAILQ_REMOVE_HEAD(&task_groups[current_task->priority], tq_el);

	STAILQ_INSERT_TAIL(&task_groups[current_task->priority], current_task, tq_el);


	for (uint8_t i = highest_prio_group; i < NUM_PRIO_GROUPS; i++) {
		highest_prio_group = i;
		struct tcb *task = STAILQ_FIRST(&task_groups[i]);

		while (task != NULL) {
			if (task->state == RUNNABLE) {
				return task;
			}

			task = STAILQ_NEXT(task, tq_el);
		}
	}

	// if we got here, something went very wrong
	// force an NMI (blocking handler);
	SCB_ICSR |= (uint32_t) SCB_ICSR_NMIPENDSET;
	return NULL;
}


void pend_sv_handler(void)
{
	push_prg_stack();

	SCB_ICSR |= SCB_ICSR_PENDSVCLR;

	current_task->state = RUNNABLE;

	current_task = get_highest_prio_task();

	current_task->state = RUNNING;

	pop_prg_stack();

	asm("bx %[exc_ret]"
	    :: [exc_ret] "r" (0xFFFFFFFD));
}

void sys_tick_handler(void)
{
	push_prg_stack();

	os_tick_count++;

	wakeup_tasks();

	current_task->state = RUNNABLE;

	current_task = get_highest_prio_task();

	current_task->state = RUNNING;

	pop_prg_stack();

	asm("bx %[exc_ret]"
	    :: [exc_ret] "r" (0xFFFFFFFD));
}


void os_init_task(task_t *task, const uint8_t *name, uint8_t *stack_base,
		uint32_t stack_size, uint8_t priority,
		void (*task_func)(void *), void *task_params)
{
	if (priority > 0xf ||
	    stack_size < 64) {
		return;
	}

	static uint8_t task_id = 0;
	CM_ATOMIC_BLOCK() {
		task->id = task_id++;
	}

	task->priority = priority;
	task->task_func = task_func;
	task->task_params = task_params;
	task->state = RUNNABLE;

	task->name = name;
	task->stack_base = (int *)stack_base;
	task->stack_size = stack_size;



	task->stack = (int *) (stack_base + stack_size - 64);

	for (uint8_t i = 0; i < 16; i++) {
		task->stack[i] = 0;
	}
	task->stack[8] = (int) task;
	task->stack[13] = (int) __os_blocking_handler;
	task->stack[14] = (int) __os_task_runner;
	task->stack[15] = 1 << 24; // write to PSR (EPSR), set the Thumb bit
}


void os_add_task(task_t *new_task)
{
	CM_ATOMIC_CONTEXT();

	STAILQ_INSERT_TAIL(&task_groups[new_task->priority], new_task, tq_el);
}

void os_yield(void)
{
	SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void os_task_delay(uint32_t ticks)
{
	CM_ATOMIC_CONTEXT();

	current_task->wakeup_time = os_tick_count + ticks;

	struct tcb *delayed_task = SLIST_FIRST(&delayed_tasks);

	if ((delayed_task == NULL) ||
	    (delayed_task->wakeup_time > current_task->wakeup_time) ||
	    (delayed_task->wakeup_time == current_task->wakeup_time &&
	     delayed_task->priority > current_task->priority)) {

		SLIST_INSERT_HEAD(&delayed_tasks, current_task, dtl_el);
	} else {

		while (delayed_task != NULL) {
			struct tcb *next = SLIST_NEXT(delayed_task, dtl_el);

			if ((next == NULL) ||
			    (next->wakeup_time > current_task->wakeup_time) ||
			    (next->wakeup_time == current_task->wakeup_time &&
			     next->priority > current_task->priority)) {

				SLIST_INSERT_AFTER(delayed_task, current_task, dtl_el);

				break;
			}

			delayed_task = next;
		}
	}

	current_task->state = DELAYED;

	os_yield();
}

void os_init(void)
{
	CM_ATOMIC_CONTEXT();

	for (uint8_t i = 0; i < NUM_PRIO_GROUPS; i++) {
		task_groups[i] = (struct tcb_queue) STAILQ_HEAD_INITIALIZER(task_groups[i]);
	}

	os_init_task(&idle_task, (uint8_t *)"idle", idle_task_stack, 128, 0xf,
			__os_idle_task, NULL);

	current_task = &idle_task;

	STAILQ_INSERT_HEAD(&task_groups[IDLE_TASK_PRIO], &idle_task, tq_el);
}

void os_start_tasks(void)
{
        systick_set_frequency(1, rcc_ahb_frequency);
        systick_interrupt_enable();
        systick_counter_enable();

        current_task = get_highest_prio_task();

        highest_prio_group = current_task->priority;

	current_task->stack += 16;

	asm volatile ("mov r0, %[task]\n\t"
	    "msr psp, %[stack]\n\t"
	    "mov r1, #2\n\t"
	    "msr control, r1\n\t"
	    "isb\n\t"
	    "bx %[runner]"
	    :: [runner] "r" (__os_task_runner),
	       [task] "r" (current_task),
	       [stack] "r" (current_task->stack)
	    : "r0", "r1", "cc", "memory");
}
