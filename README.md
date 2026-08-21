# STM32 Modular Bootloader with Delta Firmware Updates

A production-ready bootloader for STM32F407VGT6 featuring modular architecture, CRC-verified firmware validation, flash management, and delta update framework using detools/heatshrink compression.

## 🎯 Features

- **Modular Architecture**: Clean separation into Boot Manager, Image Manager, Metadata Manager, Flash Interface
- **Memory Layout**: Custom linker script with dedicated regions (Bootloader 16KB | Metadata 16KB | Application 992KB)
- **Robust Validation**: Stack pointer range check, reset handler range check, magic flag, size check, **CRC32 verification**
- **Flash Management**: Sector-based erase (Sectors 2-11), byte-level write via HAL Flash
- **Delta Updates**: Framework integrated with `detools` + `heatshrink` for ~90% payload reduction
- **Fail-Safe**: Button-triggered bootloader stay mode, automatic fallback on invalid image

## 📁 Memory Map (STM32F407VGT6 - 1MB Flash)

```
+------------------------+ 0x0800 0000 (Sector 0, 16KB)
|   BOOTLOADER           |
|   (16 KB)              |
+------------------------+ 0x0800 4000 (Sector 1, 16KB)
|   METADATA             |
|   (16 KB)              |
+------------------------+ 0x0800 8000 (Sector 2, 16KB)
|                        |
|   APPLICATION          |
|   (992 KB)             |
|   Sectors 2-11         |
|                        |
+------------------------+ 0x0810 0000
```

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      main.c                                 │
│  HAL_Init → SystemClock_Config → MX_GPIO_Init → MX_USART2   │
│                          ↓                                  │
│                 BootManager_Run()                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    boot_manager.c                           │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 1. PA0 Button Check (Stay in Bootloader mode)       │   │
│  │ 2. Image_IsBootable() → JumpToApplication()         │   │
│  │ 3. Infinite loop (Bootloader stay mode)             │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              ▼               ▼               ▼
┌─────────────────────┐ ┌──────────────┐ ┌──────────────┐
│   image_manager.c   │ │ metadata.c   │ │  flash_if.c  │
│  - SP range check   │ │ - Valid flag │ │ - Erase App  │
│  - Reset handler    │ │ - Size check │ │ - Byte Write │
│    range check      │ │ - CRC32      │ │              │
└─────────────────────┘ └──────────────┘ └──────────────┘
```

## 🔧 Modules

| Module | File | Responsibility |
|--------|------|----------------|
| **Boot Manager** | `boot_manager.c/h` | Startup sequence, button check, jump to app |
| **Image Manager** | `image_manager.c/h` | Application image validation (SP, reset handler) |
| **Metadata Manager** | `metadata.c/h` | Firmware metadata storage & CRC32 verification |
| **Flash Interface** | `flash_if.c/h` | Sector erase (Sectors 2-11), byte write |
| **Delta Update** | `delta_update.c/h` | Framework for heatshrink-compressed delta patches |
| **Memory Map** | `memory_map.h` | Address/size definitions for all regions |

## 🚀 Boot Sequence

```
1. Power On / Reset
         │
         ▼
2. HAL_Init(), SystemClock_Config(), Peripheral Init
         │
         ▼
3. BootManager_Run()
         │
         ├─── PA0 Pressed? ──Yes──→ Stay in Bootloader (blink PD14)
         │
         No
         │
         ▼
4. Image_IsBootable()
         │
         ├── Metadata Valid? (flag, size, CRC32)
         │
         ├── SP in RAM range? (0x20000000 - 0x20020000)
         │
         └── Reset Handler in App Flash? (0x08008000 - 0x080FFFFF)
         │
         ▼
5. JumpToApplication()
         │
         ├── Disable SysTick
         ├── Disable ALL interrupts (__disable_irq)
         ├── Clear NVIC (ICER + ICPR)
         ├── HAL_DeInit() + HAL_RCC_DeInit()
         ├── SCB->VTOR = APP_START_ADDRESS
         ├── __DSB() + __ISB()
         ├── __set_MSP(appStack)
         └── Jump to appResetHandler()
```

## ⚙️ Building

### Prerequisites
- STM32CubeIDE 2.x or later
- ARM GCC toolchain (included with CubeIDE)

### Build Commands
```bash
# Using STM32CubeIDE headless build
STM32CubeIDE.exe -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data <workspace_path> -import <project_path> -cleanBuild "Bootloader/Debug"

# Or open in STM32CubeIDE GUI and build
```

### Output
- `Debug/Bootloader.elf` - Debug build with symbols
- `Debug/Bootloader.map` - Linker map file
- `Debug/Bootloader.list` - Assembly listing

## 🔑 Key Technical Details

### Metadata Structure (`metadata.h`)
```c
typedef struct {
    uint32_t valid_flag;        // 0xA5A5A5A5
    uint32_t firmware_size;     // Application size in bytes
    uint32_t app_start_address; // Must be 0x08008000
    uint32_t firmware_version;  // Version number
    uint32_t crc32;             // CRC32 of application image
} firmware_metadata_t;
```

### Flash Sectors (STM32F407VGT6)
| Sector | Address Range | Size | Usage |
|--------|---------------|------|-------|
| 0 | 0x08000000 - 0x08003FFF | 16 KB | Bootloader |
| 1 | 0x08004000 - 0x08007FFF | 16 KB | Metadata |
| 2 | 0x08008000 - 0x0800BFFF | 16 KB | App |
| 3 | 0x0800C000 - 0x0800FFFF | 16 KB | App |
| 4 | 0x08010000 - 0x0801FFFF | 64 KB | App |
| 5-11 | 0x08020000 - 0x080FFFFF | 128 KB each | App |

### Jump to Application (ARM Cortex-M4)
```c
// Read SP and Reset Handler from App vector table
uint32_t appStack = *(uint32_t*)APP_START_ADDRESS;
uint32_t appEntry = *(uint32_t*)(APP_START_ADDRESS + 4U);

// Disable interrupts & peripherals
__disable_irq();
SysTick->CTRL = 0;
for (i = 0; i < 8; i++) { NVIC->ICER[i] = 0xFFFFFFFF; NVIC->ICPR[i] = 0xFFFFFFFF; }
HAL_DeInit(); HAL_RCC_DeInit();

// Switch vector table
SCB->VTOR = APP_START_ADDRESS;
__DSB(); __ISB();

// Set MSP and jump
__set_MSP(appStack);
((void(*)(void))appEntry)();
```

### Delta Update Framework (WIP)
- **detools** + **heatshrink** integrated in `Middlewares/`
- API: `DeltaUpdate_Init()` → `DeltaUpdate_Process()` → `DeltaUpdate_Finalize()`
- Uses streaming patch application with on-the-fly flash writes
- ~90% payload reduction for small changes (validated via detools prototype)

## 📋 Current Status

| Module | Status | Notes |
|--------|--------|-------|
| Boot Manager | ✅ Complete | Jump sequence, button stay mode |
| Image Manager | ✅ Complete | SP/Reset validation |
| Metadata Manager | ✅ Complete | Flag, size, CRC32 |
| Flash Interface | ✅ Complete | Sector erase, byte write |
| Delta Update | ⚠️ Stub | Framework ready, implementation in progress |
| CRC32 | ✅ Complete | Software lookup table (no HAL dependency) |
| Linker Script | ✅ Updated | 1MB FLASH, matches memory_map.h |

## 🐛 Known Warnings (Expected)
- `delta_update.c`: Unused variables/functions — stub implementation
- `metadata_test.c`: Test file, not linked in production

## 📚 References
- [STM32F407 Reference Manual (RM0090)](https://www.st.com/resource/en/reference_manual/rm0090.pdf)
- [STM32F407VGT6 Datasheet](https://www.st.com/resource/en/datasheet/stm32f407vg.pdf)
- [detools - Delta Update Tools](https://github.com/detools/detools)
- [heatshrink - Compression Library](https://github.com/atomicobject/heatshrink)

## 📄 License
MIT License - Feel free to use for learning or commercial projects.

---
*Built with STM32CubeIDE • ARM Cortex-M4 • HAL Driver*