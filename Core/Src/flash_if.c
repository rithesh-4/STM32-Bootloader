/*
 * flash_if.c
 *
 *  Created on: 07-Jul-2026
 *      Author: ramri
 */

#include "flash_if.h"
#include "stm32f4xx_hal.h"

bool Flash_EraseApplication(void)
{
    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = FLASH_SECTOR_2;
    eraseInit.NbSectors = 10; /* Sectors 2-11 (0x08008000 - 0x080FFFFF) */

    uint32_t sectorError = 0;
    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return false;
    }

    HAL_FLASH_Lock();
    return true;
}

bool Flash_Write(uint32_t address,
                 const uint8_t *data,
                 uint32_t length)
{
    if (address < APP_START_ADDRESS || address > FLASH_END_ADDRESS)
    {
        return false;
    }

    if ((address + length) > (FLASH_END_ADDRESS + 1U))
    {
        return false;
    }

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    for (uint32_t i = 0; i < length; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}