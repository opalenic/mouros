/*
 * bsp.c
 *
 *  Created on: 1. 9. 2014
 *      Author: ondra
 */

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>

#include <bsp.h>


void bsp_led_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LEDR | LEDB | LEDO | LEDG);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, LEDR | LEDB | LEDO | LEDG);
}

void bsp_led_on(enum led led)
{
	gpio_set(GPIOC, led);
}

void bsp_led_off(enum led led)
{
	gpio_clear(GPIOC, led);
}

void bsp_led_toggle(enum led led)
{
	gpio_toggle(GPIOC, led);
}

void bsp_button_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);

}

void bsp_button_exti_init(enum exti_trigger_type mode, uint8_t irq_prio)
{
	bsp_button_init();

	exti_select_source(EXTI0, GPIO0);
	exti_set_trigger(EXTI0, mode);

	exti_enable_request(EXTI0);

	nvic_set_priority(NVIC_EXTI0_1_IRQ, irq_prio);
	nvic_enable_irq(NVIC_EXTI0_1_IRQ);
}

bool bsp_button_is_pressed(void)
{
	return gpio_get(GPIOA, GPIO0);
}
