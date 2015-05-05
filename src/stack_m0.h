/*
 * stack_m0.h
 *
 *  Created on: 27. 4. 2015
 *      Author: ondra
 */

#ifndef STACK_M0_H_
#define STACK_M0_H_


#define SCHED_PUSH_STACK() \
	register int *psp_after_push asm("r0"); \
	\
	asm ("mrs %[new_psp], psp\n\t" \
	     "sub %[new_psp], #32\n\t" \
	     "mov r1, %[new_psp]\n\t" \
	     "stm r1!, {r4-r7}\n\t" \
	     "mov r4, r8\n\t" \
	     "mov r5, r9\n\t" \
	     "mov r6, r10\n\t" \
	     "mov r7, r11\n\t" \
	     "stm r1!, {r4-r7}" \
	     : [new_psp] "=r" (psp_after_push) \
	     :: "cc", "memory"); \
	\
	current_task->stack = psp_after_push

#define SCHED_POP_STACK_AND_BRANCH() \
	register int *psp_before_pop asm("r0") = current_task->stack; \
	register unsigned int ret_vector asm("r1") = 0xfffffffd; \
	\
	asm ("mov r2, #16\n\t" \
	     "add r3, %[old_psp], r2\n\t" \
	     "ldm r3!, {r4-r7}\n\t" \
	     "mov r8, r4\n\t" \
	     "mov r9, r5\n\t" \
	     "mov r10, r6\n\t" \
	     "mov r11, r7\n\t" \
	     "ldm %[old_psp]!, {r4-r7}\n\t" \
	     "msr psp, r3\n\t" \
	     "bx %[exc_ret]" \
	     :: [old_psp] "r" (psp_before_pop), \
	        [exc_ret] "r" (ret_vector) \
	     : "cc")


#endif /* STACK_M0_H_ */
