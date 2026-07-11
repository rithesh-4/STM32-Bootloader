	/* USER CODE BEGIN Header */
	/**
	  ******************************************************************************
	  * @file           : main.c
	  * @brief          : Main program body
	  ******************************************************************************
	  * @attention
	  *
	  * Copyright (c) 2026 STMicroelectronics.
	  * All rights reserved.
	  *
	  * This software is licensed under terms that can be found in the LICENSE file
	  * in the root directory of this software component.
	  * If no LICENSE file comes with this software, it is provided AS-IS.
	  *
	  ******************************************************************************
	  */
	/* USER CODE END Header */
	/* Includes ------------------------------------------------------------------*/
	#include "main.h"
	#include "metadata.h"
	#include "memory_map.h"

	/* Private includes ----------------------------------------------------------*/
	/* USER CODE BEGIN Includes */

	/* USER CODE END Includes */

	/* Private typedef -----------------------------------------------------------*/
	/* USER CODE BEGIN PTD */

	/* USER CODE END PTD */

	/* Private define ------------------------------------------------------------*/
	/* USER CODE BEGIN PD */

	/* USER CODE END PD */

	/* Private macro -------------------------------------------------------------*/
	/* USER CODE BEGIN PM */

	/* USER CODE END PM */

	/* Private variables ---------------------------------------------------------*/

	/* USER CODE BEGIN PV */

	/* USER CODE END PV */

	/* Private function prototypes -----------------------------------------------*/
	void SystemClock_Config(void);
	static void MX_GPIO_Init(void);
	/* USER CODE BEGIN PFP */

	/* USER CODE END PFP */

	/* Private user code ---------------------------------------------------------*/
	/* USER CODE BEGIN 0 */

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

	void Metadata_WriteTestData(void)
	{
	    HAL_FLASH_Unlock();

	    FLASH_EraseInitTypeDef eraseInit;
	    uint32_t sectorError;

	    eraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
	    eraseInit.Sector       = FLASH_SECTOR_1;
	    eraseInit.NbSectors    = 1;
	    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

	    firmware_metadata_t metadata =
	    {
	        .valid_flag        = FW_VALID_FLAG,
	        .firmware_size     = 0x1000U,
	        .app_start_address = APP_START_ADDRESS,
	        .firmware_version  = 1U,
	        .crc32             = 0U      // Placeholder for now
	    };

	    const uint32_t *data = (const uint32_t *)&metadata;

	    for (uint32_t i = 0; i < (sizeof(firmware_metadata_t) / sizeof(uint32_t)); i++)
	    {
	        HAL_FLASH_Program(
	            FLASH_TYPEPROGRAM_WORD,
	            METADATA_ADDRESS + (i * 4U),
	            data[i]
	        );
	    }

	    HAL_FLASH_Lock();
	}

	/* USER CODE END 0 */

	/**
	  * @brief  The application entry point.
	  * @retval int
	  */
	int main(void)
	{

	  /* USER CODE BEGIN 1 */

	  /* USER CODE END 1 */

	  /* MCU Configuration--------------------------------------------------------*/

	  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	  HAL_Init();

	  /* USER CODE BEGIN Init */

	  /* USER CODE END Init */

	  /* Configure the system clock */
	  SystemClock_Config();

	  /* USER CODE BEGIN SysInit */

	  /* USER CODE END SysInit */

	  /* Initialize all configured peripherals */
	  MX_GPIO_Init();
	  /* USER CODE BEGIN 2 */

	  //Metadata_WriteTestData(); Run this once to write the Test data to the flash, in future updates this is done by the UART.*/



	  for(int i = 0; i < 20; i++)
	  {
		  volatile GPIO_PinState StayInBoot = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

			if (StayInBoot == GPIO_PIN_SET)
			{
				while(1)
				{
				  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);
				  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				  HAL_Delay(50);
				}
			}

		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		  HAL_Delay(100);
	  }

	  if (Metadata_IsValid())
	  {
		  JumpToApplication();
	  }
	  else
	  {
		  while (1)
		  {
			  // Stay in bootloader
		  }
	  }

	  /* USER CODE END 2 */

	  /* Infinite loop */
	  /* USER CODE BEGIN WHILE */
	  while (1)
	  {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	  }
	  /* USER CODE END 3 */
	}

	/**
	  * @brief System Clock Configuration
	  * @retval None
	  */
	void SystemClock_Config(void)
	{
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	  /** Configure the main internal regulator output voltage
	  */
	  __HAL_RCC_PWR_CLK_ENABLE();
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	  /** Initializes the RCC Oscillators according to the specified parameters
	  * in the RCC_OscInitTypeDef structure.
	  */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
		Error_Handler();
	  }

	  /** Initializes the CPU, AHB and APB buses clocks
	  */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	  {
		Error_Handler();
	  }
	}

	/**
	  * @brief GPIO Initialization Function
	  * @param None
	  * @retval None
	  */
	static void MX_GPIO_Init(void)
	{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  /* USER CODE BEGIN MX_GPIO_Init_1 */

	  /* USER CODE END MX_GPIO_Init_1 */

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_14, GPIO_PIN_RESET);

	  /*Configure GPIO pin : PA0 */
	  GPIO_InitStruct.Pin = GPIO_PIN_0;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : PD12 PD14 */
	  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	  /* USER CODE BEGIN MX_GPIO_Init_2 */

	  /* USER CODE END MX_GPIO_Init_2 */
	}

	/* USER CODE BEGIN 4 */

	/* USER CODE END 4 */

	/**
	  * @brief  This function is executed in case of error occurrence.
	  * @retval None
	  */
	void Error_Handler(void)
	{
	  /* USER CODE BEGIN Error_Handler_Debug */
	  /* User can add his own implementation to report the HAL error return state */
	  __disable_irq();
	  while (1)
	  {
	  }
	  /* USER CODE END Error_Handler_Debug */
	}
	#ifdef USE_FULL_ASSERT
	/**
	  * @brief  Reports the name of the source file and the source line number
	  *         where the assert_param error has occurred.
	  * @param  file: pointer to the source file name
	  * @param  line: assert_param error line source number
	  * @retval None
	  */
	void assert_failed(uint8_t *file, uint32_t line)
	{
	  /* USER CODE BEGIN 6 */
	  /* User can add his own implementation to report the file name and line number,
		 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	  /* USER CODE END 6 */
	}
	#endif /* USE_FULL_ASSERT */
