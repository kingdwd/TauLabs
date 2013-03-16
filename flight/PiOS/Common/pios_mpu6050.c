/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6050 MPU6050 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_mpu6050.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @author     Tau Labs, http://www.taulabs.org Copyright (C) 2013.
 * @brief      MPU6050 6-axis gyro and accel chip
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Project Includes */
#include "pios.h"
#include "physical_constants.h"

#if defined(PIOS_INCLUDE_MPU6050)

/* Private constants */
#define MPU6050_TASK_PRIORITY	(tskIDLE_PRIORITY + configMAX_PRIORITIES - 1)	// max priority
#define MPU6050_TASK_STACK		(384 / 4)

/* Global Variables */

enum pios_mpu6050_dev_magic
{
	PIOS_MPU6050_DEV_MAGIC = 0xf21d26a2,
};

#define PIOS_MPU6000_MAX_QUEUESIZE 2

struct mpu6050_dev
{
	uint32_t i2c_id;
	uint8_t i2c_addr;
	enum pios_mpu60x0_accel_range accel_range;
	enum pios_mpu60x0_range gyro_range;
	xQueueHandle gyro_queue;
	xQueueHandle accel_queue;
	xTaskHandle TaskHandle;
	xSemaphoreHandle data_ready_sema;
	const struct pios_mpu60x0_cfg* cfg;
	enum pios_mpu6050_dev_magic magic;
};

//! Global structure for this device device
static struct mpu6050_dev* dev;

//! Private functions
static struct mpu6050_dev* PIOS_MPU6050_alloc(void);
static int32_t PIOS_MPU6050_Validate(struct mpu6050_dev* dev);
static void PIOS_MPU6050_Config(const struct pios_mpu60x0_cfg* cfg);
static int32_t PIOS_MPU6050_SetReg(uint8_t address, uint8_t buffer);
static int32_t PIOS_MPU6050_GetReg(uint8_t address);
static int32_t PIOS_MPU6050_ReadID();
static void PIOS_MPU6050_Task(void* parameters);

/**
 * @brief Allocate a new device
 */
static struct mpu6050_dev* PIOS_MPU6050_alloc(void)
{
	struct mpu6050_dev* mpu6050_dev;

	mpu6050_dev = (struct mpu6050_dev*)pvPortMalloc(sizeof(*mpu6050_dev));

	if (!mpu6050_dev) return (NULL);

	mpu6050_dev->magic = PIOS_MPU6050_DEV_MAGIC;

	mpu6050_dev->accel_queue = xQueueCreate(PIOS_MPU6000_MAX_QUEUESIZE, sizeof(struct pios_sensor_gyro_data));

	if (mpu6050_dev->accel_queue == NULL)
	{
		vPortFree(mpu6050_dev);
		return NULL;
	}

	mpu6050_dev->gyro_queue = xQueueCreate(PIOS_MPU6000_MAX_QUEUESIZE, sizeof(struct pios_sensor_gyro_data));

	if (mpu6050_dev->gyro_queue == NULL)
	{
		vPortFree(mpu6050_dev);
		return NULL;
	}

	mpu6050_dev->data_ready_sema = xSemaphoreCreateMutex();

	if (mpu6050_dev->data_ready_sema == NULL)
	{
		vPortFree(mpu6050_dev);
		return NULL;
	}

	return(mpu6050_dev);
}

/**
 * @brief Validate the handle to the i2c device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_MPU6050_Validate(struct mpu6050_dev* dev)
{
	if (dev == NULL)
		return -1;

	if (dev->magic != PIOS_MPU6050_DEV_MAGIC)
		return -2;

	if (dev->i2c_id == 0)
		return -3;

	return 0;
}

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor.
 * @return 0 for success, -1 for failure
 */
int32_t PIOS_MPU6050_Init(uint32_t i2c_id, uint8_t i2c_addr, const struct pios_mpu60x0_cfg* cfg)
{
	dev = PIOS_MPU6050_alloc();

	if (dev == NULL)
		return -1;

	dev->i2c_id = i2c_id;
	dev->i2c_addr = i2c_addr;
	dev->cfg = cfg;

	/* Configure the MPU6050 Sensor */
	PIOS_MPU6050_Config(cfg);

	int result = xTaskCreate(PIOS_MPU6050_Task, (const signed char*)"pios_mpu6050",
	                         MPU6050_TASK_STACK, NULL, MPU6050_TASK_PRIORITY,
	                         &dev->TaskHandle);
	PIOS_Assert(result == pdPASS);

	/* Set up EXTI line */
	PIOS_EXTI_Init(cfg->exti_cfg);

	PIOS_SENSORS_Register(PIOS_SENSOR_ACCEL, dev->accel_queue);
	PIOS_SENSORS_Register(PIOS_SENSOR_GYRO, dev->gyro_queue);

	return 0;
}

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_MPU6050_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_MPU6050_Config(struct pios_mpu60x0_cfg const* cfg)
{
	// Reset chip
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_PWR_MGMT_REG, 0x80 | cfg->Pwr_mgmt_clk);

	do
	{
		PIOS_DELAY_WaitmS(5);
	}
	while (PIOS_MPU6050_GetReg(PIOS_MPU60X0_PWR_MGMT_REG) & 0x80);

	PIOS_DELAY_WaitmS(25);

	//Power management configuration
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_PWR_MGMT_REG, cfg->Pwr_mgmt_clk);

#if defined(PIOS_MPU6050_ACCEL)
	// Set the accel scale
	PIOS_MPU6050_SetAccelRange(PIOS_MPU60X0_ACCEL_8G);
#endif

	// Sample rate divider
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_SMPLRT_DIV_REG, cfg->Smpl_rate_div);

	// Digital low-pass filter and scale
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_DLPF_CFG_REG, cfg->filter);

	// Digital low-pass filter and scale
	PIOS_MPU6050_SetGyroRange(PIOS_MPU60X0_SCALE_500_DEG);

	// Interrupt configuration
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_USER_CTRL_REG, cfg->User_ctl);

	//Power management configuration
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_PWR_MGMT_REG, cfg->Pwr_mgmt_clk);

	// Interrupt configuration
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_INT_CFG_REG, cfg->interrupt_cfg);

	// Interrupt configuration
	PIOS_MPU6050_SetReg(PIOS_MPU60X0_INT_EN_REG, cfg->interrupt_en);
}

/**
 * Set the gyro range and store it locally for scaling
 */
void PIOS_MPU6050_SetGyroRange(enum pios_mpu60x0_range gyro_range)
{
	while (PIOS_MPU6050_SetReg(PIOS_MPU60X0_GYRO_CFG_REG, gyro_range) != 0);

	dev->gyro_range = gyro_range;
}

/**
 * Set the accel range and store it locally for scaling
 */
void PIOS_MPU6050_SetAccelRange(enum pios_mpu60x0_accel_range accel_range)
{
	while (PIOS_MPU6050_SetReg(PIOS_MPU60X0_ACCEL_CFG_REG, accel_range) != 0);

	dev->accel_range = accel_range;
}

/**
 * @brief Reads one or more bytes into a buffer
 * \param[in] address MPU6050 register address (depends on size)
 * \param[out] buffer destination buffer
 * \param[in] len number of bytes which should be read
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6050_Read(uint8_t address, uint8_t* buffer, uint8_t len)
{
	uint8_t addr_buffer[] =
	{
		address,
	};

	const struct pios_i2c_txn txn_list[] =
	{
		{
			.info = __func__,
			.addr = dev->i2c_addr,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(addr_buffer),
			.buf = addr_buffer,
		},
		{
			.info = __func__,
			.addr = dev->i2c_addr,
			.rw = PIOS_I2C_TXN_READ,
			.len = len,
			.buf = buffer,
		}
	};

	return PIOS_I2C_Transfer(dev->i2c_id, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Writes one or more bytes to the MPU6050
 * \param[in] address Register address
 * \param[in] buffer source buffer
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6050_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] =
	{
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] =
	{
		{
			.info = __func__,
			.addr = dev->i2c_addr,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(data),
			.buf = data,
		},
	};

	return PIOS_I2C_Transfer(dev->i2c_id, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Read a register from MPU6050
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
static int32_t PIOS_MPU6050_GetReg(uint8_t reg)
{
	uint8_t data;

	int32_t retval = PIOS_MPU6050_Read(reg, &data, sizeof(data));

	if (retval != 0)
		return retval;
	else
		return data;
}

/**
 * @brief Writes one byte to the MPU6050
 * \param[in] reg Register address
 * \param[in] data Byte to write
 * \return 0 if operation was successful
 * \return -1 if unable to claim SPI bus
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6050_SetReg(uint8_t reg, uint8_t data)
{
	return PIOS_MPU6050_Write(reg, data);
}

/*
 * @brief Read the identification bytes from the MPU6050 sensor
 * \return ID read from MPU6050 or -1 if failure
*/
static int32_t PIOS_MPU6050_ReadID()
{
	int32_t mpu6050_id = PIOS_MPU6050_GetReg(PIOS_MPU60X0_WHOAMI);

	if (mpu6050_id < 0)
		return -1;

	return mpu6050_id;
}


static float PIOS_MPU6050_GetGyroScale()
{
	switch (dev->gyro_range)
	{
	case PIOS_MPU60X0_SCALE_250_DEG:
		return 1.0f / 131.0f;

	case PIOS_MPU60X0_SCALE_500_DEG:
		return 1.0f / 65.5f;

	case PIOS_MPU60X0_SCALE_1000_DEG:
		return 1.0f / 32.8f;

	case PIOS_MPU60X0_SCALE_2000_DEG:
		return 1.0f / 16.4f;
	}

	return 0;
}

#if defined(PIOS_MPU6050_ACCEL)
static float PIOS_MPU6050_GetAccelScale()
{
	switch (dev->accel_range)
	{
	case PIOS_MPU60X0_ACCEL_2G:
		return GRAVITY / 16384.0f;

	case PIOS_MPU60X0_ACCEL_4G:
		return GRAVITY / 8192.0f;

	case PIOS_MPU60X0_ACCEL_8G:
		return GRAVITY / 4096.0f;

	case PIOS_MPU60X0_ACCEL_16G:
		return GRAVITY / 2048.0f;
	}

	return 0;
}
#endif /* PIOS_MPU6050_ACCEL */

/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
uint8_t PIOS_MPU6050_Test(void)
{
	/* Verify that ID matches (MPU6050 ID is 0x68) */
	int32_t mpu6050_id = PIOS_MPU6050_ReadID();

	if (mpu6050_id < 0)
		return -1;

	if (mpu6050_id != 0x68)
		return -2;

	return 0;
}

/**
* @brief IRQ Handler.  Read all the data from onboard buffer
*/
bool PIOS_MPU6050_IRQHandler(void)
{
	if (PIOS_MPU6050_Validate(dev) != 0)
		return false;

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(dev->data_ready_sema, &xHigherPriorityTaskWoken);

	return xHigherPriorityTaskWoken == pdTRUE;
}

static void PIOS_MPU6050_Task(void* parameters)
{
	while (1)
	{
		//Wait for data ready interrupt
		if (xSemaphoreTake(dev->data_ready_sema, portMAX_DELAY) != pdTRUE)
			continue;

		enum
		{
		    IDX_ACCEL_XOUT_H = 0,
		    IDX_ACCEL_XOUT_L,
		    IDX_ACCEL_YOUT_H,
		    IDX_ACCEL_YOUT_L,
		    IDX_ACCEL_ZOUT_H,
		    IDX_ACCEL_ZOUT_L,
		    IDX_TEMP_OUT_H,
		    IDX_TEMP_OUT_L,
		    IDX_GYRO_XOUT_H,
		    IDX_GYRO_XOUT_L,
		    IDX_GYRO_YOUT_H,
		    IDX_GYRO_YOUT_L,
		    IDX_GYRO_ZOUT_H,
		    IDX_GYRO_ZOUT_L,
		    BUFFER_SIZE,
		};

		static uint8_t mpu6050_rec_buf[BUFFER_SIZE];

		if (PIOS_MPU6050_Read(PIOS_MPU60X0_ACCEL_X_OUT_MSB, mpu6050_rec_buf, sizeof(mpu6050_rec_buf)) < 0)
		{
			continue;
		}

		// Rotate the sensor to OP convention.  The datasheet defines X as towards the right
		// and Y as forward.  OP convention transposes this.  Also the Z is defined negatively
		// to our convention

#if defined(PIOS_MPU6050_ACCEL)

		// Currently we only support rotations on top so switch X/Y accordingly
		struct pios_sensor_accel_data accel_data;
		struct pios_sensor_gyro_data gyro_data;

		switch (dev->cfg->orientation)
		{
		case PIOS_MPU60X0_TOP_0DEG:
			accel_data.y = (int16_t)(mpu6050_rec_buf[IDX_ACCEL_XOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_XOUT_L]);
			accel_data.x = (int16_t)(mpu6050_rec_buf[IDX_ACCEL_YOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_YOUT_L]);
			gyro_data.y  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			gyro_data.x  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			break;

		case PIOS_MPU60X0_TOP_90DEG:
			accel_data.y = (int16_t) - (mpu6050_rec_buf[IDX_ACCEL_YOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_YOUT_L]);
			accel_data.x = (int16_t)(mpu6050_rec_buf[IDX_ACCEL_XOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_XOUT_L]);
			gyro_data.y  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			gyro_data.x  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			break;

		case PIOS_MPU60X0_TOP_180DEG:
			accel_data.y = (int16_t) - (mpu6050_rec_buf[IDX_ACCEL_XOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_XOUT_L]);
			accel_data.x = (int16_t) - (mpu6050_rec_buf[IDX_ACCEL_YOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_YOUT_L]);
			gyro_data.y  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			gyro_data.x  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			break;

		case PIOS_MPU60X0_TOP_270DEG:
			accel_data.y = (int16_t)(mpu6050_rec_buf[IDX_ACCEL_YOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_YOUT_L]);
			accel_data.x = (int16_t) - (mpu6050_rec_buf[IDX_ACCEL_XOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_XOUT_L]);
			gyro_data.y  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			gyro_data.x  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			break;
		}

		gyro_data.z  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_ZOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_ZOUT_L]);
		accel_data.z = (int16_t) - (mpu6050_rec_buf[IDX_ACCEL_ZOUT_H] << 8 | mpu6050_rec_buf[IDX_ACCEL_ZOUT_L]);

		int16_t raw_temp = (mpu6050_rec_buf[IDX_TEMP_OUT_H] << 8 | mpu6050_rec_buf[IDX_TEMP_OUT_L]);
		float temperature = 35.0f + ((float) raw_temp + 512.0f) / 340.0f;

		// Apply sensor scaling
		float accel_scale = PIOS_MPU6050_GetAccelScale();
		accel_data.x *= accel_scale;
		accel_data.y *= accel_scale;
		accel_data.z *= accel_scale;
		accel_data.temperature = temperature;

		float gyro_scale = PIOS_MPU6050_GetGyroScale();
		gyro_data.x *= gyro_scale;
		gyro_data.y *= gyro_scale;
		gyro_data.z *= gyro_scale;
		gyro_data.temperature = temperature;

		portBASE_TYPE xHigherPriorityTaskWoken_accel;
		xQueueSendToBackFromISR(dev->accel_queue, (void*) &accel_data, &xHigherPriorityTaskWoken_accel);

		portBASE_TYPE xHigherPriorityTaskWoken_gyro;
		xQueueSendToBackFromISR(dev->gyro_queue, (void*) &gyro_data, &xHigherPriorityTaskWoken_gyro);

#else

		struct pios_sensor_gyro_data gyro_data;

		switch (dev->cfg->orientation)
		{
		case PIOS_MPU60X0_TOP_0DEG:
			gyro_data.y  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			gyro_data.x  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			break;

		case PIOS_MPU60X0_TOP_90DEG:
			gyro_data.y  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			gyro_data.x  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			break;

		case PIOS_MPU60X0_TOP_180DEG:
			gyro_data.y  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			gyro_data.x  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			break;

		case PIOS_MPU60X0_TOP_270DEG:
			gyro_data.y  = (int16_t)(mpu6050_rec_buf[IDX_GYRO_YOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_YOUT_L]);
			gyro_data.x  = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_XOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_XOUT_L]);
			break;
		}

		gyro_data.z = (int16_t) - (mpu6050_rec_buf[IDX_GYRO_ZOUT_H] << 8 | mpu6050_rec_buf[IDX_GYRO_ZOUT_L]);

		int32_t raw_temp = (mpu6050_rec_buf[IDX_TEMP_OUT_H] << 8 | mpu6050_rec_buf[IDX_TEMP_OUT_L]);
		float temperature = 35.0f + ((float) raw_temp + 512.0f) / 340.0f;

		// Apply sensor scaling
		float gyro_scale = PIOS_MPU6050_GetGyroScale();
		gyro_data.x *= gyro_scale;
		gyro_data.y *= gyro_scale;
		gyro_data.z *= gyro_scale;
		gyro_data.temperature = temperature;

		portBASE_TYPE xHigherPriorityTaskWoken_gyro;
		xQueueSendToBackFromISR(dev->gyro_queue, (void*) &gyro_data, &xHigherPriorityTaskWoken_gyro);

#endif
	}
}

#endif

/**
 * @}
 * @}
 */
