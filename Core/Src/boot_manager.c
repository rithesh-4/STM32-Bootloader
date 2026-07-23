/*
 * boot_manager.c
 *
 *  Created on: 21-Jul-2026
 *      Author: ramri
 */

#include "boot_manager.h"
#include "main.h"
#include "memory_map.h"
#include "metadata.h"
#include "image_manager.h"

/*Static Function Prototypes*/

static void JumpToApplication(void);

static void BootManager_Process(void);



static void BootManager_Process(void){

	  /* ==== Bootloader startup: blink LEDs and check PA0 button === */
	  for(int i = 0; i < 20; i++)
	  {
		  volatile GPIO_PinState StayInBoot = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

		  if (StayInBoot == GPIO_PIN_SET)
		  {
			  /* Stay in bootloader mode, blink PD14 */
			  while(1)
			  {
				  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
				  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				  HAL_Delay(50);
			  }
		  }

		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		  HAL_Delay(100);
	  }

	  if (Image_IsBootable())
		  {
			  JumpToApplication();
		  }

	  while(1){


	  }

}


static void JumpToApplication(void)
	{
	    uint32_t appStack = *(volatile uint32_t *)APP_START_ADDRESS;
	    uint32_t appEntry = *(volatile uint32_t *)(APP_START_ADDRESS + 4U);

	    void (*app_reset_handler)(void);
	    app_reset_handler = (void (*)(void))appEntry;

	    /* Disable SysTick */
	    SysTick->CTRL = 0;
	    SysTick->LOAD = 0;
	    SysTick->VAL  = 0;

	    /* Disable all interrupts */
	    __disable_irq();

	    /* Disable and clear all NVIC interrupts */
	    for (uint32_t i = 0; i < NVIC_REGISTER_COUNT; i++)
	    {
	        NVIC->ICER[i] = 0xFFFFFFFFU;
	        NVIC->ICPR[i] = 0xFFFFFFFFU;
	    }

	    /* Reset peripherals */
	    HAL_DeInit();

	    /* Reset clock configuration */
	    HAL_RCC_DeInit();

	    /* Switch to application's vector table */
	    SCB->VTOR = APP_START_ADDRESS;

	    /* Ensure VTOR update is visible before continuing */
	    __DSB();
	    __ISB();

	    /* Set application's Main Stack Pointer */
	    __set_MSP(appStack);

	    /* Jump to application's Reset Handler (never returns) */
	    app_reset_handler();

	    /* Should never reach here */
	    while (1)
	    {
	    }
	}

void BootManager_Run(void){

	BootManager_Process();

}
