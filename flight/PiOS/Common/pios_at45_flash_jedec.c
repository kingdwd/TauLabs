/*
 ******************************************************************************
 * @file       pios_at45_flash_jedec.c
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2013
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS Flash Filesystem Function
 * @{
 * @brief Log Structured Filesystem for internal or external NOR Flash
 *****************************************************************************/
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
#include "pios.h"
#include "pios_at45_flash_jedec_priv.h"



// AT45DB161D Commands (from Datasheet)
#define JEDEC_PAGE_TO_BUFFER_1        0x53
#define JEDEC_PAGE_TO_BUFFER_2        0x55
#define JEDEC_READ_STATUS             0xD7
#define JEDEC_DEVICE_ID               0x9F
#define JEDEC_PAGE_WRITE              0x02
#define JEDEC_BUFFER1_WRITE           0x84
#define JEDEC_BUFFER2_WRITE           0x87
#define JEDEC_PAGE_READ               0xD2
#define JEDEC_BUFFER1_READ            0xD4
#define JEDEC_BUFFER2_READ            0xD6
#define JEDEC_PAGE_ERASE              0x81
#define JEDEC_MM_PAGE_PROGRAM         0x82
#define JEDEC_BUFFER1_MM_PROGRAM      0x83
#define JEDEC_BUFFER2_MM_PROGRAM      0x86

#define AT45DB161D_CHIP_ERASE_0       0xC7
#define AT45DB161D_CHIP_ERASE_1       0x94
#define AT45DB161D_CHIP_ERASE_2       0x80
#define AT45DB161D_CHIP_ERASE_3       0x9A

#define JEDEC_STATUS_BUSY             0x80

enum pios_jedec_dev_magic {
	PIOS_JEDEC_DEV_MAGIC = 0xcb55aa55,
};

//! Device handle structure
struct jedec_flash_dev {
	uint32_t spi_id;
	uint32_t slave_num;
	bool claimed;

	uint8_t manufacturer;
	uint8_t memorytype;
	uint8_t capacity;

	const struct pios_flash_jedec_cfg * cfg;
#if defined(FLASH_FREERTOS)
	xSemaphoreHandle transaction_lock;
#endif
	enum pios_jedec_dev_magic magic;
};

//! Private functions
static int32_t PIOS_Flash_Jedec_Validate(struct jedec_flash_dev * flash_dev);
static struct jedec_flash_dev * PIOS_Flash_Jedec_alloc(void);

static int32_t PIOS_Flash_Jedec_ReadID(struct jedec_flash_dev * flash_dev);
static int32_t PIOS_Flash_Jedec_ReadStatus(struct jedec_flash_dev * flash_dev);
static int32_t PIOS_Flash_Jedec_ClaimBus(struct jedec_flash_dev * flash_dev);
static int32_t PIOS_Flash_Jedec_ReleaseBus(struct jedec_flash_dev * flash_dev);
static int32_t PIOS_Flash_Jedec_Busy(struct jedec_flash_dev * flash_dev);

/**
 * @brief Allocate a new device
 */
static struct jedec_flash_dev * PIOS_Flash_Jedec_alloc(void)
{
	struct jedec_flash_dev * flash_dev;

	flash_dev = (struct jedec_flash_dev *)pvPortMalloc(sizeof(*flash_dev));
	if (!flash_dev) return (NULL);

	flash_dev->claimed = false;
	flash_dev->magic = PIOS_JEDEC_DEV_MAGIC;
#if defined(FLASH_FREERTOS)
	flash_dev->transaction_lock = xSemaphoreCreateMutex();
#endif
	return(flash_dev);
}

/**
 * @brief Validate the handle to the spi device
 */
static int32_t PIOS_Flash_Jedec_Validate(struct jedec_flash_dev * flash_dev) {
	if (flash_dev == NULL)
		return -1;
	if (flash_dev->magic != PIOS_JEDEC_DEV_MAGIC)
		return -2;
	if (flash_dev->spi_id == 0)
		return -3;
	return 0;
}

/**
 * @brief Initialize the flash device and enable write access
 */
int32_t PIOS_Flash_Jedec_Init(uintptr_t * flash_id, uint32_t spi_id, uint32_t slave_num, const struct pios_flash_jedec_cfg * cfg)
{
	struct jedec_flash_dev * flash_dev = PIOS_Flash_Jedec_alloc();
	if (flash_dev == NULL)
		return -1;

	flash_dev->spi_id = spi_id;
	flash_dev->slave_num = slave_num;
	flash_dev->cfg = cfg;

	(void) PIOS_Flash_Jedec_ReadID(flash_dev);
	if ((flash_dev->manufacturer != flash_dev->cfg->expect_manufacturer) ||
			(flash_dev->memorytype != flash_dev->cfg->expect_memorytype) ||
			(flash_dev->capacity != flash_dev->cfg->expect_capacity)) {
		/* Mismatched device has been discovered */
		return -1;
	}
	/* Give back a handle to this flash device */
	*flash_id = (uintptr_t) flash_dev;

	return 0;
}


/**
 * @brief Claim the SPI bus for flash use and assert CS pin
 * @return 0 for sucess, -1 for failure to get semaphore
 */
static int32_t PIOS_Flash_Jedec_ClaimBus(struct jedec_flash_dev * flash_dev)
{
	if (PIOS_SPI_ClaimBus(flash_dev->spi_id) < 0)
		return -1;

	PIOS_SPI_RC_PinSet(flash_dev->spi_id, flash_dev->slave_num, 0);
	flash_dev->claimed = true;

	return 0;
}

/**
 * @brief Release the SPI bus sempahore and ensure flash chip not using bus
 */
static int32_t PIOS_Flash_Jedec_ReleaseBus(struct jedec_flash_dev * flash_dev)
{
	PIOS_SPI_RC_PinSet(flash_dev->spi_id, flash_dev->slave_num, 1);
	PIOS_SPI_ReleaseBus(flash_dev->spi_id);
	flash_dev->claimed = false;
	return 0;
}

/**
 * @brief Returns if the flash chip is busy
 * @returns -1 for failure, 0 for not busy, 1 for busy
 */
static int32_t PIOS_Flash_Jedec_Busy(struct jedec_flash_dev * flash_dev)
{
	int32_t status = PIOS_Flash_Jedec_ReadStatus(flash_dev);
	if (status < 0)
		return -1;
	return status & JEDEC_STATUS_BUSY;
}

/**
 * @brief Read the status register from flash chip and return it
 */
static int32_t PIOS_Flash_Jedec_ReadStatus(struct jedec_flash_dev * flash_dev)
{
	uint8_t out[] = {JEDEC_READ_STATUS, 0x00};
	uint8_t in[] = {0,0};
	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) < 0)
		return -1;

	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,out,in,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);
	return in[1];
}

/**
 * @brief Read the status register from flash chip and return it
 */
static int32_t PIOS_Flash_Jedec_ReadID(struct jedec_flash_dev * flash_dev)
{
	uint8_t out[] = {JEDEC_DEVICE_ID};
	uint8_t in[4];
	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) < 0)
		return -1;

	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}

	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,NULL,in,sizeof(in),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}
	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	flash_dev->manufacturer = in[0];        // 0x1f - Manufacturer ID Atmel
	flash_dev->memorytype   = in[1] & 0xf0; // 0x26 00100110b (0010-dataflash)
	flash_dev->capacity     = in[1] & 0x0f; // 0x26 00100110b (0110-16mB)
	return flash_dev->manufacturer;

}

/**********************************
 *
 * Provide a PIOS flash driver API
 *
 *********************************/
#include "pios_flash_at45.h"

#if FLASH_USE_FREERTOS_LOCKS

/**
 * @brief Grab the semaphore to perform a transaction
 * @return 0 for success, -1 for timeout
 */
static int32_t PIOS_Flash_Jedec_StartTransaction(uintptr_t flash_id)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

#if defined(PIOS_INCLUDE_FREERTOS)
	if (xSemaphoreTake(flash_dev->transaction_lock, portMAX_DELAY) != pdTRUE)
		return -2;
#endif

	return 0;
}

/**
 * @brief Release the semaphore to perform a transaction
 * @return 0 for success, -1 for timeout
 */
static int32_t PIOS_Flash_Jedec_EndTransaction(uintptr_t flash_id)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

#if defined(PIOS_INCLUDE_FREERTOS)
	if (xSemaphoreGive(flash_dev->transaction_lock) != pdTRUE)
		return -2;
#endif

	return 0;
}

#else  /* FLASH_USE_FREERTOS_LOCKS */

static int32_t PIOS_Flash_Jedec_StartTransaction(uintptr_t flash_id)
{
	return 0;
}

static int32_t PIOS_Flash_Jedec_EndTransaction(uintptr_t flash_id)
{
	return 0;
}

#endif	/* FLASH_USE_FREERTOS_LOCKS */

/**
 * @brief Erase a sector on the flash chip
 * @param[in] add Address of flash to erase
 * @returns 0 if successful
 * @retval -1 if unable to claim bus
 * @retval
 */
static int32_t PIOS_Flash_Jedec_EraseSector(uintptr_t flash_id, uint32_t addr)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;
	// Erase 1 page of 512 bytes
	uint8_t out[] = {JEDEC_PAGE_ERASE, (uint8_t)(addr >> 6) , (uint8_t)(addr << 2) , 0x00 };

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) != 0)
		return -1;

	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	// Keep polling when bus is busy too
	while(!PIOS_Flash_Jedec_Busy(flash_dev)) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
	}

	return 0;
}

/**
 * @brief Execute the whole chip
 * @returns 0 if successful, -1 if unable to claim bus, -2 if unable to transfer block
 */
static int32_t PIOS_Flash_Jedec_EraseChip(uintptr_t flash_id)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;

		if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
			return -1;

		uint8_t out[] = {AT45DB161D_CHIP_ERASE_0, AT45DB161D_CHIP_ERASE_1, AT45DB161D_CHIP_ERASE_2, AT45DB161D_CHIP_ERASE_3 };

		if (PIOS_Flash_Jedec_ClaimBus(flash_dev) != 0)
			return -1;

		if (PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
			PIOS_Flash_Jedec_ReleaseBus(flash_dev);
			return -2;
		}

		PIOS_Flash_Jedec_ReleaseBus(flash_dev);

		// Keep polling when bus is busy too
		int i = 0;
		while(!PIOS_Flash_Jedec_Busy(flash_dev)) {
	#if defined(FLASH_FREERTOS)
			vTaskDelay(1);
			if ((i++) % 100 == 0)
	#else
			if ((i++) % 10000 == 0)
	#endif
			PIOS_LED_Toggle(PIOS_LED_ALARM);
		}

		return 0;
}


/**
 * @brief Write one page of data (up to 512 bytes) aligned to a page start
 * @param[in] addr Address in flash to write to
 * @param[in] data Pointer to data to write to flash
 * @param[in] len Length of data to write (max 512 bytes)
 * @return Zero if success or error code
 * @retval -1 Unable to claim SPI bus
 * @retval -2 Size exceeds 512 bytes
 * @retval -3 Length to write would wrap around page boundary
 */
int32_t PIOS_Flash_Jedec_WriteData(uintptr_t flash_id, uint16_t addr, uint8_t * data, uint16_t len)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;
	uint32_t offset = 0;

	if(PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;
	/* Can only write one page at a time */
	if (len > 0x200)
		return -2;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) != 0)
		return -1;

	uint8_t out[] = {0x82, (uint8_t)(addr >> 6) , (uint8_t)((addr << 2) | (offset >> 8)), (uint8_t)(offset)};

	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -1;
	}

	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,data,NULL,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -1;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

// Keep polling when bus is busy too
	while(!PIOS_Flash_Jedec_Busy(flash_dev)) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
	}
	return 0;
}

/**
 * @brief Read data from a location in flash memory
 * @param[in] addr Address in flash to write to
 * @param[in] data Pointer to data to write from flash
 * @param[in] len Length of data to write (max 256 bytes)
 * @return Zero if success or error code
 * @retval -1 Unable to claim SPI bus
 */
static int32_t PIOS_Flash_Jedec_ReadData(uintptr_t flash_id, uint32_t addr, uint16_t offset, uint8_t * data, uint16_t len)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) == -1)
		return -1;

	uint8_t out[] = {JEDEC_PAGE_READ, (uint8_t)(addr >> 6) , (uint8_t)((addr << 2) | (offset >> 8)), (uint8_t)(offset), 0x00, 0x00, 0x00, 0x00};
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}
    // Read page
	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,NULL,data,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -3;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	return 0;
}

/**
 * @brief Read data from Page a location in flash memory
 * @param[in] page Page 0-4095 in flash to read
 * @param[in] addr Address in page to read
 * @param[in] data Pointer to data to read from flash
 * @param[in] len Length of data to read (max 512 bytes)
 * @return Zero if success or error code
 * @retval -1 Unable to claim SPI bus
 */
int32_t PIOS_Flash_Jedec_ReadPage(uintptr_t flash_id, uint32_t addr, uint16_t offset, uint8_t * data, uint16_t len)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) == -1)
		return -1;

	uint8_t out[] = {JEDEC_PAGE_READ, (uint8_t)(addr >> 6) , (uint8_t)((addr << 2) | (offset >> 8)), (uint8_t)(offset), 0x00, 0x00, 0x00, 0x00};
	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}
// Read page
	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,NULL,data,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -3;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	return 0;
}

//
int32_t PIOS_Flash_Jedec_PageToBuffer(uintptr_t flash_id, uint16_t addr)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;
	uint8_t out[4];

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) == -1)
		return -1;

	out[0] = JEDEC_PAGE_TO_BUFFER_1;
	out[1] = (unsigned char)(addr >> 6);
	out[2] = (unsigned char)(addr << 2);
	out[3] = 0x00;

	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}
	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	// Keep polling when bus is busy too
	while(!PIOS_Flash_Jedec_Busy(flash_dev)) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
	}

	return 0;
}

int32_t PIOS_Flash_Jedec_BufferToPage(uintptr_t flash_id, uint16_t addr)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;
	uint8_t out[4];

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) == -1)
		return -1;

	out[0] = JEDEC_BUFFER1_MM_PROGRAM;
	out[1] = (unsigned char)(addr >> 6);
	out[2] = (unsigned char)(addr << 2);
	out[3] = 0x00;

	if(PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}
	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	// Keep polling when bus is busy too
	while(!PIOS_Flash_Jedec_Busy(flash_dev)) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
	}
	return 0;
}

int32_t PIOS_Flash_Jedec_WriteBuffer (uintptr_t flash_id, uint16_t addr,  uint8_t * data, uint16_t len)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;
	uint8_t out[4];

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) == -1)
		return -1;

	out[0] = JEDEC_BUFFER1_WRITE;
	out[1] = 0x00;
	out[2] = (unsigned char)(addr >> 8);
	out[3] = (unsigned char)(addr );


	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}

	/* Clock out data to flash */
	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,data,NULL,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -3;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);

	// Keep polling when bus is busy too
	while(!PIOS_Flash_Jedec_Busy(flash_dev)) {
#if defined(FLASH_FREERTOS)
		vTaskDelay(1);
#endif
	}
	return 0;
}

int32_t PIOS_Flash_Jedec_ReadBuffer (uintptr_t * flash_id, unsigned char BufferNum, uint16_t IntBufferAdr,  uint8_t * data, uint16_t len)
{
	struct jedec_flash_dev * flash_dev = (struct jedec_flash_dev *)flash_id;
	uint8_t out[5];

	if (PIOS_Flash_Jedec_Validate(flash_dev) != 0)
		return -1;

	if (PIOS_Flash_Jedec_ClaimBus(flash_dev) == -1)
		return -1;

  if (BufferNum==1)
	  out[0] = JEDEC_BUFFER1_READ;
  else
	  out[0] = JEDEC_BUFFER2_READ;

  	  out[1] = 0x00;
	  out[2] = (unsigned char)(IntBufferAdr >> 8);
	  out[3] = (unsigned char)(IntBufferAdr );
	  out[4] = 0x00;

	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -2;
	}

	/* Clock out data to flash */
	if (PIOS_SPI_TransferBlock(flash_dev->spi_id,NULL,data,len,NULL) < 0) {
		PIOS_Flash_Jedec_ReleaseBus(flash_dev);
		return -1;
	}

	PIOS_Flash_Jedec_ReleaseBus(flash_dev);
	return 0;
}

/* Provide a flash driver to external drivers */
const struct pios_flash_driver pios_jedec_flash_driver = {
	.start_transaction = PIOS_Flash_Jedec_StartTransaction,
	.end_transaction   = PIOS_Flash_Jedec_EndTransaction,
	.erase_chip        = PIOS_Flash_Jedec_EraseChip,
	.erase_sector      = PIOS_Flash_Jedec_EraseSector,
	.write_data        = PIOS_Flash_Jedec_WriteData,
	.write_buffer      = PIOS_Flash_Jedec_WriteBuffer,
	.buffer_to_page    = PIOS_Flash_Jedec_BufferToPage,
	.page_to_buffer    = PIOS_Flash_Jedec_PageToBuffer,
	.read_data         = PIOS_Flash_Jedec_ReadData,
};

