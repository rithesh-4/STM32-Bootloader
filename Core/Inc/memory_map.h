/*
 * memory_map.h
 *
 *  Created on: 07-Jul-2026
 *      Author: ramri
 */

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

/* RAM Address */
#define RAM_START_ADDRESS  0x20000000U
#define RAM_SIZE           (128U * 1024U)
#define RAM_END_ADDRESS    (RAM_START_ADDRESS + RAM_SIZE)   // 0x20020000

/*Flash Address*/
#define FLASH_BASE_ADDRESS    0x08000000U
#define FLASH_END_ADDRESS     0x080FFFFFU

/* Bootloader */
#define BOOTLOADER_START_ADDRESS    0x08000000U
#define BOOTLOADER_SIZE             (16U * 1024U)

/* Metadata */
#define METADATA_ADDRESS            0x08004000U
#define METADATA_SIZE               (16U * 1024U)

/* Application */
#define APP_START_ADDRESS           0x08008000U
#define APP_END_ADDRESS             0x080FFFFFU
#define APP_MAX_SIZE                (APP_END_ADDRESS - APP_START_ADDRESS + 1U)

#endif
