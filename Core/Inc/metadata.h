/*
 * metadata.h
 *
 *  Created on: 21-Jun-2026
 *      Author: ramri
 */

// metadata.h

#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include <stdbool.h>

#define METADATA_ADDRESS   0x08004000U
#define FW_VALID_FLAG      0xA5A5A5A5U

typedef struct
{
    uint32_t valid_flag;
    uint32_t firmware_size;
}firmware_metadata_t;

bool Metadata_IsValid(void);

#endif
