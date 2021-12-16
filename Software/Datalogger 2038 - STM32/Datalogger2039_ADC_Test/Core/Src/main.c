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
#include "stm32f4xx_hal_adc_ex.h"
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
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t TP4056_charger_freq = 0;
uint32_t AD1_RES_ADC_DC_DC_5V = 0;
uint32_t AD2_RES_ADC_CHARGER = 0;
uint32_t AD3_RES_ADC_VBAT = 0;
uint32_t AD1_RES_ADC_VREF = 0;

/* ADC const calibration */
float const_adc_x2 = 0;
float const_adc_x16 = 0;

float batt_percent_up = 0;
float batt_percent_down = 100;

uint32_t battery_failure_timeout = 0;
volatile uint32_t battery_failure_cnt = 0;
volatile HAL_StatusTypeDef battery_failure_flag = HAL_OK;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void get_ADC_Vref(void);
float map(float x, float in_min, float in_max, float out_min, float out_max);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float map(float x, float in_min, float in_max, float out_min, float out_max) {
	if ((in_max - in_min) > (out_max - out_min)) {
		return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1)
				+ out_min;
	} else {
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}
}
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
	MX_ADC1_Init();
	MX_ADC2_Init();
	MX_ADC3_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	HAL_Delay(50);

	get_ADC_Vref();

	HAL_GPIO_WritePin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin, GPIO_PIN_SET);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		get_ADC_Vref();

		AD1_RES_ADC_DC_DC_5V = 0;
		AD2_RES_ADC_CHARGER = 0;
		AD3_RES_ADC_VBAT = 0;

		for (int i = 0; i < 50; i++) {
			// Start ADC Conversion
			HAL_ADC_Start(&hadc1);
			HAL_ADC_Start(&hadc2);
			HAL_ADC_Start(&hadc3);
			// Poll ADC1 Perihperal & TimeOut = 1mSec
			HAL_ADC_PollForConversion(&hadc1, 1);
			HAL_ADC_PollForConversion(&hadc2, 1);
			HAL_ADC_PollForConversion(&hadc3, 1);
			// Read The ADC Conversion Result & Map It To PWM DutyCycle
			AD1_RES_ADC_DC_DC_5V += HAL_ADC_GetValue(&hadc1);
			AD2_RES_ADC_CHARGER += HAL_ADC_GetValue(&hadc2);
			AD3_RES_ADC_VBAT += HAL_ADC_GetValue(&hadc3);
		}

		float f_adc1 = 0, f_adc2 = 0, f_adc3 = 0; //, f_vref = 0;

		f_adc1 = AD1_RES_ADC_DC_DC_5V / 50;
		f_adc2 = AD2_RES_ADC_CHARGER / 50;
		f_adc3 = AD3_RES_ADC_VBAT / 50;

		// (3.3 / 4096) = 0,000805664
		// R1=10k; R2=10k
		// 3.3V=10k; 20k=2*3.3
		// 0,000805664 * 2 = 0,001611328
		// 0,001611328 * 0,916996047 = 0,001477581
		// R1=15k; R2=1k
		// 3.3V=1k; 16k=16*3.3
		// 0,000805664 * 16 = 0,012890624
		// 0,012890624 * 0,916996047 = 0,011820651

		f_adc1 = f_adc1 * const_adc_x2;
		f_adc2 = f_adc2 * const_adc_x16;
		f_adc3 = f_adc3 * const_adc_x2;

		UsrLog("ADC1 %0.1fV, ADC2 %0.1fV, ADC3 %0.1fV", f_adc1, f_adc2, f_adc3);
		UsrLog("TP4056 LED CHRG freq: %lu", TP4056_charger_freq);

		// battery_failure_flag:
		// interruption on pin PA5 (frequency counter)
		// ---> see stm32f4xx_it.c file

		if (battery_failure_flag == 1) {
			UsrLog("Battery: error; tick: %lu", HAL_GetTick());
		} else {
			if (HAL_GPIO_ReadPin(CHARGER_STATE_GPIO_Port, CHARGER_STATE_Pin)
					== GPIO_PIN_RESET) {
				if (f_adc2 < 8.0) {
					UsrLog("Battery: in use; tick: %lu", HAL_GetTick());

					batt_percent_up = 0.0;
				} else {
					UsrLog("Battery: charging; tick: %lu", HAL_GetTick());
				}
			} else {
				UsrLog("Battery: charged; tick: %lu", HAL_GetTick());
			}
		}

		HAL_Delay(2000);
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
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void) {

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc2.Init.Resolution = ADC_RESOLUTION_12B;
	hadc2.Init.ScanConvMode = DISABLE;
	hadc2.Init.ContinuousConvMode = DISABLE;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DMAContinuousRequests = DISABLE;
	hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc2) != HAL_OK) {
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_3;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */

	/* USER CODE END ADC2_Init 2 */

}

/**
 * @brief ADC3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC3_Init(void) {

	/* USER CODE BEGIN ADC3_Init 0 */

	/* USER CODE END ADC3_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC3_Init 1 */

	/* USER CODE END ADC3_Init 1 */
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc3.Init.Resolution = ADC_RESOLUTION_12B;
	hadc3.Init.ScanConvMode = DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.NbrOfConversion = 1;
	hadc3.Init.DMAContinuousRequests = DISABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc3) != HAL_OK) {
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC3_Init 2 */

	/* USER CODE END ADC3_Init 2 */

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
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, BUZZER_Pin | DC_DC_ON_Pin | LED2_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LCD_BACK_PWM_GPIO_Port, LCD_BACK_PWM_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, UC_PWR_Pin | LCD_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : BUZZER_Pin DC_DC_ON_Pin LED2_Pin */
	GPIO_InitStruct.Pin = BUZZER_Pin | DC_DC_ON_Pin | LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : CHARGER_STATE_Pin */
	GPIO_InitStruct.Pin = CHARGER_STATE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CHARGER_STATE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LCD_BACK_PWM_Pin */
	GPIO_InitStruct.Pin = LCD_BACK_PWM_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(LCD_BACK_PWM_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : UC_PWR_Pin LCD_PWR_Pin */
	GPIO_InitStruct.Pin = UC_PWR_Pin | LCD_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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

/* Handle PA5 interrupt */
// EXTI Line5 External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_5) // If The INT Source Is EXTI Line5 (PA5 Pin)
	{
		battery_failure_cnt++;

		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // Toggle The Output (LED) Pin
	}
}

void get_ADC_Vref(void) {
	// Set ADC Channel 6 (PA6)
	hadc1.Instance->SQR3 = ADC_CHANNEL_6;

	AD1_RES_ADC_VREF = 0;

	for (int i = 0; i < 25; i++) {
		// Start ADC Conversion
		HAL_ADC_Start(&hadc1);
		// Poll ADC1 Perihperal & TimeOut = 1mSec
		HAL_ADC_PollForConversion(&hadc1, 1);
		// Read The ADC Conversion Result & Map It To PWM DutyCycle
		AD1_RES_ADC_VREF += HAL_ADC_GetValue(&hadc1);

		HAL_Delay(1);
	}

	// Set ADC Channel 4 (PA4)
	hadc1.Instance->SQR3 = ADC_CHANNEL_4;

	AD1_RES_ADC_VREF /= 25;

	UsrLog("%lu", AD1_RES_ADC_VREF);

	float f_vref = AD1_RES_ADC_VREF;

	f_vref = 2.495 / f_vref;

	// (3.3 / 4096) = 0,000805664
	// R1=10k; R2=10k
	// 3.3V=10k; 20k=2*3.3
	// 0,000805664 * 2 = 0,001611328
	// 0,001611328 * 0,916996047 = 0,001477581
	// R1=15k; R2=1k
	// 3.3V=1k; 16k=16*3.3
	// 0,000805664 * 16 = 0,012890624
	// 0,012890624 * 0,916996047 = 0,011820651

	const_adc_x2 = (f_vref * 2);
	const_adc_x16 = (f_vref * 16);
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
