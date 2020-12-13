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
				pwm_timings_buffer[timings_offset - i] = 20;
			} else {
				pwm_timings_buffer[timings_offset - i] = 8;
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

	__HAL_TIM_SET_COMPARE(handle->timer_handle, handle->timer_channel, 0);
	HAL_TIM_Base_Start(handle->timer_handle);

	if (HAL_TIM_PWM_Start_DMA(handle->timer_handle, handle->timer_channel,
						   (uint32_t*)pwm_timings_buffer, sizeof(pwm_timings_buffer))) return false;

	while (next_led_index < handle->num_leds) {
		if (pwm_pulse_half_finished) {
			pwm_pulse_half_finished = false;
			if (next_led_index < handle->num_leds) {
				prepare_led_timings(handle, next_led_index++, PWM_TIMINGS_BUFFER_LED0);
			} else {
				prepare_led_zeros(PWM_TIMINGS_BUFFER_LED0);
			}
		} else if (pwm_pulse_finished) {
			pwm_pulse_finished = false;
			if (next_led_index < handle->num_leds) {
				prepare_led_timings(handle, next_led_index++, PWM_TIMINGS_BUFFER_LED1);
			} else {
				prepare_led_zeros(PWM_TIMINGS_BUFFER_LED1);
			}
		}
	}

	HAL_TIM_PWM_Stop_DMA(handle->timer_handle, handle->timer_channel);
	HAL_TIM_Base_Stop(handle->timer_handle);

	return true;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	pwm_pulse_finished = true;
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	pwm_pulse_half_finished = true;
}