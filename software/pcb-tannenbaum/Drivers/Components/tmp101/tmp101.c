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

static bool tmp101_read_u16(tmp101_handle_t *handle, tmp101_register_t reg, uint16_t *buffer)
{
	if (HAL_I2C_Master_Transmit(handle->i2c_handle, handle->device_address << 1u, (*uint8_t)&reg, sizeof(command_buffer),
	                            TMP101_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	if (HAL_I2C_Master_Receive(handle->i2c_handle, handle->device_address << 1u, (uint8_t*)buffer, 2, TMP101_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	return true;
}

bool tmp101_read_temperature(tmp101_handle_t *handle)
{
	uint8_t config = 0b00000000;

	// Write config register to the tmp
	if (!tmp101_write_register(handle, TMP101_REGISTER_CONFIG, config)) return false;

	uint16_t temperature;

	if (!tmp101_read_u16(handle, TMP101_REGISTER_TEMPERATURE, &temperature)) return false;

	return true;
}