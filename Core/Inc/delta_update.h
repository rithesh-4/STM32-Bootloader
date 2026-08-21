/*
 * delta_update.h
 *
 *  Created on: 23-Jul-2026
 *      Author: ramri
 */

#ifndef DELTA_UPDATE_H
#define DELTA_UPDATE_H

#include <stdbool.h>
#include <stdint.h>

bool DeltaUpdate_Init(uint32_t patch_size);

bool DeltaUpdate_Process(const uint8_t *patch_data,
                         uint32_t patch_length);

bool DeltaUpdate_Finalize(void);

#endif
