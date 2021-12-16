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
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Ref.: https://stm32f4-discovery.net/hal_api/group___t_m___f_a_t_f_s.html */
/* See Project>FATFS>App>fatfs.c file*/
//Use custom get_fattime() function
#define FATFS_CUSTOM_FATTIME 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile DS3231_Time_t tm1;
uint16_t year_century = 2000;

uint32_t display_time_cnt = 0;

uint32_t usb_failure_timeout = 0;
uint32_t usb_failure_timeout2 = 0;
uint32_t usb_failure_timeout3 = 0;
uint32_t usb2_failure_cnt = 0;
uint32_t usb2_fail_freq = 0;
uint8_t usb2_failure_flag = 0;
uint8_t usb2_failure_flag_old = 0;
uint8_t button1_flag = 0;
uint8_t button2_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
int __io_putchar(int ch);
int _write(int file, char *ptr, int len);
void beep(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

extern uint8_t MSC_Application(void);
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
	MX_USART1_UART_Init();
	MX_USB_HOST_Init();
	MX_FATFS_Init();
	/* USER CODE BEGIN 2 */
	tm1.day = 7; /* (week) */
	tm1.year = 99;
	tm1.month = 12;
	tm1.date = 31;
	tm1.hours = 23;
	tm1.minutes = 59;
	tm1.seconds = 58;

	beep();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
		MX_USB_HOST_Process();

		/* USER CODE BEGIN 3 */
		if ((usb_failure_timeout2 + 100) < HAL_GetTick()) {
			usb_failure_timeout2 = HAL_GetTick();
			if ((usb2_failure_flag == 1) && (usb2_failure_flag_old == 0)) {
				usb2_failure_flag_old = 1;
				//UsrLog("USB2 Power fail (interrupt freq: %lu)", usb2_fail_freq);
				UsrLog("USB2 Power fail: Deactivating power");
				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET);
			}

			if ((usb_failure_timeout3 + 5000) < HAL_GetTick()) {
				usb_failure_timeout3 = HAL_GetTick();

				if ((usb2_failure_flag == 0) && (usb2_failure_flag_old == 1)) {
					usb2_failure_flag_old = 0;
					UsrLog("Reactivating power");
					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_RESET);
				}
			}

			if (button1_flag == 1) {
				beep();

				MSC_Application();

				beep();

				button1_flag = 0;
			}

			if (button2_flag == 1) {
				beep();

				button2_flag = 0;
			}
		}

		if ((display_time_cnt + 999) < HAL_GetTick()) {
			display_time_cnt = HAL_GetTick();
			UsrLog("%u/%u/%u %u:%u:%u", tm1.month, tm1.date,
					year_century + tm1.year, tm1.hours, tm1.minutes,
					tm1.seconds);
		}
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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
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
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, DC_DC_ON_Pin | LED2_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : BUZZER_Pin DC_DC_ON_Pin LED2_Pin */
	GPIO_InitStruct.Pin = BUZZER_Pin | DC_DC_ON_Pin | LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_HS_PWR_Pin UC_PWR_Pin */
	GPIO_InitStruct.Pin = USB_HS_PWR_Pin | UC_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_HS_OC_Pin BUTTON1_Pin BUTTON2_Pin */
	GPIO_InitStruct.Pin = USB_HS_OC_Pin | BUTTON1_Pin | BUTTON2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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

void beep(void) {
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

/* Handle interrupt */
// EXTI Line External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_9) // If The INT Source Is EXTI Line9 (PD9 Pin)
	{
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // Toggle The Output (LED) Pin
		usb2_failure_cnt++;
		//UsrLog("USB2 Power fail"); usb2_failure_flag
	}
	if (GPIO_Pin == GPIO_PIN_11) // If The INT Source Is EXTI Line11 (PD11 Pin)
	{
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // Toggle The Output (LED) Pin
		//UsrLog("Button1 (Run: MSC_Application)");

		button1_flag = 1;
	}
	if (GPIO_Pin == GPIO_PIN_15) // If The INT Source Is EXTI Line15 (PD15 Pin)
	{
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // Toggle The Output (LED) Pin
		//UsrLog("Button2");
		button2_flag = 1;
	}
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
