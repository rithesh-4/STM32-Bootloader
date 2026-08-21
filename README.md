# STM32 Bootloader

Modular bootloader for the STM32F407VGT6 with CRC-verified firmware validation and delta update support via [detools](https://github.com/detools/detools) + [heatshrink](https://github.com/atomicobject/heatshrink) compression.

## Overview

This project implements a production-style bootloader for ARM Cortex-M4 microcontrollers. It handles firmware validation, flash management, and application handoff with a clean module separation.

**Key features:**

- Modular architecture (boot manager, image validator, metadata manager, flash driver)
- CRC32 firmware integrity verification
- Delta/differential firmware updates using detools with heatshrink compression
- Fail-safe: stays in bootloader if no valid application is found
- Button-triggered bootloader entry for manual firmware recovery

## Memory Layout

```
Address Range              Region            Size
─────────────────────────────────────────────────
0x08000000 - 0x08003FFF   Bootloader        16 KB
0x08004000 - 0x08007FFF   Metadata          16 KB
0x08008000 - 0x080FFFFF   Application       992 KB
```

STM32F407VGT6 sectors 0-1 reserved for bootloader and metadata. Sectors 2-11 used for application firmware.

## Project Structure

```
Core/
  Src/
    main.c               Entry point, peripheral init
    boot_manager.c       Boot sequence, button check, jump to app
    image_manager.c      Application image validation (SP, reset handler)
    metadata.c           Metadata storage and CRC32 verification
    flash_if.c           Flash sector erase and byte write
    delta_update.c       Delta update processing via detools
  Inc/
    memory_map.h         Address and size definitions
    metadata.h           Firmware metadata structure
    flash_if.h           Flash interface API
    delta_update.h       Delta update API
Middlewares/
  detools/               Delta update library
    heatshrink/          Compression decoder
NodeMCU_Files/
  UART_Firmware/          Arduino sketch for NodeMCU UART bridge (firmware upload)
Drivers/
  STM32F4xx_HAL_Driver/   HAL peripheral drivers
  CMSIS/                  ARM CMSIS headers
STM32F407VGTX_FLASH.ld    Linker script (1MB flash)
```

## Building

### Requirements

- STM32CubeIDE 2.x (includes ARM GCC toolchain)

### Using STM32CubeIDE

Open the project in STM32CubeIDE and build the `Debug` configuration.

### Headless build

```bash
STM32CubeIDE.exe -nosplash \
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data <workspace_path> \
  -import <project_path> \
  -cleanBuild "Bootloader/Debug"
```

### Output

The build produces `Debug/Bootloader.elf` along with `.map` and `.list` files.

## Boot Sequence

1. **Reset** - `HAL_Init()`, clock config, GPIO/UART init
2. **Button check** - If PA0 is pressed, stay in bootloader mode (LED blink)
3. **Image validation** - `Image_IsBootable()` checks:
   - Firmware metadata valid flag, size, and CRC32
   - Stack pointer in RAM range (`0x20000000 - 0x20020000`)
   - Reset handler in application flash region
4. **Jump to application** - Disable interrupts, clear NVIC, set VTOR, set MSP, and jump to reset handler

## Delta Updates

The bootloader supports differential firmware updates. A delta patch (old firmware -> new firmware) is processed using detools with heatshrink decompression, reducing patch size by ~90% compared to full firmware images.

```c
DeltaUpdate_Init(patch_size);
DeltaUpdate_Process(patch_chunk, chunk_length);  // repeat until complete
DeltaUpdate_Finalize();  // applies patch, writes to flash
```

The patch is applied in-place: the old firmware is read from flash, the new firmware is computed in a RAM buffer, and then the application region is erased and rewritten.

## Metadata Structure

```c
typedef struct {
    uint32_t valid_flag;        // 0xA5A5A5A5 when firmware is valid
    uint32_t firmware_size;     // Size of application in bytes
    uint32_t app_start_address; // 0x08008000
    uint32_t firmware_version;  // Version number
    uint32_t crc32;             // CRC32 of application image
} firmware_metadata_t;
```

## Flash Sectors

| Sector | Address Range | Size | Usage |
|--------|---------------|------|-------|
| 0 | `0x08000000 - 0x08003FFF` | 16 KB | Bootloader |
| 1 | `0x08004000 - 0x08007FFF` | 16 KB | Metadata |
| 2-3 | `0x08008000 - 0x0800FFFF` | 32 KB | Application |
| 4 | `0x08010000 - 0x0801FFFF` | 64 KB | Application |
| 5-11 | `0x08020000 - 0x080FFFFF` | 896 KB | Application |

## References

- [STM32F407 Reference Manual (RM0090)](https://www.st.com/resource/en/reference_manual/rm0090.pdf)
- [STM32F407VGT6 Datasheet](https://www.st.com/resource/en/datasheet/stm32f407vg.pdf)
- [detools](https://github.com/detools/detools)
- [heatshrink](https://github.com/atomicobject/heatshrink)

## License

MIT
