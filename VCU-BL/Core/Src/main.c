/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "adc.h"
#include "can.h"
#include "crc.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "iwdg.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "App/fota.h"
#include "App/util.h"
#include "DMA/dma_simcom.h"
#include "Drivers/battery.h"
#include "Drivers/can.h"
#include "Drivers/flasher.h"
#include "Drivers/simcom.h"
#include "Libs/eeprom.h"
#include "Libs/errata.h"

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
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  ERRATA_I2C_ClearBusyFlag();
  printf_init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CRC_Init();
  MX_CAN1_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  MX_UART9_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  GATE_LedWrite(1);
  HAL_Delay(250);
  GATE_LedWrite(0);
  HAL_Delay(500);

  BAT_Init();
  EE_Init();
  CAN_Init();
  SIM_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  HAL_IWDG_Refresh(&hiwdg);
  /* IAP flag has been set, initiate firmware download procedure */
  if (*(uint32_t *)IAP_FLAG_ADDR == IFLAG_SRAM) {
    printf("IAP set, do FOTA.\n");
    /* Everything went well */
    if (FOTA_Upgrade(IAP_IO_Type())) {
      /* Reset IAP flag */
      *(uint32_t *)IAP_FLAG_ADDR = IFLAG_RESET;
      /* Take branching decision on next reboot */
      FOTA_Reboot();
    }
    /* Reset IAP flag */
    *(uint32_t *)IAP_FLAG_ADDR = IFLAG_RESET;
    /* FOTA failed */
    HAL_NVIC_SystemReset();
  }
  /* Jump to application if it exist and FOTA finished */
  else if (FOTA_ValidImage(APP_START_ADDR) && !IAP_InProgress()) {
    printf("Jump to application.\n");
    /* Jump sequence */
    FOTA_JumpToApp();
  }
  /* Power reset during FOTA, try once more */
  else if (IAP_InProgress()) {
    if (IAP_IO_Type() == ITYPE_VCU) {
      printf("FOTA set, do FOTA once more.\n");
      /* Everything went well, boot form new image */
      if (FOTA_Upgrade(ITYPE_VCU)) {
        /* Take branching decision on next reboot */
        FOTA_Reboot();
      }
      /* Erase partially programmed application area */
      FLASH_EraseAppArea();
    }
    /* Reset FOTA flag */
    IAP_ResetFlag();
    HAL_NVIC_SystemReset();
  }
  /* Try to restore the backup */
  else {
    /* Check is the backup image valid */
    if (FOTA_ValidImage(BKP_START_ADDR)) {
      printf("Has backed-up image, roll-back.\n");
      /* Restore back old image to application area */
      if (FLASH_RestoreApp()) {
        /* Take branching decision on next reboot */
        FOTA_Reboot();
      }
    } else {
      printf("No image at all, do FOTA.\n");
      /* Download new firmware for the first time */
      if (FOTA_Upgrade(ITYPE_VCU)) {
        /* Take branching decision on next reboot */
        FOTA_Reboot();
      }
    }
    HAL_NVIC_SystemReset();
    /* Meaningless, failure indicator */
    logError("Boot-loader failure!!");
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  //  logError("Error Handler");
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
   */
  printf("Wrong parameters value: file %s on line %d\r\n", file, line) * /
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
