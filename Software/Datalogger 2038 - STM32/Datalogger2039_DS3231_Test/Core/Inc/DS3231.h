/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

// Arduino DS3232RTC Library
// https://github.com/JChristensen/DS3232RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Arduino library for the Maxim Integrated DS3232 and DS3231
// Real-Time Clocks.
// Requires PJRC's improved version of the Arduino Time Library,
// https://playground.arduino.cc/Code/Time
// https://github.com/PaulStoffregen/Time

#ifndef SRC_DS3231_H_
#define SRC_DS3231_H_

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>

typedef struct {
	uint8_t seconds; /*!< Seconds parameter, from 00 to 59 */
	uint8_t minutes; /*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;   /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t day;     /*!< Day in a week, from 1 to 7 */
	uint8_t date;    /*!< Date in a month, 1 to 31 */
	uint8_t month;   /*!< Month in a year, 1 to 12 */
	uint8_t year;    /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
} DS3231_Time_t;

/* I2C settings for I2C library */
#ifndef DS3231_I2C
#define DS3231_I2C					I2C1
#define DS3231_I2C_PINSPACK			I2C_PinsPack_1
#endif

/* I2C slave address for DS1307 */
#define DS3231_I2C_ADDR				0xD0

/* Registers location */
#define DS3231_SECONDS				0x00
#define DS3231_MINUTES				0x01
#define DS3231_HOURS				0x02
#define DS3231_DAY					0x03
#define DS3231_DATE					0x04
#define DS3231_MONTH				0x05
#define DS3231_YEAR					0x06
#define DS3231_CONTROL				0x07

/* Bits in control register */
#define DS3231_CONTROL_OUT			7
#define DS3231_CONTROL_SQWE			4
#define DS3231_CONTROL_RS1			1
#define DS3231_CONTROL_RS0			0

typedef enum {
	DS3231_Result_Ok = 0x00,         /*!< Everything OK */
	DS3231_Result_Error,             /*!< An error occurred */
	DS3231_Result_DeviceNotConnected /*!< Device is not connected */
} DS3231_Result_t;

typedef struct  {
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
	uint8_t Wday;   // day of week, sunday is day 1
	uint8_t Day;
	uint8_t Month;
	uint8_t Year;   // offset from 1970;
} 	tmElements_t, TimeElements, *tmElementsPtr_t;

//#define time_t uint32_t

// Alarm masks
typedef enum {
    ALM1_EVERY_SECOND = 0x0F,
    ALM1_MATCH_SECONDS = 0x0E,
    ALM1_MATCH_MINUTES = 0x0C,     // match minutes *and* seconds
    ALM1_MATCH_HOURS = 0x08,       // match hours *and* minutes, seconds
    ALM1_MATCH_DATE = 0x00,        // match date *and* hours, minutes, seconds
    ALM1_MATCH_DAY = 0x10,         // match day *and* hours, minutes, seconds
    ALM2_EVERY_MINUTE = 0x8E,
    ALM2_MATCH_MINUTES = 0x8C,     // match minutes
    ALM2_MATCH_HOURS = 0x88,       // match hours *and* minutes
    ALM2_MATCH_DATE = 0x80,        // match date *and* hours, minutes
    ALM2_MATCH_DAY = 0x90,         // match day *and* hours, minutes
} ALARM_TYPES_t;

// Square-wave output frequency (TS2, RS1 bits)
typedef enum {
    SQWAVE_1_HZ,
    SQWAVE_1024_HZ,
    SQWAVE_4096_HZ,
    SQWAVE_8192_HZ,
    SQWAVE_NONE
} SQWAVE_FREQS_t;

typedef enum {
	NoClearOSF = 0,
	ClearOSF
} OscStoppedEnumTypeDef;

typedef enum {
	ALARME_DISABLED = 0,
	ALARME_ENABLED
} AlarmEnableEnumTypeDef;



// Alarm mask bits
#define A1M1 7
#define A1M2 7
#define A1M3 7
#define A1M4 7
#define A2M2 7
#define A2M3 7
#define A2M4 7

#define RTC_CONTROL 0x0E
#define RTC_STATUS 0x0F

// Control register bits
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0

// Status register bits
#define OSF 7
#define BB32KHZ 6
#define CRATE1 5
#define CRATE0 4
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

#define DYDT 6                     // Day/Date flag bit in alarm Day/Date registers

#define ALM1_SECONDS 0x07
#define ALM1_MINUTES 0x08
#define ALM1_HOURS 0x09
#define ALM1_DAYDATE 0x0A
#define ALM2_MINUTES 0x0B
#define ALM2_HOURS 0x0C
#define ALM2_DAYDATE 0x0D

#define ALARM_1 1                  // constants for alarm functions
#define ALARM_2 2

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

HAL_StatusTypeDef DS3231_init(I2C_HandleTypeDef *i2c_port);
HAL_StatusTypeDef DS3231_Setup(void);
void DS3231_IO_as_OUTPUT(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef readRTC(uint8_t addr, uint8_t *value);
HAL_StatusTypeDef DS3231_receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
		uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef DS3231_send(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
		uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t DS3231_Bcd2Bin(uint8_t bcd);
uint8_t DS3231_Bin2Bcd(uint8_t bin);
HAL_StatusTypeDef DS3231_GetDateTime(volatile DS3231_Time_t *time);
HAL_StatusTypeDef DS3231_SetDateTime(DS3231_Time_t *time);
HAL_StatusTypeDef writeRTC(uint8_t addr, uint8_t value);
HAL_StatusTypeDef readRTC(uint8_t addr, uint8_t *value);
HAL_StatusTypeDef DS3231_SetAlarmSec(ALARM_TYPES_t alarmType, uint8_t seconds,
		uint8_t minutes, uint8_t hours, uint8_t daydate);
HAL_StatusTypeDef DS3231_SetAlarm(ALARM_TYPES_t alarmType, uint8_t minutes,
		uint8_t hours, uint8_t daydate);
HAL_StatusTypeDef DS3231_AlarmInterrupt(uint8_t alarmNumber,
		uint8_t interruptEnabled);
HAL_StatusTypeDef DS3231_Alarm(uint8_t alarmNumber, uint8_t *alarmFlagBit);
HAL_StatusTypeDef DS3231_OscStopped(uint8_t clearOSF, uint8_t *OscStopped);
HAL_StatusTypeDef DS3231_SquareWave(SQWAVE_FREQS_t freq);

#endif /* SRC_DS3232RTC_H_ */
