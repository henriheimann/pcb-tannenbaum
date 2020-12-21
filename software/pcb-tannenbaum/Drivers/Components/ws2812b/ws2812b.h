#pragma once

#include <stdbool.h>
#include <stdint.h>

#if (defined STM32L011xx) || (defined STM32L021xx) || \
	(defined STM32L031xx) || (defined STM32L041xx) || \
	(defined STM32L051xx) || (defined STM32L052xx) || (defined STM32L053xx) || \
	(defined STM32L061xx) || (defined STM32L062xx) || (defined STM32L063xx) || \
	(defined STM32L071xx) || (defined STM32L072xx) || (defined STM32L073xx) || \
	(defined STM32L081xx) || (defined STM32L082xx) || (defined STM32L083xx)
#include "stm32l0xx_ll_tim.h"
#include "stm32l0xx_ll_dma.h"
#else
#error Platform not implemented
#endif

#define WS2812B_NUM_COLORS  3

typedef struct {

	uint8_t num_leds;

	TIM_TypeDef *timer_handle;

	uint32_t timer_channel;

	/**
	 * Must be of size @see{num_leds} * WS2812B_NUM_COLORS
	 */
	uint8_t led_color_buffer[];

} ws2812b_handle_t;

bool ws2812b_transmit(ws2812b_handle_t *handle);
void ws2812b_finished_isr();
void ws2812b_half_finished_isr();
