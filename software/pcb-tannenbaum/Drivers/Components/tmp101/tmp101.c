#include "tmp101.h"

/**
 * Registers addresses.
 */
typedef enum
{
	TMP101_REGISTER_TEMPERATURE = 0x00,
	TMP101_REGISTER_CONFIG = 0x01,
	TMP101_REGISTER_TLOW = 0x02,
	TMP101_REGISTER_THIGH = 0x03
} tmp101_register_t;

static bool tmp101_write_register(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t value)
{
	uint8_t command_buffer[2] = {reg, value};

	if (HAL_I2C_Master_Transmit(handle->i2c_handle, handle->device_address << 1u, command_buffer, sizeof(command_buffer),
	                            TMP101_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	return true;
}

static bool tmp101_read(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t *buffer, size_t size)
{
	if (HAL_I2C_Master_Transmit(handle->i2c_handle, handle->device_address << 1u, (uint8_t*)&reg, 1,
	                            TMP101_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	if (HAL_I2C_Master_Receive(handle->i2c_handle, handle->device_address << 1u, buffer, size, TMP101_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	return true;
}

bool tmp101_read_temperature(tmp101_handle_t *handle, int16_t *temperature)
{
	uint8_t config = 0b01100000;

	// Write config register to the tmp
	if (!tmp101_write_register(handle, TMP101_REGISTER_CONFIG, config)) return false;

	HAL_Delay(500);

	uint8_t buffer[2];

	if (!tmp101_read(handle, TMP101_REGISTER_TEMPERATURE, buffer, 2)) return false;

	*temperature = (int16_t)((uint16_t)(buffer[0] << 4u) | ((uint16_t)(buffer[1] >> 4u) & 0x0fu));

	return true;
}