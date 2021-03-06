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
#include "DS3231.h"
#include "main.h"
#include <stdio.h>

/*
 #define  UsrLog(...)   do { \
		printf(__VA_ARGS__); \
		printf("\n"); \
} while (0)
 */

const uint8_t ALARME1_ENABLE = ALARME_DISABLED;  // enabled = 1, disabled = 0
const uint8_t ALARME2_ENABLE = ALARME_ENABLED;  // enabled = 1, disabled = 0

extern DS3231_Time_t tm1;
I2C_HandleTypeDef DS3231_i2c_port;

HAL_StatusTypeDef DS3231_init(I2C_HandleTypeDef *i2c_port) {
	uint8_t OscStopped = { 0 };

	uint8_t i = 0;

	/* DS3231 hardware reset */
	HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_SET);
	HAL_Delay(300);

	/* DS3231 sotfware reset */
	/* I2C interface
	 *
	 * If a microcontroller connected to the DS3231 resets
	 * because of a loss of VCC or other event, it is
	 * possible that the microcontroller and DS3231 I2C
	 * communications could become unsynchronized, e.g.,
	 * the microcontroller resets while reading data from
	 * the DS3231. When the microcontroller resets, the
	 * DS3231 I2C interface may be placed into a known state
	 * by toggling SCL until SDA is observed to be at a high level.
	 */

	DS3231_i2c_port.Instance = i2c_port->Instance;
	DS3231_i2c_port.Init.ClockSpeed = i2c_port->Init.ClockSpeed;
	DS3231_i2c_port.Init.DutyCycle = i2c_port->Init.DutyCycle;
	DS3231_i2c_port.Init.OwnAddress1 = i2c_port->Init.OwnAddress1;
	DS3231_i2c_port.Init.AddressingMode = i2c_port->Init.AddressingMode;
	DS3231_i2c_port.Init.DualAddressMode = i2c_port->Init.DualAddressMode;
	DS3231_i2c_port.Init.OwnAddress2 = i2c_port->Init.OwnAddress2;
	DS3231_i2c_port.Init.GeneralCallMode = i2c_port->Init.GeneralCallMode;
	DS3231_i2c_port.Init.NoStretchMode = i2c_port->Init.NoStretchMode;

	HAL_I2C_DeInit(i2c_port);

	uint32_t timeout = 0;

	/* Ref.: https://stackoverflow.com/questions/33454883/i-cant-get-ds3231-rtc-to-work*/

	/* resetRTC() - begin */
	while (HAL_GPIO_ReadPin(RTC_SDA_GPIO_Port, RTC_SDA_Pin) == GPIO_PIN_RESET) {
		DS3231_IO_as_OUTPUT(RTC_SDA_GPIO_Port, RTC_SDA_Pin);
		DS3231_IO_as_OUTPUT(RTC_SCL_GPIO_Port, RTC_SCL_Pin);

		HAL_Delay(1);

		HAL_GPIO_WritePin(RTC_SDA_GPIO_Port, RTC_SDA_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RTC_SCL_GPIO_Port, RTC_SCL_Pin, GPIO_PIN_SET);

		HAL_Delay(1);

		if (HAL_GPIO_ReadPin(RTC_SDA_GPIO_Port, RTC_SDA_Pin) == GPIO_PIN_SET) {
			UsrLog("RTC_SDA: HIGH");
			HAL_GPIO_WritePin(RTC_SDA_GPIO_Port, RTC_SDA_Pin, GPIO_PIN_RESET);
			HAL_Delay(1);
			HAL_GPIO_WritePin(RTC_SDA_GPIO_Port, RTC_SDA_Pin, GPIO_PIN_SET);
			HAL_Delay(1);
		}

		HAL_GPIO_WritePin(RTC_SCL_GPIO_Port, RTC_SCL_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);

		if (timeout < 1000) {
			timeout++;
		} else {
			UsrLog("DS3231 Init Error (timeout)");
			return HAL_ERROR;
		}
	}
	/* resetRTC() - end */

	if (HAL_I2C_Init(&DS3231_i2c_port) != HAL_OK) {

	}

	while (DS3231_OscStopped(NoClearOSF, &OscStopped) != HAL_OK) {
		if (i < 9) {
			i++;
		} else {
			break;
		}
		HAL_Delay(0);
	}

	if (OscStopped == 1) {
		UsrLog("DS3231_OscStopped");

		OscStopped = 0;

		i = 0;

		while (OscStopped == 1) {
			if (DS3231_OscStopped(ClearOSF, &OscStopped) != HAL_OK) {
				UsrLog("DS3231 ERROR");
			}

			UsrLog("DS3231_OscStopped");
			HAL_Delay(200);

			if (i < 9) {
				i++;
			} else {
				break;
			}
			HAL_Delay(0);
		}
	}

	if (DS3231_GetDateTime(&tm1) == HAL_OK) {
		UsrLog("DS3231_GetDateTime Ok");
		UsrLog("%02d/%02d/%02d %02d:%02d", tm1.date, tm1.month, tm1.year,
				tm1.hours, tm1.minutes);
	} else {
		i = 0;

		while (DS3231_GetDateTime(&tm1) != HAL_OK) {
			UsrLog("DS3231: Error");

			if (i < 9) {
				i++;
			} else {
				break;
			}
			HAL_Delay(0);
		}
	}

	if (DS3231_Setup() == HAL_OK) {
		UsrLog("DS3231_Setup Ok");
	} else {
		i = 0;

		while (DS3231_Setup() != HAL_OK) {
			UsrLog("DS3231: Error");

			if (i < 99) {
				i++;
			} else {
				return HAL_ERROR;
			}
			HAL_Delay(0);
		}
	}

	return HAL_OK;
}

void DS3231_IO_as_OUTPUT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	HAL_GPIO_DeInit(GPIOx, GPIO_Pin);
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

HAL_StatusTypeDef DS3231_Setup(void) {
	//	DS3231_Alarm(ALARM_1); // clear the alarm flag
	//
	//	DS3231_Alarm(ALARM_2); // clear the alarm flag

	if (DS3231_SquareWave(SQWAVE_NONE) != HAL_OK) {
		UsrLog("DS3231_SquareWave");
		return HAL_ERROR;
	} else if (DS3231_SetAlarmSec(ALM2_EVERY_MINUTE, 0, 0, 0, 0) != HAL_OK) // set Alarm 2 to occur after every minute
			{
		UsrLog("DS3231_SetAlarmSec");
		return HAL_ERROR;
	} else if (DS3231_AlarmInterrupt(ALARM_1, ALARME1_ENABLE) != HAL_OK) // clear the alarm flag
			{
		UsrLog("DS3231_AlarmInterrupt 1");
		return HAL_ERROR;
	} else if (DS3231_AlarmInterrupt(ALARM_2, ALARME2_ENABLE) != HAL_OK) // clear the alarm flag
			{
		UsrLog("DS3231_AlarmInterrupt 2");
		return HAL_ERROR;
	} else {
		return HAL_OK;
	}
}

HAL_StatusTypeDef DS3231_send(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
		uint8_t *pData, uint16_t Size, uint32_t Timeout) {

	HAL_StatusTypeDef result = HAL_I2C_Master_Transmit(hi2c, DevAddress, pData,
			Size, Timeout);

	return result;
}

HAL_StatusTypeDef DS3231_receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
		uint8_t *pData, uint16_t Size, uint32_t Timeout) {

	HAL_StatusTypeDef result = HAL_I2C_Master_Receive(hi2c, DevAddress, pData,
			Size, Timeout);

	return result;
}

uint8_t DS3231_Bcd2Bin(uint8_t bcd) {
	uint8_t dec = 10 * (bcd >> 4);
	dec += bcd & 0x0F;
	return dec;
}

uint8_t DS3231_Bin2Bcd(uint8_t bin) {
	uint8_t low = 0;
	uint8_t high = 0;

	/* High nibble */
	high = bin / 10;
	/* Low nibble */
	low = bin - (high * 10);

	/* Return */
	return high << 4 | low;
}

HAL_StatusTypeDef DS3231_GetDateTime(volatile DS3231_Time_t *time) {
	uint8_t data[7];
	uint8_t data2[7];

	uint8_t check_data = 0;

	int cnt = 0;

	if (HAL_I2C_IsDeviceReady(&DS3231_i2c_port, DS3231_I2C_ADDR, 2, 5)
			!= HAL_OK) {
		UsrLog("DS3231 GetDateTime: Device is not Ready; ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	}

	while (check_data == 0) {
		/* Read multi bytes */
		if (DS3231_send(&DS3231_i2c_port, DS3231_I2C_ADDR, DS3231_SECONDS, 1,
				100) != HAL_OK) // HAL_MAX_DELAY
				{
			UsrLog("DS3231 DS3231_send 1 ERROR");
			DS3231_init(&DS3231_i2c_port);
			return HAL_ERROR;
		}

		if (DS3231_receive(&DS3231_i2c_port, DS3231_I2C_ADDR, data, 7, 100)
				!= HAL_OK) {
			UsrLog("DS3231 DS3231_receive 1 ERROR");
			DS3231_init(&DS3231_i2c_port);
			return HAL_ERROR;
		}

		/* Read multi bytes */
		if (DS3231_send(&DS3231_i2c_port, DS3231_I2C_ADDR, DS3231_SECONDS, 1,
				100) != HAL_OK) {
			UsrLog("DS3231 DS3231_send 2 ERROR");
			DS3231_init(&DS3231_i2c_port);
			return HAL_ERROR;
		}

		if (DS3231_receive(&DS3231_i2c_port, DS3231_I2C_ADDR, data2, 7, 100)
				!= HAL_OK) {
			UsrLog("DS3231 DS3231_receive 2 ERROR");
			DS3231_init(&DS3231_i2c_port);
			return HAL_ERROR;
		}

		check_data = 1;

		for (int i = 0; i < 7; i++) {
			if (data[i] != data2[i]) {
				check_data = false;
			}
		}

		if (cnt < 99) {
			cnt++;
		} else {
			break;
		}
	}

	/* Fill data */
	time->seconds = DS3231_Bcd2Bin(data[DS3231_SECONDS]);
	time->minutes = DS3231_Bcd2Bin(data[DS3231_MINUTES]);
	time->hours = DS3231_Bcd2Bin(data[DS3231_HOURS]);
	time->day = DS3231_Bcd2Bin(data[DS3231_DAY]);
	time->date = DS3231_Bcd2Bin(data[DS3231_DATE]);
	time->month = DS3231_Bcd2Bin(data[DS3231_MONTH]);
	time->year = DS3231_Bcd2Bin(data[DS3231_YEAR]);

	if ((time->day < 1) | (time->day > 7) | (time->date < 1) | (time->date > 31)
			| (time->month < 1) | (time->month > 12) | (time->hours < 0)
			| (time->hours > 23) | (time->minutes < 0) | (time->minutes > 59)
			| (time->seconds < 0) | (time->seconds > 59)) {
		DS3231_Time_t time2;

		time2.seconds = 0;
		time2.minutes = 0;
		time2.hours = 0;
		time2.day = 1;
		time2.date = 1;
		time2.month = 1;
		time2.year = 0;

		DS3231_SetDateTime(&time2);

		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef DS3231_SetDateTime(DS3231_Time_t *time) {
	if (HAL_I2C_IsDeviceReady(&DS3231_i2c_port, DS3231_I2C_ADDR, 2, 5)
			!= HAL_OK) {
		UsrLog("DS3231_SetDateTime Device is not Ready ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	}

	uint8_t data[8] = {
	DS3231_SECONDS, DS3231_Bin2Bcd(time->seconds), DS3231_Bin2Bcd(
			time->minutes), DS3231_Bin2Bcd(time->hours), DS3231_Bin2Bcd(
			time->day), DS3231_Bin2Bcd(time->date), DS3231_Bin2Bcd(time->month),
			DS3231_Bin2Bcd(time->year), };

	/* Write to device */

	if (DS3231_send(&DS3231_i2c_port, DS3231_I2C_ADDR, data, 8, 100)
			!= HAL_OK) {
		UsrLog("DS3231 DS3231_send 1 ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	}

	DS3231_Time_t time2;

	/* Read to device */

	if (DS3231_GetDateTime(&time2) != HAL_OK) {
		UsrLog("DS3231 GetDateTime 1 ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	} else {
		if ((time->seconds == time2.seconds) & (time->minutes == time2.minutes)
				& (time->hours == time2.hours) & (time->year == time2.year)
				& (time->month == time2.month) & (time->date == time2.date)
				& (time->day == time2.day)) {
			return HAL_OK;
		} else {
			return HAL_ERROR;
		}
	}
}

// Write a single byte to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Returns the I2C status (zero if successful).
HAL_StatusTypeDef writeRTC(uint8_t addr, uint8_t value) {
	uint8_t data[] = { addr, value };

	if (DS3231_send(&DS3231_i2c_port, DS3231_I2C_ADDR, data, 2, 100)
			!= HAL_OK) {
		UsrLog("DS3231 writeRTC DS3231_send 1 ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	}

	return HAL_OK;
}

// Read a single byte from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
HAL_StatusTypeDef readRTC(uint8_t addr, uint8_t *value) {
	uint8_t data[] = { addr };
	if (DS3231_send(&DS3231_i2c_port, DS3231_I2C_ADDR, data, 1, 100)
			!= HAL_OK) {
		UsrLog("DS3231 writeRTC DS3231_send 1 ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	}

	uint8_t b[1];
	if (DS3231_receive(&DS3231_i2c_port, DS3231_I2C_ADDR, b, 1, 100)
			!= HAL_OK) {
		UsrLog("DS3231 writeRTC DS3231_receive 1 ERROR");
		DS3231_init(&DS3231_i2c_port);
		return HAL_ERROR;
	}

	*value = b[0];

	return HAL_OK;
}

// Set an alarm time. Sets the alarm registers only.  To cause the
// INT pin to be asserted on alarm match, use alarmInterrupt().
// This method can set either Alarm 1 or Alarm 2, depending on the
// value of alarmType (use a value from the ALARM_TYPES_t enumeration).
// When setting Alarm 2, the seconds value must be supplied but is
// ignored, recommend using zero. (Alarm 2 has no seconds register.)
HAL_StatusTypeDef DS3231_SetAlarmSec(ALARM_TYPES_t alarmType, uint8_t seconds,
		uint8_t minutes, uint8_t hours, uint8_t daydate) {
	uint8_t addr = 0;
	uint8_t result = 0;
	uint8_t value;

	seconds = DS3231_Bin2Bcd(seconds);
	minutes = DS3231_Bin2Bcd(minutes);
	hours = DS3231_Bin2Bcd(hours);
	daydate = DS3231_Bin2Bcd(daydate);

	if (alarmType & 0x01)
		seconds |= _BV(A1M1);
	if (alarmType & 0x02)
		minutes |= _BV(A1M2);
	if (alarmType & 0x04)
		hours |= _BV(A1M3);
	if (alarmType & 0x08)
		daydate |= _BV(A1M4);
	if (alarmType & 0x10)
		daydate |= _BV(DYDT);

	if (!(alarmType & 0x80))  // alarm 1
	{
		addr = ALM1_SECONDS;
		if (writeRTC(addr++, seconds) != HAL_OK) {
			UsrLog("DS3231_SetAlarmSec writeRTC 1 ERROR");
			return HAL_ERROR;
		}

		if (readRTC(addr - 1, &value) != HAL_OK) {
			UsrLog("DS3231_SetAlarmSec readRTC 1 ERROR");
			return HAL_ERROR;
		}

		if (value != seconds) {
			result++;
		}
	} else {
		addr = ALM2_MINUTES;
	}

	if (writeRTC(addr++, minutes) != HAL_OK) {
		UsrLog("DS3231_SetAlarmSec writeRTC 2 ERROR");
		return HAL_ERROR;
	}

	if (readRTC(addr - 1, &value) != HAL_OK) {
		UsrLog("DS3231_SetAlarmSec readRTC 2 ERROR");
		return HAL_ERROR;
	}

	if (value != minutes) {
		result++;
	}

	if (writeRTC(addr++, hours) != HAL_OK) {
		UsrLog("DS3231_SetAlarmSec writeRTC 3 ERROR");
		return HAL_ERROR;
	}

	if (readRTC(addr - 1, &value) != HAL_OK) {
		UsrLog("DS3231_SetAlarmSec readRTC 3 ERROR");
		return HAL_ERROR;
	}

	if (value != hours) {
		result++;
	}

	if (writeRTC(addr++, daydate) != HAL_OK) {
		UsrLog("DS3231_SetAlarmSec writeRTC 4 ERROR");
		return HAL_ERROR;
	}

	if (readRTC(addr - 1, &value) != HAL_OK) {
		UsrLog("DS3231_SetAlarmSec readRTC 4 ERROR");
		return HAL_ERROR;
	}

	if (value != daydate) {
		UsrLog("DS3231_SetAlarmSec comparation ERROR");
		result++;
	}

	if (result != 0) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

// Set an alarm time. Sets the alarm registers only. To cause the
// INT pin to be asserted on alarm match, use alarmInterrupt().
// This method can set either Alarm 1 or Alarm 2, depending on the
// value of alarmType (use a value from the ALARM_TYPES_t enumeration).
// However, when using this method to set Alarm 1, the seconds value
// is set to zero. (Alarm 2 has no seconds register.)
HAL_StatusTypeDef DS3231_SetAlarm(ALARM_TYPES_t alarmType, uint8_t minutes,
		uint8_t hours, uint8_t daydate) {
	return DS3231_SetAlarmSec(alarmType, 0, minutes, hours, daydate);
}

// Enable or disable an alarm "interrupt" which asserts the INT pin
// on the RTC.
HAL_StatusTypeDef DS3231_AlarmInterrupt(uint8_t alarmNumber,
		uint8_t interruptEnabled) {
	uint8_t controlReg[1], mask = 0, value[1];

	if (readRTC(RTC_CONTROL, controlReg) != HAL_OK) {
		UsrLog("DS3231_AlarmInterrupt RTC_CONTROL 1 ERROR");
		return HAL_ERROR;
	}

	mask = _BV(A1IE) << (alarmNumber - 1);

	if (interruptEnabled == 1) {
		controlReg[0] |= mask;
	} else {
		controlReg[0] &= ~mask;
	}

	if (writeRTC(RTC_CONTROL, controlReg[0]) != HAL_OK) {
		UsrLog("DS3231_AlarmInterrupt writeRTC 1 ERROR");
		return HAL_ERROR;
	}

	if (readRTC(RTC_CONTROL, value) != HAL_OK) {
		UsrLog("DS3231_AlarmInterrupt readRTC 2 ERROR");
		return HAL_ERROR;
	}

	if (controlReg[0] == value[0]) {
		return HAL_OK;
	} else {
		UsrLog("DS3231_AlarmInterrupt comparation ERROR");
		return HAL_ERROR;
	}
}

// Returns true or false depending on whether the given alarm has been
// triggered, and resets the alarm flag bit.
HAL_StatusTypeDef DS3231_Alarm(uint8_t alarmNumber, uint8_t *alarmFlagBit) {
	uint8_t statusReg[1], mask = 0, value[1];

	if (readRTC(RTC_STATUS, statusReg) != HAL_OK) {
		UsrLog("DS3231_Alarm readRTC 1 ERROR");
		return HAL_ERROR;
	}

	mask = _BV(A1F) << (alarmNumber - 1);
	if (statusReg[0] & mask) {
		statusReg[0] &= ~mask;

		if (writeRTC(RTC_STATUS, statusReg[0]) != HAL_OK) {
			UsrLog("DS3231_Alarm writeRTC 1 ERROR");
			return HAL_ERROR;
		}

		if (readRTC(RTC_STATUS, value) != HAL_OK) {
			UsrLog("DS3231_Alarm readRTC 2 ERROR");
			return HAL_ERROR;
		}

		if (statusReg[0] == value[0]) {
			*alarmFlagBit = 1;
			return HAL_OK;
		} else {
			UsrLog("DS3231_Alarm Comparation ERROR");
			return HAL_ERROR;
		}
	} else {
		*alarmFlagBit = 0;
		return HAL_OK;
	}
}

// Returns the value of the oscillator stop flag (OSF) bit in the
// control/status register which indicates that the oscillator is or    *
// was stopped, and that the timekeeping data may be invalid.
// Optionally clears the OSF bit depending on the argument passed.
HAL_StatusTypeDef DS3231_OscStopped(uint8_t clearOSF, uint8_t *OscStopped) {
	uint8_t statusReg[1], value[1];

	if (readRTC(RTC_STATUS, statusReg) != HAL_OK) {
		UsrLog("DS3231_OscStopped readRTC 1 ERROR");
		return HAL_ERROR;
	}

	uint8_t ret = statusReg[0] & _BV(OSF); // isolate the osc stop flag to return to caller

	if (ret & (clearOSF == 1)) // clear OSF if it's set and the caller wants to clear it
			{
		statusReg[0] &= ~_BV(OSF);

		if (writeRTC(RTC_STATUS, statusReg[0]) != HAL_OK) {
			UsrLog("DS3231_OscStopped writeRTC 1 ERROR");
			return HAL_ERROR;
		}

		if (readRTC(RTC_STATUS, value) != HAL_OK) {
			UsrLog("DS3231_OscStopped readRTC 2 ERROR");
			return HAL_ERROR;
		}

		if (statusReg[0] != value[0]) {
			UsrLog("DS3231_OscStopped Comparation ERROR");
			return HAL_ERROR;
		}
	}

	*OscStopped = ret;

	return HAL_OK;
}

// Enable or disable the square wave output.
// Use a value from the SQWAVE_FREQS_t enumeration for the parameter.
HAL_StatusTypeDef DS3231_SquareWave(SQWAVE_FREQS_t freq) {
	uint8_t controlReg[1], value[1];

	if (readRTC(RTC_CONTROL, controlReg) != HAL_OK) {
		UsrLog("DS3231_SquareWave readRTC 1 ERROR");
		return HAL_ERROR;
	}

	if (freq >= SQWAVE_NONE) {
		controlReg[0] |= _BV(INTCN);
	} else {
		controlReg[0] = (controlReg[0] & 0xE3) | (freq << RS1);
	}

	if (writeRTC(RTC_CONTROL, controlReg[0]) != HAL_OK) {
		UsrLog("DS3231_SquareWave writeRTC 1 ERROR");
		return HAL_ERROR;
	}

	if (readRTC(RTC_CONTROL, value) != HAL_OK) {
		UsrLog("DS3231_SquareWave readRTC 2 ERROR");
		return HAL_ERROR;
	}

	if (controlReg[0] == value[0]) {
		return HAL_OK;
	} else {
		UsrLog("DS3231_SquareWave Comparation ERROR");
		return HAL_ERROR;
	}
}
