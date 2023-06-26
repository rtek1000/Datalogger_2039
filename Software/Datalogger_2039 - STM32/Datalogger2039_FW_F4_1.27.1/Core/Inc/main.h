/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "1-Version.h"
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "log_uart.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define maxChannel 8U

typedef enum {
	LcdScreenGraphic = 0,
	LcdScreenCfgCh,
	LcdScreenCfgChConfirm,
	LcdScreenCfgChCheck,
	LcdScreenCfgClk,
	LcdScreenMonCh,
	LcdScreenMonChOffset,
	LcdScreenBattChr,
	LcdScreenPassword,
	LcdScreenUsbExport,
	LcdScreenMain,
	LcdScreenPasswordConfig,
	LcdScreenMemTest,
	LcdScreenCfgChMemErase,
	LcdScreenLanguage
} LcdScreenTypeDef;

typedef enum {
	TemperatureUnit_F = 0, TemperatureUnit_C
} TemperatureUnitTypeDef;

typedef enum {
	memTestStatusBegin = 0,
	memTestStatusTest,
	memTestStatusFail,
	memTestStatusPass,
	memTestStatusLock,
	memTestStatusSkip
} memTestStatusTypeDef;

typedef enum {
	Battery_in_use = 0, Battery_charging, Battery_charged, Battery_error
} BattStatusTypeDef;

typedef enum {
	ScreenActive = 0, ScreenDimm, ScreenOff, ScreenAutoDimm
} ScreenStateTypeDef;

typedef enum {
	Zoom_Week = 0, Zoom_Day, Zoom_Hour
} ChartZoomTypeDef;

typedef struct _DATAHEADER_ {
	uint8_t FLAG_ENABLED;
	uint8_t START_DAY;
	uint8_t START_MONTH;
	uint8_t START_YEAR;
	uint8_t START_HOUR;
	uint8_t START_MINUTE;
	uint8_t STOP_DAY;
	uint8_t STOP_MONTH;
	uint8_t STOP_YEAR;
	uint8_t STOP_HOUR;
	uint8_t STOP_MINUTE;
} DATAHEADER_TypeDef;

typedef enum {
	lang_en = 0, lang_es, lang_ptBr
} langStatusTypeDef;

typedef enum {
	sepDot = 0, sepComma
} sepDecStatusTypeDef;

typedef enum {
	mmddyy = 0, ddmmyy
} dateFormatStatusTypeDef;

typedef enum {
	datFile16 = 0, csvFile, datFile64
} datTypeStatusTypeDef;

typedef struct _CONFIGSTRUCT_ {
	uint16_t FileCntDat;
	uint16_t FileCntCsv;
	uint8_t FileType;
	uint8_t DateFormat;
	uint8_t DecimalSeparator;
	uint8_t TemperatureUnit;
	uint8_t Language;
	uint8_t Key[32];
	char PW_Code[6];
	uint32_t SensorOffset[maxChannel];
	uint8_t ChExported[8]; // maxChannel = 8 bits
} ConfigStruct_TypeDef;

/*
 - Sensor Temp: 2 bytes
 - Sensor ID: 8 bytes
 - Ang X: 1 byte
 - Ang Y: 1 byte
 - Bat V: 1 byte
 - AC V: 1 byte
 - Evento: 1 byte
 */

typedef struct _DATASTRUCT_ {
	uint8_t Temperature[2];
	uint8_t Sensor_Id[8];
	uint8_t AccX;
	uint8_t AccY;
	uint8_t BattV;
	uint8_t ChargerV;
	//uint8_t Event;
	uint8_t Date[3];
	uint8_t Time[2];
} DadaStruct_TypeDef;

typedef struct _DATASTRUCT2_ {
	char Temperature[8]; // 12.1234
	char Sensor_Id[17];  // 8*2=16
	int AccX; // -89
	int AccY; // -89
	float BattV; // 3.75
	float ChargerV; // 33.75
	//char Event[4];
	char Date[9]; // dd/mm/aa
	char Time[7]; // hh:mm

} DadaStruct2_TypeDef;

typedef struct _DATASTRUCT2b_ {
	float Temperature_f; // 12.1234
	uint8_t Temperature[2]; // 12.1234
	char Sensor_Id[18];  // 8*2=16
	uint8_t AccX; // -89
	uint8_t AccY; // -89
	uint8_t BattV; // 3.75
	uint8_t ChargerV; // 33.75
	//char Event[4];
	char Date[9]; // dd/mm/aa
	char Time[7]; // hh:mm
} DadaStruct2b_TypeDef;

typedef struct _DATASTRUCT3_ {
	uint8_t start_day;
	uint8_t start_month;
	uint8_t start_year;
	uint8_t start_hour;
	uint8_t start_minute;
	uint8_t stop_day;
	uint8_t stop_month;
	uint8_t stop_year;
	uint8_t stop_hour;
	uint8_t stop_minute;
	uint8_t lenght_day;
	uint8_t lenght_hour;
	uint8_t lenght_minute;
} DadaStruct3_TypeDef;

// https://cs.stanford.edu/people/miles/iso8859.html
typedef enum {
	circumflex_accent = 94,
	grave_accent = 96,
	tilde_accent = 126,
	diaeresis_accent = 168,
	acute_accent = 180,

	C_cedil = 199,
	c_cedil = 231,

	A_grave_accent = 192,
	A_acute_accent = 193,
	A_circumflex_accent = 194,
	A_tilde_accent = 195,
	A_diaeresis_accent = 196,

	E_grave_accent = 200,
	E_acute_accent = 201,
	E_circumflex_accent = 202,
	E_diaeresis_accent = 203,

	I_grave_accent = 204,
	I_acute_accent = 205,
	I_circumflex_accent = 206,
	I_diaeresis_accent = 207,

	O_grave_accent = 210,
	O_acute_accent = 211,
	O_circumflex_accent = 212,
	O_tilde_accent = 213,
	O_diaeresis_accent = 214,

	U_grave_accent = 217,
	U_acute_accent = 218,
	U_circumflex_accent = 219,
	U_diaeresis_accent = 220,

	a_grave_accent = 224,
	a_acute_accent = 225,
	a_circumflex_accent = 226,
	a_tilde_accent = 227,
	a_diaeresis_accent = 228,

	e_grave_accent = 232,
	e_acute_accent = 233,
	e_circumflex_accent = 234,
	e_diaeresis_accent = 235,

	i_grave_accent = 236,
	i_acute_accent = 237,
	i_circumflex_accent = 238,
	i_diaeresis_accent = 239,

	o_grave_accent = 242,
	o_acute_accent = 243,
	o_circumflex_accent = 244,
	o_tilde_accent = 245,
	o_diaeresis_accent = 246,

	u_grave_accent = 249,
	u_acute_accent = 250,
	u_circumflex_accent = 251,
	u_diaeresis_accent = 252
} ISO8859_1_CodePage_TypeDef;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#ifndef FALSE
#define FALSE 0U
#endif

#ifndef KBD_LOG
#define KBD_LOG 1U
#endif

#ifndef false
#define false 0U
#endif

#ifndef TRUE
#define TRUE  1U
#endif

#ifndef true
#define true  1U
#endif

#define W25Q64_ID 7

#define password_request_yes 1U
#define password_request_no 0U
#define maxChar 57U
#define maxChar_x2 200U

// #define output_csv_file 1U

#define check_lcd_status 1U

// #define DISABLE_FLASH_CRC_AT_BOOT 1U
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void IWDG_delay_ms(uint32_t delay);
void Sleep_wakeup_by_Timer(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_VBAT_Pin GPIO_PIN_0
#define ADC_VBAT_GPIO_Port GPIOA

#define ADC_CHARGER_Pin GPIO_PIN_3
#define ADC_CHARGER_GPIO_Port GPIOA

#define ADC_DC_DC_5V_Pin GPIO_PIN_4
#define ADC_DC_DC_5V_GPIO_Port GPIOA

#define ADC_VREF_Pin GPIO_PIN_6
#define ADC_VREF_GPIO_Port GPIOA

#define FLASH_CS_Pin GPIO_PIN_15
#define FLASH_CS_GPIO_Port GPIOA
/* -----------------------------------------------------------*/
#define HC595_OE_Pin GPIO_PIN_0
#define HC595_OE_GPIO_Port GPIOB

#define HC595_LAT_Pin GPIO_PIN_1
#define HC595_LAT_GPIO_Port GPIOB

#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB

#define HC595_CLK_Pin GPIO_PIN_8
#define HC595_CLK_GPIO_Port GPIOB

#define HC595_DAT_Pin GPIO_PIN_10
#define HC595_DAT_GPIO_Port GPIOB
/* -----------------------------------------------------------*/
#define CHARGER_STATE_Pin GPIO_PIN_0
#define CHARGER_STATE_GPIO_Port GPIOC

#define RTC_RESET_Pin GPIO_PIN_3
#define RTC_RESET_GPIO_Port GPIOC

#define USB_FS_OC_Pin GPIO_PIN_7
#define USB_FS_OC_GPIO_Port GPIOC

#define USB_FS_PWR_Pin GPIO_PIN_8
#define USB_FS_PWR_GPIO_Port GPIOC
/* -----------------------------------------------------------*/
#define FLASH_WP_Pin GPIO_PIN_0
#define FLASH_WP_GPIO_Port GPIOD

#define FLASH_PWR_Pin GPIO_PIN_1
#define FLASH_PWR_GPIO_Port GPIOD

#define UC_PWR_Pin GPIO_PIN_2
#define UC_PWR_GPIO_Port GPIOD

#define LCD_PWR_Pin GPIO_PIN_4
#define LCD_PWR_GPIO_Port GPIOD

#define LCD_RST_Pin GPIO_PIN_5
#define LCD_RST_GPIO_Port GPIOD

#define LCD_DC_Pin GPIO_PIN_6
#define LCD_DC_GPIO_Port GPIOD

#define LCD_CS_Pin GPIO_PIN_7
#define LCD_CS_GPIO_Port GPIOD
/* -----------------------------------------------------------*/
#define USB_HS_PWR_Pin GPIO_PIN_8
#define USB_HS_PWR_GPIO_Port GPIOD

#define USB_HS_OC_Pin GPIO_PIN_9
#define USB_HS_OC_GPIO_Port GPIOD

#define BUTTON1_Pin GPIO_PIN_11
#define BUTTON1_GPIO_Port GPIOD

#define RTC_INT_STATE_Pin GPIO_PIN_12
#define RTC_INT_STATE_GPIO_Port GPIOD

#define AC_DC_STATE_Pin GPIO_PIN_13
#define AC_DC_STATE_GPIO_Port GPIOD

#define BUTTON2_Pin GPIO_PIN_15
#define BUTTON2_GPIO_Port GPIOD
/* -----------------------------------------------------------*/
#define LED2_Pin GPIO_PIN_0
#define LED2_GPIO_Port GPIOE

#define RTC_PWR_Pin GPIO_PIN_1
#define RTC_PWR_GPIO_Port GPIOE

#define BUZZER_Pin GPIO_PIN_2
#define BUZZER_GPIO_Port GPIOE

#define DC_DC_ON_Pin GPIO_PIN_3
#define DC_DC_ON_GPIO_Port GPIOE

#define I2C3_PWR_Pin GPIO_PIN_4
#define I2C3_PWR_GPIO_Port GPIOE

#define ETH_PWR_Pin GPIO_PIN_6
#define ETH_PWR_GPIO_Port GPIOE

#define SENSOR_PWR_Pin GPIO_PIN_7
#define SENSOR_PWR_GPIO_Port GPIOE
/* -----------------------------------------------------------*/
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

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
