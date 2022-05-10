/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
	TemperatureUnit_C = 0,
	TemperatureUnit_F
}TemperatureUnitTypeDef;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/*
#ifndef UsrLog
#define  UsrLog(...)   do { \
} while (0)
#endif
*/

 #ifndef UsrLog
 #define  UsrLog(...)   do { \
		printf(__VA_ARGS__); \
		printf("\r"); \
		printf("\n"); \
 } while (0)
 #endif
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUZZER_Pin GPIO_PIN_2
#define BUZZER_GPIO_Port GPIOE
#define HC595_OE_Pin GPIO_PIN_0
#define HC595_OE_GPIO_Port GPIOB
#define HC595_LAT_Pin GPIO_PIN_1
#define HC595_LAT_GPIO_Port GPIOB
#define SENSOR_PWR_Pin GPIO_PIN_7
#define SENSOR_PWR_GPIO_Port GPIOE
#define SENSOR1_Pin GPIO_PIN_8
#define SENSOR1_GPIO_Port GPIOE
#define SENSOR2_Pin GPIO_PIN_9
#define SENSOR2_GPIO_Port GPIOE
#define SENSOR3_Pin GPIO_PIN_10
#define SENSOR3_GPIO_Port GPIOE
#define SENSOR4_Pin GPIO_PIN_11
#define SENSOR4_GPIO_Port GPIOE
#define SENSOR5_Pin GPIO_PIN_12
#define SENSOR5_GPIO_Port GPIOE
#define SENSOR6_Pin GPIO_PIN_13
#define SENSOR6_GPIO_Port GPIOE
#define SENSOR7_Pin GPIO_PIN_14
#define SENSOR7_GPIO_Port GPIOE
#define SENSOR8_Pin GPIO_PIN_15
#define SENSOR8_GPIO_Port GPIOE
#define HC595_DAT_Pin GPIO_PIN_10
#define HC595_DAT_GPIO_Port GPIOB
#define UC_PWR_Pin GPIO_PIN_2
#define UC_PWR_GPIO_Port GPIOD
#define HC595_CLK_Pin GPIO_PIN_8
#define HC595_CLK_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_0
#define LED2_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
