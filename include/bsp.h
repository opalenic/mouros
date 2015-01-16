/*
 * bsp.h
 *
 *  Created on: 2. 9. 2014
 *      Author: ondra
 */

#ifndef BSP_H_
#define BSP_H_

#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>

#define BSP_BUTTON_EXTI EXTI0
#define BSP_BUTTON_EXTI_ISR exti0_1_isr

enum led {
	LEDR = GPIO6,
	LEDB = GPIO7,
	LEDO = GPIO8,
	LEDG = GPIO9
};


void bsp_led_init(void);
void bsp_led_on(enum led led);
void bsp_led_off(enum led led);
void bsp_led_toggle(enum led led);


void bsp_button_init(void);
void bsp_button_exti_init(enum exti_trigger_type mode, uint8_t irq_prio);
bool bsp_button_is_pressed(void);


#endif /* BSP_H_ */
