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

// 2021-06-24
// Datalogger 2039 temperature sensor test
// - Short circuit protection for data line and power

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DS18B20.h"
#include "IWDG_Delay.h"
#include "SN74HC595.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

//           |     flag     |
// Need add: -u _printf_float to:
// Properties > C/C++ Build > Settings >
// MCU GCC Linker > Miscellaneous > Others flags

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NumOfSensors 8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
char str[80];
uint32_t interval2500ms = 0;

const uint8_t Num_Of_Sensors = NumOfSensors;
DS18B20_TypeDef SENSORS[NumOfSensors];
//#define sensorFailTest 1	// uncomment to get fail

#ifndef sensorFailTest
OneWire_ChannelTypeDef OneWireCh0 = { SENSOR1_GPIO_Port, SENSOR1_Pin };
OneWire_ChannelTypeDef OneWireCh1 = { SENSOR2_GPIO_Port, SENSOR2_Pin };
OneWire_ChannelTypeDef OneWireCh2 = { SENSOR3_GPIO_Port, SENSOR3_Pin };
OneWire_ChannelTypeDef OneWireCh3 = { SENSOR4_GPIO_Port, SENSOR4_Pin };
OneWire_ChannelTypeDef OneWireCh4 = { SENSOR5_GPIO_Port, SENSOR5_Pin };
OneWire_ChannelTypeDef OneWireCh5 = { SENSOR6_GPIO_Port, SENSOR6_Pin };
OneWire_ChannelTypeDef OneWireCh6 = { SENSOR7_GPIO_Port, SENSOR7_Pin };
OneWire_ChannelTypeDef OneWireCh7 = { SENSOR8_GPIO_Port, SENSOR8_Pin };
#endif

#ifdef sensorFailTest
OneWire_ChannelTypeDef OneWireCh0 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh1 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh2 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh3 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh4 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh5 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh6 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh7 = {GPIOC, GPIO_PIN_15};
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void print_temperatures(void);
void beep(uint8_t repetions);

extern void HC595_init(void);
extern void HC595_set(uint8_t value);
extern void DS18B20_init(void);
extern void DS18B20_IO_as_INPUT(OneWire_ChannelTypeDef channel);
extern void DS18B20_IO_as_OUTPUT(OneWire_ChannelTypeDef channel);
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
	if (DWT_Delay_Init()) {
		Error_Handler(); /* Call Error Handler */
	}

	SENSORS[0].CHANNEL = OneWireCh0;
	SENSORS[1].CHANNEL = OneWireCh1;
	SENSORS[2].CHANNEL = OneWireCh2;
	SENSORS[3].CHANNEL = OneWireCh3;
	SENSORS[4].CHANNEL = OneWireCh4;
	SENSORS[5].CHANNEL = OneWireCh5;
	SENSORS[6].CHANNEL = OneWireCh6;
	SENSORS[7].CHANNEL = OneWireCh7;

	SENSORS[0].FLAG_ENABLED = 1;
	SENSORS[1].FLAG_ENABLED = 1;
	SENSORS[2].FLAG_ENABLED = 1;
	SENSORS[3].FLAG_ENABLED = 1;
	SENSORS[4].FLAG_ENABLED = 1;
	SENSORS[5].FLAG_ENABLED = 1;
	SENSORS[6].FLAG_ENABLED = 1;
	SENSORS[7].FLAG_ENABLED = 1;
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
	MX_IWDG_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	DS18B20_init();

	beep(1);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if ((HAL_GetTick() - 2500) >= interval2500ms) {
			interval2500ms = HAL_GetTick();

			if (DS18B20_read_all(SENSORS, NumOfSensors) != HAL_OK) {
				uint8_t beep_error = 0;

				//UsrLog("DS18B20_read_all NOT OK");

				for (uint8_t i = 0; i < 8; i++) {
					if ((SENSORS[i].FLAG_ENABLED == 1)
							&& (SENSORS[i].FLAG_TIMEOUT == 1)) // sensor error
							{
						beep_error = 1;
					}
				}

				if (beep_error == 1) // sensor error
						{
					UsrLog("DS18B20_read_all Error");
					//beep(2);
				}
			} else {
				print_temperatures();
			}

			DS18B20_start_conversion(SENSORS, NumOfSensors);
		}

		IWDG_delay_ms(0); // Clear IWDG
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
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			BUZZER_Pin | SENSOR_PWR_Pin | SENSOR1_Pin | SENSOR2_Pin
					| SENSOR3_Pin | SENSOR4_Pin | SENSOR5_Pin | SENSOR6_Pin
					| SENSOR7_Pin | SENSOR8_Pin | LED2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(HC595_OE_GPIO_Port, HC595_OE_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, HC595_LAT_Pin | HC595_DAT_Pin | HC595_CLK_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : BUZZER_Pin SENSOR_PWR_Pin SENSOR1_Pin SENSOR2_Pin
	 SENSOR3_Pin SENSOR4_Pin SENSOR5_Pin SENSOR6_Pin
	 SENSOR7_Pin SENSOR8_Pin LED2_Pin */
	GPIO_InitStruct.Pin = BUZZER_Pin | SENSOR_PWR_Pin | SENSOR1_Pin
			| SENSOR2_Pin | SENSOR3_Pin | SENSOR4_Pin | SENSOR5_Pin
			| SENSOR6_Pin | SENSOR7_Pin | SENSOR8_Pin | LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : HC595_OE_Pin HC595_LAT_Pin HC595_DAT_Pin HC595_CLK_Pin */
	GPIO_InitStruct.Pin = HC595_OE_Pin | HC595_LAT_Pin | HC595_DAT_Pin
			| HC595_CLK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : UC_PWR_Pin */
	GPIO_InitStruct.Pin = UC_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(UC_PWR_GPIO_Port, &GPIO_InitStruct);

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

void print_temperatures(void) {
	uint8_t cnt1 = 0;

	for (int i = 0; i < 8; i++) {
		if (SENSORS[i].FLAG_ENABLED == 1){
			cnt1++;
		}

		if ((SENSORS[i].FLAG_ENABLED == 1)
				&& (SENSORS[i].FLAG_POWER_FAIL != 1)) {
			uint8_t sensor_ID[8];

			for (int j = 0; j < 8; j++) {
				sensor_ID[j] = SENSORS[i].SERIAL_ID[j];
			}

			DS18B20_hex_id(sensor_ID, str);

			UsrLog("Sensor %d Temper: %.4f*C ID: %s", i + 1,
					SENSORS[i].TEMPERATURE, str);
		} else if ((SENSORS[i].FLAG_ENABLED == 1)
				&& (SENSORS[i].FLAG_POWER_FAIL == 1)) {
			UsrLog("Sensor %d error: energy failure, check for short circuit", i);
		}
	}

	if(cnt1 == 0) {
		UsrLog("All Sensors Disabled");
	}
}

void beep(uint8_t repetions) {
	while (repetions--) {
		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // LED2 On
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On

		IWDG_delay_ms(5);

		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // LED2 Off
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off

		if (repetions > 0)
			IWDG_delay_ms(25);
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
