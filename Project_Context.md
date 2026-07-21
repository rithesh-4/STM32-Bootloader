\# STM32F407 Production Bootloader

\## Project Context \& Design Specification



Version: 2.0

Target MCU: STM32F407VGTx

IDE: STM32CubeIDE

Language: C

Framework: STM32 HAL



\---



\# 1. Project Overview



This repository contains a custom production-style bootloader for the STM32F407VGTx microcontroller.



The objective is NOT to create the smallest bootloader possible.



The objective is to learn and implement how production embedded products perform firmware updates while maintaining good software engineering practices.



The project is intentionally split into six incremental versions.



Each version introduces a single major feature while preserving backwards compatibility.



The final project should resemble the architecture of a commercial embedded bootloader.



\---



\# 2. Learning Objectives



The project exists to master:



\- Embedded C

\- ARM Cortex-M4 architecture

\- STM32 Flash programming

\- Boot process

\- Interrupt handling

\- Vector table relocation

\- Memory management

\- Firmware updates

\- UART protocols

\- CRC verification

\- Secure Boot

\- Production firmware architecture



The code should always prioritize readability and maintainability over clever optimizations.



\---



\# 3. Hardware



MCU



STM32F407VGTx



Flash



1 MB



RAM



128 KB



Core



ARM Cortex-M4



Boot Interface



Custom Bootloader



System Bootloader is NOT used.



\---



\# 4. Development Environment



IDE



STM32CubeIDE



HAL



STM32 HAL



Compiler



arm-none-eabi-gcc



Debugger



ST-Link



Version Control



Git



Repository



STM32-Bootloader



\---



\# 5. Bootloader Philosophy



This project intentionally avoids writing all logic inside main.c.



Every feature belongs inside its own module.



Modules should have one responsibility.



Large functions should be avoided.



The bootloader should be maintainable.



The architecture should scale naturally as additional features are added.



\---



\# 6. Development Roadmap



Version 1

\-----------



Completed



Features



\- Basic bootloader

\- Jump to application

\- Vector table relocation

\- MSP initialization



\--------------------------------------------------



Version 2



Completed



Features



\- Flash memory partitioning

\- Metadata region

\- Metadata validation

\- Application validation

\- Flash abstraction

\- Clean application handoff



\--------------------------------------------------



Version 3



Current Next Milestone



Features



\- UART firmware update

\- Packet parser

\- Flash erase/write

\- CRC verification

\- Metadata writing

\- Firmware installation



\--------------------------------------------------



Version 4



Planned



Features



\- Version management

\- Boot flags

\- Recovery improvements

\- Watchdog support

\- Robust error handling



\--------------------------------------------------



Version 5



Planned



Features



\- Secure Boot

\- Firmware authentication

\- Anti rollback

\- Image verification

\- Cryptographic validation



\--------------------------------------------------



Version 6



Planned



Features



\- Production cleanup

\- Documentation

\- Optimization

\- Performance improvements

\- Final testing



\---



\# 7. Current Project Status



Completed



✓ Bootloader startup



✓ Memory map



✓ Metadata validation



✓ Application validation



✓ Boot decision



✓ Jump sequence



Remaining



\- Flash interface implementation

\- UART update

\- CRC

\- Metadata write

\- Production robustness

\- Security



Overall Completion



Approximately 30%



\---



\# 8. Flash Memory Layout



Bootloader



Start



0x08000000



Size



16 KB



\--------------------------------------------------



Metadata



Start



0x08004000



Size



16 KB



\--------------------------------------------------



Application



Start



0x08008000



End



0x080FFFFF



Maximum Size



APP\_END\_ADDRESS - APP\_START\_ADDRESS + 1



\---



\# 9. Current Modules



memory\_map



Responsibilities



\- Flash addresses

\- RAM addresses

\- Shared constants



\--------------------------------------------------



metadata



Responsibilities



\- Metadata structure

\- Metadata validation

\- Metadata access



Validation currently checks



\- Valid flag

\- Application address

\- Firmware size

\- Initial stack pointer



CRC verification will be implemented later.



\--------------------------------------------------



flash\_if



Responsibilities



Application flash erase



Application flash write



Application flash verify



Currently only interfaces exist.



Implementation belongs to Version 3.



\--------------------------------------------------



metadata\_test



Development only.



Purpose



Write test metadata into flash during development.



This module must never become production code.



Eventually removed.



\--------------------------------------------------



main



Responsibilities



Boot decision



Hardware initialization



Application handoff



No business logic should accumulate here.



\---



\# 10. Current Boot Sequence



Power On



↓



Reset



↓



HAL\_Init()



↓



Clock Configuration



↓



GPIO Initialization



↓



Boot Delay



↓



Metadata Validation



↓



Application Validation



↓



JumpToApplication()



↓



Application Executes



If validation fails



↓



Remain inside bootloader



\---



\# 11. Application Jump Sequence



Current implementation performs



Read MSP



Read Reset Handler



Disable SysTick



Disable Interrupts



Disable NVIC



Clear Pending Interrupts



HAL\_DeInit()



HAL\_RCC\_DeInit()



Relocate VTOR



Set MSP



Execute DSB



Execute ISB



Jump to Application



This sequence should not change unless required.



\---



\# 12. Metadata Structure



Current Fields



valid\_flag



firmware\_size



application\_start\_address



firmware\_version



crc32



Future versions may include



metadata\_version



boot\_counter



rollback\_counter



security\_flags



image\_hash



timestamp



\---



\# 13. Coding Standards



Use STM32 HAL.



Never modify HAL drivers.



Never modify startup assembly unless required.



Never modify linker scripts without approval.



Avoid global variables.



Avoid magic numbers.



Keep flash addresses inside memory\_map.h.



Always validate external input.



Always check HAL return values.



Keep functions short.



Keep modules cohesive.



Use descriptive names.



Document non-obvious logic.



\---



\# 14. Design Decisions



Metadata is stored separately from the application.



Memory layout is fixed.



Flash addresses are centralized.



Validation is independent of JumpToApplication().



Application jump follows ARM Cortex-M recommendations.



Development-only code is isolated.



Future security features must not require redesign of existing modules.



\---



\# 15. Current Known Limitations



These are intentional.



No UART updates.



No flash programming.



No CRC verification.



No rollback.



No watchdog.



No secure boot.



No firmware authentication.



These belong to later roadmap versions.



\---



\# 16. Future UART Update Flow



Bootloader



↓



Receive Command



↓



Receive Packet



↓



Validate Packet CRC



↓



Erase Application



↓



Write Flash



↓



Verify CRC



↓



Write Metadata



↓



Reset MCU



↓



Validate Metadata



↓



Jump to Application



\---



\# 17. Security Roadmap



Version 5 introduces



Firmware authentication



Anti rollback



Secure Boot



Image verification



Integrity validation



Cryptographic signatures



The current project is NOT intended to be secure.



\---



\# 18. Git Workflow



Commit after every logical feature.



Examples



Implement flash\_if



Implement UART parser



Implement CRC



Implement Metadata write



Do not create large commits.



Tag every completed version.



Never commit temporary files.



\---



\# 19. Repository Rules



Never modify



Drivers/



CMSIS



Generated CubeMX code



Startup assembly



Linker scripts



unless explicitly instructed.



\---



\# 20. AI Agent Instructions



Read this document before making changes.



Read the repository before proposing solutions.



Explain architectural changes before implementing them.



Wait for approval before editing files.



Do not modify unrelated files.



Do not refactor entire modules unnecessarily.



Keep changes minimal.



Never commit.



Never push.



Never create branches.



Do not implement future roadmap items unless explicitly requested.



Always explain WHY.



Always preserve modularity.



Always preserve memory layout.



Always preserve current boot sequence.



Always preserve current module boundaries.



If unsure



Ask.



Never guess.



\---



\# 21. Long-Term Goal



The final deliverable should resemble a commercial STM32 bootloader.



It should demonstrate



\- Embedded systems knowledge

\- Production software architecture

\- Good Git practices

\- Modular firmware design

\- Robust update mechanisms

\- Security awareness

\- Maintainable code

\- Professional documentation



This repository is intended to serve both as a learning platform and as a portfolio-quality embedded firmware project.

