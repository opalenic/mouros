/**
 * @file
 *
 * This file contains the definition of the macros used to save and reload a
 * task's context in pend_sv_handler() and sys_tick_handler().
 *
 */

#ifndef STACK_M4F_H_
#define STACK_M4F_H_


/**
 * This macro saves the state of the processor registers onto the current task
 * stack.
 *
 * @details Store the PSP in a register (r0).
 *          Stack R4-R11.
 *          If the FPU was used (indicated by the bit in the link register),
 *          stack the FPU registers. Stacking D8-D15 (S16-S31) triggers the
 *          stacking of D0-D7 (S0-S15).
 */
#define SCHED_PUSH_STACK() \
	register int *psp_after_push asm("r0"); \
	register int exc_ret asm("r1"); \
	\
	asm ("mrs %[new_psp], psp\n\t" \
	     "stmdb %[new_psp]!, {r4-r11}\n\t" \
	     "mov %[exc_ret], lr\n\t" \
	     "tst %[exc_ret], #0x10\n\t" \
	     "it eq\n\t" \
	     "vstmdbeq %[new_psp]!, {d8-d15}" \
	     : [new_psp] "=r" (psp_after_push), \
	       [exc_ret] "=r" (exc_ret) \
	     :: "cc", "memory"); \
	\
	current_task->stack = psp_after_push; \
	current_task->exc_ret = exc_ret

/**
 * This macro restores the state of the processor registers from the current
 * task stack, and returns to current task execution.
 *
 * @details The exc_ret value is restored.
 *          If the FPU was active (indicated in exc_ret), restore the FPU
 *          context. The remaining FPU registers will be automatically restored
 *          on exception exit.
 *          Restore R4-R11.
 *          Restore the PSP.
 *          Branch to the current task.
 */
#define SCHED_POP_STACK_AND_BRANCH() \
	asm ("mov lr, %[exc_ret]" \
	     :: [exc_ret] "r" (current_task->exc_ret)); \
	\
	register int *psp_before_pop asm("r0") = current_task->stack; \
	\
	asm ("tst lr, #0x10\n\t" \
	     "it eq\n\t" \
	     "vldmiaeq %[old_psp]!, {d8-d15}\n\t" \
	     "ldmia %[old_psp]!, {r4-r11}\n\t" \
	     "msr psp, %[old_psp]\n\t" \
	     "bx lr" \
	     :: [old_psp] "r" (psp_before_pop) \
	     : "cc");


#endif /* STACK_M4F_H_ */
