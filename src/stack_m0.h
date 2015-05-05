/*
 * stack_m0.h
 *
 *  Created on: 27. 4. 2015
 *      Author: ondra
 */

#ifndef SRC_STACK_M0_H_
#define SRC_STACK_M0_H_

__attribute__((always_inline))
static inline int *push_stack(void)
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

	return new_psp;
}

__attribute__((always_inline))
static inline void pop_stack(int* stack)
{
	register int *old_psp asm("r0") = stack;

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


#endif /* SRC_STACK_M0_H_ */
