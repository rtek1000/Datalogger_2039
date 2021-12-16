/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "w25qxx.h"
#include "dwt_stm32_delay.h"
#include "IWDG_Delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef FALSE
#define FALSE 0U
#endif

#ifndef TRUE
#define TRUE  1U
#endif

#define W25Q64_ID 7
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t FLASH_Error = FALSE;
char str[80];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void W25Q64_init(void) {
	if (!W25qxx_Init()) {
		UsrLog("W25qxx_Init: fail");

		for (uint8_t i = 0; i < 4; i++) {
			if (!W25qxx_Init()) {
				UsrLog("W25qxx_Init: fail");
			} else {
				UsrLog("W25qxx_Init: ok");
				i = 4;
			}

			IWDG_delay_ms(500);
		}
	}

	IWDG_delay_ms(0);

	if (w25qxx.ID != W25Q64_ID) {
		for (uint8_t i = 0; i < 4; i++) {
			IWDG_delay_ms(500);
		}

		FLASH_Error = TRUE;

		UsrLog("W25Q64_ID: error");
	} else {
		FLASH_Error = FALSE;
	}

	W25qxx_ReadUniqID();

	UsrLog("W25Q64 UniqID: %02X%02X%02X%02X%02X%02X%02X%02X", w25qxx.UniqID[0],
			w25qxx.UniqID[1], w25qxx.UniqID[2], w25qxx.UniqID[3],
			w25qxx.UniqID[4], w25qxx.UniqID[5], w25qxx.UniqID[6],
			w25qxx.UniqID[7]);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	time_t t;
	srand((unsigned) time(&t));
	UsrLog("%d", rand() % 50);
	UsrLog("%d", rand() % 256);
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	if (DWT_Delay_Init()) {
		Error_Handler(); /* Call Error Handler */
	}
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_IWDG_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	W25Q64_init();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		W25qxx_ReadUniqID();

		UsrLog("SysTick: %lu; W25Q64 UniqID: %02X%02X%02X%02X%02X%02X%02X%02X",
				HAL_GetTick(), w25qxx.UniqID[0], w25qxx.UniqID[1],
				w25qxx.UniqID[2], w25qxx.UniqID[3], w25qxx.UniqID[4],
				w25qxx.UniqID[5], w25qxx.UniqID[6], w25qxx.UniqID[7]);

		uint8_t buff1[256];
		uint8_t buff2[256];
		uint32_t Page_Address = 0;
		uint32_t SectorAddr = 0;

		for (int i = 0; i < 256; i++) {
			buff1[i] = rand() % 256;
		}

		UsrLog("Rand() data:");

		for (int i = 0; i < 16; i++) {
			UsrLog(
					"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"
							"", buff1[i * 8], buff1[(i * 8) + 1],
					buff1[(i * 8) + 2], buff1[(i * 8) + 3], buff1[(i * 8) + 4],
					buff1[(i * 8) + 5], buff1[(i * 8) + 6], buff1[(i * 8) + 7],
					buff1[(i * 8) + 8], buff1[(i * 8) + 9], buff1[(i * 8) + 10],
					buff1[(i * 8) + 11], buff1[(i * 8) + 12],
					buff1[(i * 8) + 13], buff1[(i * 8) + 14],
					buff1[(i * 8) + 15]);
		}

		/*
		 * <<< Caution: EEPROM data will be erased!!! >>>
		 */

/* Uncomment the lines below to erase and write to EEPROM: */
//		/* Erase EEPROM */
//		W25qxx_EraseSector(SectorAddr);
//
//		/* Write random data */
//		W25qxx_WritePage(buff1, Page_Address, 0, 256);

		/* Read data */
		W25qxx_ReadPage(buff2, Page_Address, 0, 256);

		UsrLog("W25Q64 data:");
		for (int i = 0; i < 16; i++) {
			UsrLog(
					"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"
							"", buff2[i * 8], buff2[(i * 8) + 1],
					buff2[(i * 8) + 2], buff2[(i * 8) + 3], buff2[(i * 8) + 4],
					buff2[(i * 8) + 5], buff2[(i * 8) + 6], buff2[(i * 8) + 7],
					buff2[(i * 8) + 8], buff2[(i * 8) + 9], buff2[(i * 8) + 10],
					buff2[(i * 8) + 11], buff2[(i * 8) + 12],
					buff2[(i * 8) + 13], buff2[(i * 8) + 14],
					buff2[(i * 8) + 15]);
		}

		/* Compare data */
		int comp = 0;

		for (int i = 0; i < 256; i++) {
			if (buff1[i] != buff2[i]) {
				comp++;
			}
		}

		if (comp > 0) {
			UsrLog("Write random data: Error");
		} else {
			UsrLog("Write random data: OK");
		}

		IWDG_delay_ms(10000);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void) {

	/* USER CODE BEGIN IWDG_Init 0 */

	/* USER CODE END IWDG_Init 0 */

	/* USER CODE BEGIN IWDG_Init 1 */

	/* USER CODE END IWDG_Init 1 */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 4095;
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN IWDG_Init 2 */

	/* USER CODE END IWDG_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, BUZZER_Pin | LED2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, FLASH_WP_Pin | FLASH_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : BUZZER_Pin LED2_Pin */
	GPIO_InitStruct.Pin = BUZZER_Pin | LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : FLASH_CS_Pin */
	GPIO_InitStruct.Pin = FLASH_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(FLASH_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : FLASH_WP_Pin FLASH_PWR_Pin UC_PWR_Pin */
	GPIO_InitStruct.Pin = FLASH_WP_Pin | FLASH_PWR_Pin | UC_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
int __io_putchar(int ch) {
	uint8_t c[1];
	c[0] = ch & 0x00FF;
	HAL_UART_Transmit(&huart1, &*c, 1, 10);
	return ch;
}

int _write(int file, char *ptr, int len) {
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		__io_putchar(*ptr++);
	}
	return len;
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
