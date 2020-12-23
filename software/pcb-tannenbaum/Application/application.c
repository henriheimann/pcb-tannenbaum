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

static volatile bool user_button_pressed = false;

static uint32_t random()
{
	static uint32_t x32 = 314159265;
	x32 ^= x32 << 13u;
	x32 ^= x32 >> 17u;
	x32 ^= x32 << 5u;
	return x32;
}

void enter_standby_mode()
{
	for (uint8_t led_index = 0; led_index < 12; led_index++) {
		ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 0;
		ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 0;
		ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
	}
	ws2812b_transmit(&ws2812b_handle);

	LL_PWR_EnableUltraLowPower();
	LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
	LL_LPM_EnableDeepSleep();

#if defined ( __CC_ARM)
	__force_stores();
#endif

	__WFI();
}


void application_user_button_isr()
{
	user_button_pressed = true;
}

_Noreturn void application_main()
{
	ws2812b_transmit(&ws2812b_handle);

	uint32_t ticks_elapsed = 500;

	while (1) {
		if (ticks_elapsed > 500) {
			ticks_elapsed = 0;

			int16_t temperature;
			tmp101_read_temperature(&tmp101_handle, TMP101_RESOLUTION_9BITS, &temperature);
			temperature = temperature / 16;

			uint32_t voltage_below_2_5V = LL_PWR_IsActiveFlag_PVDO();

			for (uint8_t led_index = 0; led_index < 12; led_index++) {
				uint8_t random_number;
				if (temperature >= 28) {
					random_number = random() % 2;
				} else if (voltage_below_2_5V) {
					random_number = random() % 3;
				} else {
					random_number = random() % 6;
				}
				switch (random_number) {
					case 0:
					default:
						ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 1;
						ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 1;
						ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
						break;
					case 1:
						ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 0;
						ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 1;
						ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
						break;
					case 2:
						ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 1;
						ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 0;
						ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 0;
						break;
					case 3:
						ws2812b_handle.led_color_buffer[led_index * 3 + 0] = 0;
						ws2812b_handle.led_color_buffer[led_index * 3 + 1] = 0;
						ws2812b_handle.led_color_buffer[led_index * 3 + 2] = 3;
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

		if (LL_SYSTICK_IsActiveCounterFlag()) {
			ticks_elapsed++;
		}

		if (user_button_pressed) {
			user_button_pressed = false;
			enter_standby_mode();
		}
	}
}