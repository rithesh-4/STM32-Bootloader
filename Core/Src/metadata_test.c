/**
 ******************************************************************************
 * @file    metadata_test.c
 * @brief   Test utility for manually provisioning firmware metadata.
 *
 * @details
 * This module is intended for development and validation only.
 * In the production bootloader, firmware metadata is written
 * automatically after a successful UART firmware update.
 *
 *  Created on: 12-Jul-2026
 *  Author: ramri
 ******************************************************************************
 */

#include "metadata_test.h"
#include "metadata.h"
#include "memory_map.h"
#include "main.h"

	void Metadata_WriteTestData(void)
	{
	    HAL_FLASH_Unlock();

	    FLASH_EraseInitTypeDef eraseInit;
	    uint32_t sectorError;

	    eraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
	    eraseInit.Sector       = FLASH_SECTOR_1;
	    eraseInit.NbSectors    = 1;
	    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

	    firmware_metadata_t metadata =
	    {
	        .valid_flag        = FW_VALID_FLAG,
	        .firmware_size     = 0x1000U,
	        .app_start_address = APP_START_ADDRESS,
	        .firmware_version  = 1U,
	        .crc32             = 0U      // Placeholder for now
	    };

	    const uint32_t *data = (const uint32_t *)&metadata;

	    for (uint32_t i = 0; i < (sizeof(firmware_metadata_t) / sizeof(uint32_t)); i++)
	    {
	        HAL_FLASH_Program(
	            FLASH_TYPEPROGRAM_WORD,
	            METADATA_ADDRESS + (i * 4U),
	            data[i]
	        );
	    }

	    HAL_FLASH_Lock();
	}
