/*
 * metadata.h
 *
 *  Created on: 21-Jun-2026
 *      Author: ramri
 */

#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include <stdbool.h>

#include "memory_map.h"

/* Metadata Constants */
#define FW_VALID_FLAG      0xA5A5A5A5U

/* Firmware Metadata Structure */
typedef struct
{
    uint32_t valid_flag;          // Indicates if firmware is valid
    uint32_t firmware_size;       // Size of application in bytes
    uint32_t app_start_address;   // Start address of the application
    uint32_t firmware_version;    // Firmware version number
    uint32_t crc32;               // CRC32 of the application image

} firmware_metadata_t;

/* Function Prototypes */

/**
 * @brief Returns a pointer to the firmware metadata stored in Flash.
 *
 * @return Pointer to firmware metadata.
 */
const firmware_metadata_t* Metadata_Get(void);

/**
 * @brief Validates the firmware metadata.
 *
 * @return true if metadata is valid.
 * @return false otherwise.
 */
bool Metadata_IsValid(void);

#endif /* METADATA_H */
