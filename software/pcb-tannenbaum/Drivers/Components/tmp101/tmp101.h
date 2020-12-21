#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if (defined STM32L011xx) || (defined STM32L021xx) || \
	(defined STM32L031xx) || (defined STM32L041xx) || \
	(defined STM32L051xx) || (defined STM32L052xx) || (defined STM32L053xx) || \
	(defined STM32L061xx) || (defined STM32L062xx) || (defined STM32L063xx) || \
	(defined STM32L071xx) || (defined STM32L072xx) || (defined STM32L073xx) || \
	(defined STM32L081xx) || (defined STM32L082xx) || (defined STM32L083xx)
#include "stm32l0xx_ll_i2c.h"
#include "stm32l0xx_ll_utils.h"
#else
#error Platform not implemented
#endif

#ifndef TMP101_I2C_TIMEOUT
#define TMP101_I2C_TIMEOUT 30
#endif

#define TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_LOW      0x48
#define TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_FLOAT    0x49
#define TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_HIGH     0x4A

/**
 * Structure defining a handle describing a TMP101 device.
 */
typedef struct {

	/**
	 * The handle to the I2C bus for the device.
	 */
	I2C_TypeDef *i2c_handle;

	/**
	 * The I2C device address.
	 * @see{TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_LOW}, @see{TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_FLOAT} and
	 * @see{TMP101_I2C_DEVICE_ADDRESS_ADD0_PIN_HIGH}
	 */
	uint16_t device_address;

} tmp101_handle_t;

/**
 * Uses the TMP101 sensor to take a single 12-bit measurement.
 * @param handle The handle to the TMP101 sensor.
 * @param temperature Buffer for the temperature sensor. Signed 8.4 fixed point value.
 * @return True on success, false otherwise.
 */
bool tmp101_read_temperature(tmp101_handle_t *handle, int16_t *temperature);
