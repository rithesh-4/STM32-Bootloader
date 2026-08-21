/*
 * delta_update.c
 *
 *  Created on: 23-Jul-2026
 *      Author: ramri
 */

#include "delta_update.h"
#include "detools.h"
#include "flash_if.h"
#include "memory_map.h"
#include "metadata.h"

typedef struct{

    uint32_t read_address;
    uint32_t write_address;
    uint32_t read_offset;
    uint32_t write_offset;

}delta_update_context_t;

static struct detools_apply_patch_t patch;

static delta_update_context_t context;

static int ReadCallback(void *arg_p, uint8_t *buf_p, size_t size){

	delta_update_context_t *ctx = (delta_update_context_t *)arg_p;

}

bool DeltaUpdate_Init(uint32_t patch_size)
{

}

bool DeltaUpdate_Process(const uint8_t *patch_data,
                         uint32_t patch_length)
{

}

bool DeltaUpdate_Finalize(void)
{

}

