#include <stm32l0xx_ll_utils.h>
#include "ws2812b.h"

static uint8_t pwm_timings_buffer[2 * WS2812B_NUM_COLORS * 8] = {0};

#define PWM_TIMINGS_BUFFER_LED0     0
#define PWM_TIMINGS_BUFFER_LED1     (WS2812B_NUM_COLORS * 8)

static uint8_t next_led_index = 0;
static volatile bool pwm_pulse_finished = false;
static volatile bool pwm_pulse_half_finished = false;

static void prepare_led_timings(ws2812b_handle_t *handle, uint8_t led, uint8_t offset)
{
	for (uint8_t color_index = 0; color_index < WS2812B_NUM_COLORS; color_index++) {
		uint8_t color = handle->led_color_buffer[(led * WS2812B_NUM_COLORS) + color_index];
		uint8_t timings_offset = offset + color_index * 8 + 7;
		for (uint8_t i = 0; i < 8; i++) {
			if (color & 0x1u) {
				pwm_timings_buffer[timings_offset - i] = 27;
			} else {
				pwm_timings_buffer[timings_offset - i] = 11;
			}
			color >>= 1u;
		}
	}
}

static void prepare_led_zeros(uint8_t base_offset)
{
	for (uint8_t offset = 0; offset < 3 * 8; offset++) {
		pwm_timings_buffer[base_offset + offset] = 8;
	}
}

bool ws2812b_transmit(ws2812b_handle_t *handle)
{
	// TODO: Assert PWM Period matches WS2812

	prepare_led_timings(handle, 0, PWM_TIMINGS_BUFFER_LED0);
	prepare_led_timings(handle, 1, PWM_TIMINGS_BUFFER_LED1);

	pwm_pulse_finished = false;
	pwm_pulse_half_finished = false;
	next_led_index = 2;

	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&TIM2->CCR2);
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&pwm_timings_buffer);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, sizeof(pwm_timings_buffer));
	LL_TIM_OC_SetCompareCH2(TIM2, 0);
	LL_TIM_EnableDMAReq_CC2(TIM2);

	LL_DMA_ClearFlag_TC3(DMA1);
	LL_DMA_ClearFlag_HT3(DMA1);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
	LL_TIM_EnableCounter(TIM2);

	while (next_led_index < (handle->num_leds + 2)) {
		if (pwm_pulse_half_finished) {
			pwm_pulse_half_finished = false;
			if (next_led_index < handle->num_leds) {
				prepare_led_timings(handle, next_led_index, PWM_TIMINGS_BUFFER_LED0);
			} else {
				prepare_led_zeros(PWM_TIMINGS_BUFFER_LED0);
			}
			next_led_index++;
		} else if (pwm_pulse_finished) {
			pwm_pulse_finished = false;
			if (next_led_index < handle->num_leds) {
				prepare_led_timings(handle, next_led_index, PWM_TIMINGS_BUFFER_LED1);
			} else {
				prepare_led_zeros(PWM_TIMINGS_BUFFER_LED1);
			}
			next_led_index++;
		}
	}

	LL_TIM_DisableCounter(TIM2);
	LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

	return true;
}

void ws2812b_finished_isr()
{
	pwm_pulse_finished = true;
}

void ws2812b_half_finished_isr()
{
	pwm_pulse_half_finished = true;
}