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
	LL_I2C_HandleTransfer(handle->i2c_handle, (handle->device_address << 1u), LL_I2C_ADDRSLAVE_7BIT, 1,
	                      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	while (!LL_I2C_IsActiveFlag_STOP(handle->i2c_handle)) {
		if (LL_I2C_IsActiveFlag_TXIS(handle->i2c_handle)) {
			LL_I2C_TransmitData8(handle->i2c_handle, reg);
		}
	}

	LL_I2C_ClearFlag_STOP(handle->i2c_handle);

	return true;
}

static bool tmp101_read(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t *buffer, size_t size)
{
	LL_I2C_HandleTransfer(handle->i2c_handle, (handle->device_address << 1u), LL_I2C_ADDRSLAVE_7BIT, 1,
	                      LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

	while (!LL_I2C_IsActiveFlag_TC(handle->i2c_handle)) {
		if (LL_I2C_IsActiveFlag_TXIS(handle->i2c_handle)) {
			LL_I2C_TransmitData8(handle->i2c_handle, reg);
		}
	}

	LL_I2C_HandleTransfer(handle->i2c_handle, (handle->device_address << 1u), LL_I2C_ADDRSLAVE_7BIT, size,
	                      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);

	while (!LL_I2C_IsActiveFlag_STOP(handle->i2c_handle)) {
		if (LL_I2C_IsActiveFlag_RXNE(handle->i2c_handle)) {
			*buffer++ = LL_I2C_ReceiveData8(handle->i2c_handle);
		}
	}

	LL_I2C_ClearFlag_STOP(handle->i2c_handle);

	return true;
}

bool tmp101_read_temperature(tmp101_handle_t *handle, int16_t *temperature)
{
	uint8_t config = 0b01100000;

	// Write config register to the tmp
	if (!tmp101_write_register(handle, TMP101_REGISTER_CONFIG, config)) return false;

	LL_mDelay(500);

	uint8_t buffer[2] = {0};

	if (!tmp101_read(handle, TMP101_REGISTER_TEMPERATURE, buffer, 2)) return false;

	*temperature = (int16_t)((uint16_t)(buffer[0] << 4u) | ((uint16_t)(buffer[1] >> 4u) & 0x0fu));

	return true;
}