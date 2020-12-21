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

static bool i2c_transmit(I2C_TypeDef *i2c_handle, uint8_t device_address, uint8_t *data, size_t length)
{
	LL_I2C_HandleTransfer(i2c_handle, device_address << 1u, LL_I2C_ADDRSLAVE_7BIT, length,
	                      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	while (!LL_I2C_IsActiveFlag_STOP(i2c_handle)) {
		if (LL_I2C_IsActiveFlag_TXIS(i2c_handle)) {
			LL_I2C_TransmitData8(i2c_handle, (*data++));
		}
	}

	LL_I2C_ClearFlag_STOP(i2c_handle);

	return true;
}

static bool i2c_receive(I2C_TypeDef *i2c_handle, uint8_t device_address, uint8_t *buffer, size_t length)
{
	LL_I2C_HandleTransfer(i2c_handle, device_address << 1u, LL_I2C_ADDRSLAVE_7BIT, length,
	                      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	while (!LL_I2C_IsActiveFlag_STOP(i2c_handle)) {
		if (LL_I2C_IsActiveFlag_RXNE(i2c_handle)) {
			*buffer++ = LL_I2C_ReceiveData8(i2c_handle);
		}
	}

	LL_I2C_ClearFlag_STOP(i2c_handle);

	return true;
}

static bool tmp101_write_register(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t value)
{
	uint8_t command_buffer[2] = {reg, value};
	return i2c_transmit(handle->i2c_handle, handle->device_address, command_buffer, sizeof(command_buffer));
}

static bool tmp101_read(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t *buffer, size_t size)
{
	if (!i2c_transmit(handle->i2c_handle, handle->device_address, (uint8_t*)&reg, 1)) return false;
	return i2c_receive(handle->i2c_handle, handle->device_address, buffer, size);
}

bool tmp101_read_temperature(tmp101_handle_t *handle, int16_t *temperature)
{
	uint8_t config = 0b01100000;

	// Write config register to the tmp
	if (!tmp101_write_register(handle, TMP101_REGISTER_CONFIG, config)) return false;

	LL_mDelay(500);

	uint8_t buffer[2];

	if (!tmp101_read(handle, TMP101_REGISTER_TEMPERATURE, buffer, 2)) return false;

	*temperature = (int16_t)((uint16_t)(buffer[0] << 4u) | ((uint16_t)(buffer[1] >> 4u) & 0x0fu));

	return true;
}