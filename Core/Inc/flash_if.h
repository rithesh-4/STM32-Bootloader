/*
 * flash_if.h
 *
 *  Created on: 07-Jul-2026
 *      Author: ramri
 */

#ifndef FLASH_IF_H
#define FLASH_IF_H

#include <stdint.h>
#include <stdbool.h>
#include "memory_map.h"

bool Flash_EraseApplication(void);

bool Flash_Write(uint32_t address,
                 const uint8_t *data,
                 uint32_t length);

#endif
