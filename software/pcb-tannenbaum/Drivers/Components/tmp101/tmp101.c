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

typedef enum
{
	TMP101_THERMOSTAT_MODE_COMPARATOR = 0x01,
	TMP101_THERMOSTAT_MODE_INTERRUPT = 0x00
} tmp101_thermostat_mode_t;

typedef struct
{
	union {
		struct {
			bool shutdown : 1;
			tmp101_thermostat_mode_t thermostat_mode : 1;
			bool polarity : 1;
			uint8_t fault_queue : 2;
			tmp101_resolution_t resolution : 2;
			bool os_alert : 1;
		};
		uint8_t register_value;
	};
} __attribute__ ((packed)) tmp101_config_register_t;

static bool tmp101_write(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t value)
{
	uint32_t timeout = TMP101_I2C_TIMEOUT;

	uint8_t buffer[2] = { reg, value };
	uint8_t *buffer_ptr = buffer;

	LL_I2C_HandleTransfer(handle->i2c_handle, (handle->device_address << 1u), LL_I2C_ADDRSLAVE_7BIT, 2,
	                      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	while (!LL_I2C_IsActiveFlag_STOP(handle->i2c_handle)) {
		if (LL_I2C_IsActiveFlag_TXIS(handle->i2c_handle)) {
			LL_I2C_TransmitData8(handle->i2c_handle, *buffer_ptr++);
			timeout = TMP101_I2C_TIMEOUT;
		}
		if (LL_SYSTICK_IsActiveCounterFlag()) {
			if (timeout-- == 0) {
				return false;
			}
		}
	}

	LL_I2C_ClearFlag_STOP(handle->i2c_handle);

	return true;
}

static bool tmp101_read(tmp101_handle_t *handle, tmp101_register_t reg, uint8_t *buffer, size_t size)
{
	uint32_t timeout = TMP101_I2C_TIMEOUT;

	LL_I2C_HandleTransfer(handle->i2c_handle, (handle->device_address << 1u), LL_I2C_ADDRSLAVE_7BIT, 1,
	                      LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

	while (!LL_I2C_IsActiveFlag_TC(handle->i2c_handle)) {
		if (LL_I2C_IsActiveFlag_TXIS(handle->i2c_handle)) {
			LL_I2C_TransmitData8(handle->i2c_handle, reg);
		}
		if (LL_SYSTICK_IsActiveCounterFlag()) {
			if (timeout-- == 0) {
				return false;
			}
		}
	}

	timeout = TMP101_I2C_TIMEOUT;

	LL_I2C_HandleTransfer(handle->i2c_handle, (handle->device_address << 1u), LL_I2C_ADDRSLAVE_7BIT, size,
	                      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);

	while (!LL_I2C_IsActiveFlag_STOP(handle->i2c_handle)) {
		if (LL_I2C_IsActiveFlag_RXNE(handle->i2c_handle)) {
			*buffer++ = LL_I2C_ReceiveData8(handle->i2c_handle);
			timeout = TMP101_I2C_TIMEOUT;
		}
		if (LL_SYSTICK_IsActiveCounterFlag()) {
			if (timeout-- == 0) {
				return false;
			}
		}
	}

	LL_I2C_ClearFlag_STOP(handle->i2c_handle);

	return true;
}

bool tmp101_read_temperature(tmp101_handle_t *handle, tmp101_resolution_t resolution, int16_t *temperature)
{
	tmp101_config_register_t config = {
			.shutdown = true,
			.thermostat_mode = TMP101_THERMOSTAT_MODE_INTERRUPT,
			.polarity = false,
			.fault_queue = 0,
			.resolution = resolution,
			.os_alert = true
	};

	// Write config register to the tmp
	if (!tmp101_write(handle, TMP101_REGISTER_CONFIG, config.register_value)) return false;

	// Wait for single shot conversion to complete
	do {
		if (!tmp101_read(handle, TMP101_REGISTER_CONFIG, &config.register_value, 1)) return false;
	} while (!config.shutdown);

	// Read and convert temperature value
	uint8_t buffer[2] = {0};
	if (!tmp101_read(handle, TMP101_REGISTER_TEMPERATURE, buffer, 2)) return false;
	*temperature = (int16_t)((uint16_t)(buffer[0] << 4u) | ((uint16_t)(buffer[1] >> 4u) & 0x0fu));

	return true;
}