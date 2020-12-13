#include <tmp101/tmp101.h>
#include <i2c.h>
#include <tim.h>
#include <ws2812b/ws2812b.h>

#include "application.h"

static tmp101_handle_t tmp101_handle = {
		.i2c_handle = &hi2c1,
		.device_address = TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_HIGH
};

static ws2812b_handle_t ws2812b_handle = {
	.timer_handle = &htim2,
	.timer_channel = TIM_CHANNEL_2,
	.num_leds = 12,
	.led_color_buffer = {
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0
	}
};

static uint32_t random()
{
	static uint32_t x32 = 314159265;
	x32 ^= x32 << 13u;
	x32 ^= x32 >> 17u;
	x32 ^= x32 << 5u;
	return x32;
}

_Noreturn void application_main()
{
	ws2812b_transmit(&ws2812b_handle);

	uint32_t last_update = HAL_GetTick();
	bool last_button_state = false;
	bool instant_update = false;
	int state = 0;

	while (1) {

		if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET) {
			last_button_state = false;
		} else if (last_button_state == false) {
			last_button_state = true;
			instant_update = true;
			state = (state + 1) % 2;
		}

		if (instant_update || ((HAL_GetTick() - last_update) > 250)) {
			last_update = HAL_GetTick();
			instant_update = false;

			if (state == 0) {
				for (uint8_t led_index = 0; led_index < 12 * WS2812B_NUM_COLORS; led_index++) {
					ws2812b_handle.led_color_buffer[led_index] = 0;
				}
			} else if (state == 1) {
				for (uint8_t led_index = 0; led_index < 12; led_index++) {
					switch (random() % 6) {
						case 0:
							ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 1;
							ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 0;
							ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
							break;
						case 1:
							ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 0;
							ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 1;
							ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
							break;
						case 2:
							ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 0;
							ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 0;
							ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 7;
							break;
						case 3:
							ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 1;
							ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 1;
							ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
							break;
						case 4:
							ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 0;
							ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 1;
							ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 1;
							break;
						case 5:
							ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 1;
							ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 0;
							ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 1;
							break;
					}
				}
			}

			ws2812b_transmit(&ws2812b_handle);
		}
	}
}