#include <tmp101/tmp101.h>
#include <i2c.h>
#include <ws2812b/ws2812b.h>

#include "application.h"

static tmp101_handle_t tmp101_handle = {
		.i2c_handle = I2C1,
		.device_address = TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_HIGH
};

static ws2812b_handle_t ws2812b_handle = {
	.timer_handle = TIM2,
	.timer_channel = LL_TIM_CHANNEL_CH2,
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
	//ws2812b_transmit(&ws2812b_handle);

	uint32_t ticks_elapsed = 0;
	bool last_button_state = false;
	bool instant_update = false;
	int state = 0;

	while (1) {
		if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) {
			ticks_elapsed++;
		}

		if (LL_GPIO_ReadInputPort(USER_BUTTON_GPIO_Port) & USER_BUTTON_Pin) {
			last_button_state = false;
		} else if (last_button_state == false) {
			last_button_state = true;
			instant_update = true;
			state = (state + 1) % 2;
		}

		if (instant_update || ticks_elapsed > 250) {
			ticks_elapsed = 0;
			instant_update = false;

			/*int16_t temperature;
			tmp101_read_temperature(&tmp101_handle, &temperature);
			if (temperature > 0) {
				ws2812b_handle.led_color_buffer[0] = 0;
			}*/

			for (int led_index = 0; led_index < 12; led_index++) {
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
						ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 5;
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

			ws2812b_transmit(&ws2812b_handle);
		}
	}
}