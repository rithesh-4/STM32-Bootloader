/*
 * metadata.c
 *
 *  Created on: 21-Jun-2026
 *      Author: ramri
 */

// metadata.c

#include "metadata.h"

static const firmware_metadata_t *metadata = (firmware_metadata_t *)METADATA_ADDRESS;

bool Metadata_IsValid(void)
{
    if(metadata->valid_flag != FW_VALID_FLAG)
    {
        return false;
    }

    if(metadata->firmware_size == 0)
    {
        return false;
    }

    return true;
}

