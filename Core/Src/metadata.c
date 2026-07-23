/*
 * metadata.c
 *
 *  Created on: 11-Jul-2026
 *      Author: ramri
 */

#include "metadata.h"
/**
 * @brief Returns a pointer to the metadata stored in Flash.
 */
const firmware_metadata_t* Metadata_Get(void)
{
    return (const firmware_metadata_t*)METADATA_ADDRESS;
}

/**
 * @brief Validates firmware metadata.
 */
bool Metadata_IsValid(void)
{
    const firmware_metadata_t *metadata = Metadata_Get();

    /* Check valid flag */
    if (metadata->valid_flag != FW_VALID_FLAG)
    {
        return false;
    }

    /* Check application start address */
    if (metadata->app_start_address != APP_START_ADDRESS)
    {
        return false;
    }

    /* Check firmware size */
    if ((metadata->firmware_size == 0U) || (metadata->firmware_size > APP_MAX_SIZE))
    {
        return false;
    }

    /* CRC verification will be added in a later step */


    return true;
}
