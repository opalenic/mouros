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


struct task_group {
	struct tcb *first;
	struct tcb *last;
};

static struct task_group task_groups[NUM_PRIO_GROUPS];
static uint8_t highest_prio_group = 0;


static struct tcb *delayed_tasks = NULL;

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

	CM_ATOMIC_BLOCK() {
		task->state = STOPPED;

		os_yield();
	}
}


__attribute__((always_inline))
static inline void push_prg_stack(void)
{
	register int *new_psp asm("r0");

	asm ("mrs %[new_sp], psp\n\t"
	     "sub %[new_sp], #32\n\t"
	     "mov r1, %[new_sp]\n\t"
	     "stm r1!, {r4-r7}\n\t"
	     "mov r4, r8\n\t"
	     "mov r5, r9\n\t"
	     "mov r6, r10\n\t"
	     "mov r7, r11\n\t"
	     "stm r1!, {r4-r7}"
	     : [new_sp] "=r" (new_psp)
	     :: "cc", "memory");

	current_task->stack = new_psp;
}

__attribute__((always_inline))
static inline void pop_prg_stack(void)
{
	register int *old_psp asm("r0") = current_task->stack;

	asm ("mov r2, #16\n\t"
	     "add r1, %[old_sp], r2\n\t"
	     "ldm r1!, {r4-r7}\n\t"
	     "mov r8, r4\n\t"
	     "mov r9, r5\n\t"
	     "mov r10, r6\n\t"
	     "mov r11, r7\n\t"
	     "ldm %[old_sp]!, {r4-r7}\n\t"
	     "msr psp, r1"
	     :: [old_sp] "r" (old_psp)
	     : "cc");
}

//__attribute__((always_inline))
static void wakeup_tasks(void)
{
	struct tcb *task = delayed_tasks;

	while (task != NULL) {
		if (task->wakeup_time > os_tick_count) {
			return;
		}

		task->state = RUNNABLE;
		if (task->priority < highest_prio_group) {
			highest_prio_group = task->priority;
		}

		delayed_tasks = task->next_wd_task;
		task->next_wd_task = NULL;

		task->next_task = task_groups[task->priority].first;
		task_groups[task->priority].first = task;

		task = delayed_tasks;
	}
}

//__attribute__((always_inline))
static inline void return_task_to_runqueue(struct tcb *task)
{
	uint8_t prio = task->priority;
	// don't do anything if the queue only has one member
	if (task_groups[prio].first == NULL) {
		task_groups[prio].first = task;
		task_groups[prio].last = task;

	} else {
		task_groups[prio].last->next_task = task;
		task_groups[prio].last = task;
	}

	task->next_task = NULL;
}

//__attribute__((always_inline))
static struct tcb *get_task_from_runqueue(void)
{
	for (uint8_t i = highest_prio_group; i < NUM_PRIO_GROUPS; i++) {
		highest_prio_group = i;
		struct tcb *task = task_groups[i].first;

		if (task != NULL) {
			task_groups[i].first = task->next_task;
			if (task->next_task == NULL) {
				task_groups[i].last = NULL;
			}

			return task;
		}
	}

	// if we got here, something went very wrong
	while (true);
	return NULL;
}

__attribute__((naked))
void pend_sv_handler(void)
{
	push_prg_stack();

	SCB_ICSR |= SCB_ICSR_PENDSVCLR;

	if (current_task->state == RUNNING) {
		current_task->state = RUNNABLE;
		return_task_to_runqueue(current_task);
	}

	current_task = get_task_from_runqueue();

	current_task->state = RUNNING;

	pop_prg_stack();

	asm("bx %[exc_ret]"
	    :: [exc_ret] "r" (0xFFFFFFFD));
}

__attribute__((naked))
void sys_tick_handler(void)
{
	push_prg_stack();

	os_tick_count++;

	return_task_to_runqueue(current_task);

	wakeup_tasks();

	current_task->state = RUNNABLE;

	comm_send_num_u(os_tick_count);
	comm_send_str((uint8_t *)": ");
	comm_send_str(current_task->name);

	current_task = get_task_from_runqueue();

	comm_send_str((uint8_t *)" -> ");
	comm_send_str(current_task->name);


	uint32_t psp, msp;
	asm("mrs %[psp], psp\n\t"
	    "mrs %[msp], msp"
	    :: [psp] "r" (psp), [msp] "r" (msp));

	comm_send_str((uint8_t *)" PSP: ");
	comm_send_num_u(psp);

	comm_send_str((uint8_t *)" MSP: ");
	comm_send_num_u(msp);

	comm_send_str((uint8_t *)"\r\n");

	current_task->state = RUNNING;

	pop_prg_stack();

	asm("bx %[exc_ret]"
	    :: [exc_ret] "r" (0xFFFFFFFD));
}


void os_init_task(task_t *task, const uint8_t *name, uint8_t *stack_base,
		uint32_t stack_size, uint8_t priority,
		void (*task_func)(void *), void *task_params)
{
	if (priority > IDLE_TASK_PRIO ||
	    stack_size < 64) {
		return;
	}

	static uint8_t task_id = 0;
	CM_ATOMIC_BLOCK() {
		task->id = task_id++;
	}

	task->next_task = NULL;
	task->next_wd_task = NULL;

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

	new_task->next_task = NULL;

	if (task_groups[new_task->priority].first == NULL) {
		task_groups[new_task->priority].first = new_task;
	} else {
		task_groups[new_task->priority].last->next_task = new_task;
	}

	task_groups[new_task->priority].last = new_task;

	if (new_task->priority < highest_prio_group) {
		highest_prio_group = new_task->priority;
	}

}

void os_yield(void)
{
	SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void os_task_delay(uint32_t ticks)
{
	CM_ATOMIC_CONTEXT();

	current_task->wakeup_time = os_tick_count + ticks;
	current_task->state = DELAYED;

	struct tcb *delayed_task = delayed_tasks;

	if (delayed_task == NULL) {
		delayed_tasks = current_task;
		current_task->next_wd_task = NULL;

	} else if (delayed_task->wakeup_time >= current_task->wakeup_time) {
		delayed_tasks = current_task;
		current_task->next_wd_task = delayed_task;

	} else {
		while (delayed_task != NULL) {
			struct tcb *next = delayed_task->next_wd_task;

			if (next == NULL ||
			    next->wakeup_time >= current_task->wakeup_time) {
				delayed_task->next_wd_task = current_task;
				current_task->next_wd_task = next;

				break;
			}

			delayed_task = next;
		}
	}

	os_yield();
}


void os_task_sleep(void)
{
	CM_ATOMIC_CONTEXT();

	current_task->state = SLEEPING;

	os_yield();
}


bool os_task_wakeup(task_t *task)
{
	CM_ATOMIC_CONTEXT();

	if (task->state != SLEEPING) {
		return false;
	}

	task->state = RUNNABLE;

	return_task_to_runqueue(task);

	if (current_task->priority > task->priority) {
		os_yield();
	}

	return true;
}


static void insert_waiting_task(struct resource *res) {

	struct tcb *task = res->first_waiting;

	if (task == NULL) {
		res->first_waiting = current_task;
		current_task->next_wd_task = NULL;

	} else if (task->priority > current_task->priority) {

		res->first_waiting = current_task;
		current_task->next_wd_task = task;

	} else {
		while (task != NULL) {
			struct tcb *next = task->next_wd_task;

			if (next == NULL ||
					next->priority > current_task->priority) {

				task->next_wd_task = current_task;
				current_task->next_wd_task = next;

				break;
			}

			task = next;
		}
	}
}

void os_acquire_resource(resource_t *res)
{
	while (true) {
		CM_ATOMIC_BLOCK() {
			if (res->is_available) {
				res->is_available = false;
				return;
			} else {
				current_task->state = WAITING_FOR_RESOURCE;

				insert_waiting_task(res);
			}

			os_yield();
		}
	}
}


void os_release_resource(resource_t *res)
{
	CM_ATOMIC_CONTEXT();

	res->is_available = true;

	struct tcb *first = res->first_waiting;

	if (first != NULL) {
		res->first_waiting = first->next_wd_task;

		first->next_wd_task = NULL;
		first->state = RUNNABLE;

		if (first->priority > highest_prio_group) {
			highest_prio_group = first->priority;
		}

		return_task_to_runqueue(first);
	}
}


void os_init(void)
{
	CM_ATOMIC_CONTEXT();

	for (uint8_t i = 0; i < NUM_PRIO_GROUPS; i++) {
		task_groups[i].first = NULL;
		task_groups[i].last = NULL;
	}

	os_init_task(&idle_task, (uint8_t *)"idle", idle_task_stack, 128, IDLE_TASK_PRIO,
			__os_idle_task, NULL);

	current_task = &idle_task;

	os_add_task(&idle_task);
}

void os_start_tasks(void)
{
	systick_set_frequency(10, rcc_ahb_frequency);

	// Systick priority needs to be set to lowest, so that a systick
	// interrupt arriving during the processing of an isr doesn't save
	// the context of the isr instead of the task in which the isr interrupt
	// happened.
	nvic_set_priority(NVIC_SYSTICK_IRQ, 0xff);

	// Same as above. This allows for calling os_yield() in isrs.
	nvic_set_priority(NVIC_PENDSV_IRQ, 0xff);

	systick_interrupt_enable();
	systick_counter_enable();

	current_task = get_task_from_runqueue();

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
