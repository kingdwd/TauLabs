/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS_OBJLIST Object list based flash filesystem (low ram)
 * @{
 *
 * @file       pios_45_flash_jedec.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Virtual Robotix Network Team, http://www.virtualrobotix.com Copyright (C) 2013.
 * @brief      A file system for storing UAVObject in flash chip
 * @see        The GNU Public License (GPL) Version 3
 *
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

#ifndef PIOS_AT45_FLASH_JEDEC_H_
#define PIOS_AT45_FLASH_JEDEC_H_

#include "pios_flash_at45.h"		/* API definition for flash drivers */

extern const struct pios_flash_driver pios_jedec_flash_driver;

#define JEDEC_MANUFACTURER_ATMEL    0X1F  // 0x1f - Manufacturer ID Atmel

struct pios_flash_jedec_cfg {
	uint8_t expect_manufacturer;
	uint8_t expect_memorytype;
	uint8_t expect_capacity;
};

int32_t PIOS_Flash_Jedec_Init(uintptr_t * flash_id, uint32_t spi_id, uint32_t slave_num, const struct pios_flash_jedec_cfg * cfg);

#endif	/* PIOS_AT45_FLASH_JEDEC_H_ */
