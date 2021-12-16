// Source: https://embdev.net/attachment/391825/STM32_CunbeMX_DS18B20.pdf
// Source CRC: https://forum.arduino.cc/index.php?topic=557347.0

/*
 * GPL v3
 */

/*
 * DS18B20.h
 *
 *  Created on: Jan 30, 2020
 *      Author: UserZ
 */

#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_

#include "main.h"
#include "dwt_stm32_delay.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
#define  UsrLog(...)   do { \
                            printf(__VA_ARGS__); \
                            printf("\n"); \
} while (0)
*/

//#define using_timer7 1

#ifdef using_timer7
extern TIM_HandleTypeDef htim7;
#endif


typedef  struct  _OneWire_Channel
{
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
}
OneWire_ChannelTypeDef;

typedef  struct  _DS18B20_
{
	OneWire_ChannelTypeDef CHANNEL;
	uint8_t SERIAL_ID[8];
	float TEMPERATURE;
	uint8_t FLAG_PRESENT;
	uint8_t FLAG_ENABLED;
	uint8_t FLAG_TIMEOUT;
	uint8_t FLAG_UNIT;
	uint8_t FLAG_POWER_FAIL;
	uint8_t RESOLUTION_BITS;
}
DS18B20_TypeDef;

typedef enum {
	DS18B20_12_bits = 12,
	DS18B20_11_bits = 11,
	DS18B20_10_bits = 10,
	DS18B20_9_bits = 9
}DS18B20_ResolutionBitsTypeDef;

typedef  struct  _ScratchPad
{
	uint8_t TEMP_LSB;
	uint8_t TEMP_MSB;
	uint8_t HIGH_ALARM_TEMP;
	uint8_t LOW_ALARM_TEMP;
	uint8_t CONFIGURATION;
	uint8_t INTERNAL_BYTE;
	uint8_t COUNT_REMAIN;
	uint8_t COUNT_PER_C;
	uint8_t SCRATCHPAD_CRC;
}
ScratchPadTypeDef;

typedef enum {
	TEMP_LSB = 0,
	TEMP_MSB,
	HIGH_ALARM_TEMP,
	LOW_ALARM_TEMP,
	CONFIGURATION,
	INTERNAL_BYTE,
	COUNT_REMAIN,
	COUNT_PER_C,
	SCRATCHPAD_CRC
}ScratchpadEnumTypeDef;

typedef enum {
	DS18B20_SLEEP_INTERVAL = 0,
	DS18B20_START_CONVERSION,
	DS18B20_READ,
	DS18B20_START_CONVERSION_NO_IRQ
} DS18B20readEnumTypeDef;

#define DS18B20_9_BIT  0x1F //  9 bit
#define DS18B20_10_BIT 0x3F // 10 bit
#define DS18B20_11_BIT 0x5F // 11 bit
#define DS18B20_12_BIT 0x7F // 12 bit

void DS18B20_delayus(uint32_t delay);
void DS18B20_LH_signal(uint32_t L_time, uint32_t H_time, OneWire_ChannelTypeDef channel);
void DS18B20_write_bit(uint8_t bit, OneWire_ChannelTypeDef channel);
void DS18B20_write_byte(uint8_t data, OneWire_ChannelTypeDef channel);
uint8_t DS18B20_read_bit(OneWire_ChannelTypeDef channel);
uint8_t DS18B20_read_byte(OneWire_ChannelTypeDef channel);
uint8_t DS18B20_reset(OneWire_ChannelTypeDef channel);
void DS18B20_restart_ALL(DS18B20_TypeDef* SENSORS);
//uint8_t DS18B20_get_presence(OneWire_ChannelTypeDef channel);
//uint8_t DS18B20_start_temperature_conversion(OneWire_ChannelTypeDef channel);
float DS18B20_get_temperature(OneWire_ChannelTypeDef channel);
//float DS18B20_get_temperature_no_wait(OneWire_ChannelTypeDef channel);

HAL_StatusTypeDef DS18B20_readSensors(int mode, DS18B20_TypeDef* SENSORS);
HAL_StatusTypeDef DS18B20_get_presence(DS18B20_TypeDef sensor);
HAL_StatusTypeDef DS18B20_start_temperature_conversion(DS18B20_TypeDef sensor);
HAL_StatusTypeDef DS18B20_start_conversion(DS18B20_TypeDef* SENSORS, uint8_t NumOfSensors);
HAL_StatusTypeDef DS18B20_read_all(DS18B20_TypeDef* SENSORS, uint8_t NumOfSensors);
HAL_StatusTypeDef DS18B20_get_temperature_no_wait(DS18B20_TypeDef *sensor);

HAL_StatusTypeDef DS18B20_get_resolution(uint8_t *resolution, OneWire_ChannelTypeDef channel);
//uint8_t DS18B20_get_resolution(OneWire_ChannelTypeDef channel, uint8_t *data_res);

uint8_t foo1(uint8_t *data_res);
HAL_StatusTypeDef foo2(uint8_t* data, uint8_t channel);

HAL_StatusTypeDef DS18B20_write_scratchpad(ScratchPadTypeDef* pad_data, OneWire_ChannelTypeDef channel);
HAL_StatusTypeDef DS18B20_set_resolution(uint8_t res, OneWire_ChannelTypeDef channel);
HAL_StatusTypeDef DS18B20_get_scratchpad(uint8_t* pad_data, OneWire_ChannelTypeDef channel);

HAL_StatusTypeDef DS18B20_get_ID(uint8_t* id_data, OneWire_ChannelTypeDef channel);
void DS18B20_hex_id(uint8_t* sensor_ID, char* str);

//HAL_StatusTypeDef DS18B20_reset(DS18B20_TypeDef sensor);

uint8_t DS18B20_send_ID(uint8_t* id_data, OneWire_ChannelTypeDef channel);
float DS18B20_get_temperature_with_ID(uint8_t* id_data, OneWire_ChannelTypeDef channel);
uint8_t DS18B20_CRC8(const uint8_t *addr, uint8_t len);

#endif /* INC_DS18B20_H_ */
