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
typedef struct {
	uint8_t seconds; /*!< Seconds parameter, from 00 to 59 */
	uint8_t minutes; /*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;   /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t day;     /*!< Day in a week, from 1 to 7 */
	uint8_t date;    /*!< Date in a month, 1 to 31 */
	uint8_t month;   /*!< Month in a year, 1 to 12 */
	uint8_t year;    /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
} DS3231_Time_t;
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

/**/
#ifndef UsrLog
#define  UsrLog(...)   do { \
		printf(__VA_ARGS__); \
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
#define DC_DC_ON_Pin GPIO_PIN_3
#define DC_DC_ON_GPIO_Port GPIOE
#define USB_HS_PWR_Pin GPIO_PIN_8
#define USB_HS_PWR_GPIO_Port GPIOD
#define USB_HS_OC_Pin GPIO_PIN_9
#define USB_HS_OC_GPIO_Port GPIOD
#define USB_HS_OC_EXTI_IRQn EXTI9_5_IRQn
#define BUTTON1_Pin GPIO_PIN_11
#define BUTTON1_GPIO_Port GPIOD
#define BUTTON1_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON2_Pin GPIO_PIN_15
#define BUTTON2_GPIO_Port GPIOD
#define BUTTON2_EXTI_IRQn EXTI15_10_IRQn
#define UC_PWR_Pin GPIO_PIN_2
#define UC_PWR_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_0
#define LED2_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
