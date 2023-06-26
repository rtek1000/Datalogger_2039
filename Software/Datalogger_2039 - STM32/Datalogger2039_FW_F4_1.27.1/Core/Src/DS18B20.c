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

#include "DS18B20.h"
#include "main.h"
#include "num_base_64.h"

extern int16_t sensor_offset[maxChannel];

extern uint8_t DS18B20_resolution_set;

extern uint8_t init_flag;

extern uint8_t HC595_buff;

extern const uint8_t Num_Of_Sensors;
extern DS18B20_TypeDef SENSORS[];

extern TIM_HandleTypeDef htim7;

extern uint8_t temperature_unit;

static void DS18B20_IO_as_INPUT(OneWire_ChannelTypeDef channel);
static void DS18B20_IO_as_OUTPUT(OneWire_ChannelTypeDef channel);
static void DS18B20_IO_wait_for_1(uint32_t time, OneWire_ChannelTypeDef channel);
static uint8_t DS18B20_IO_check_for_1(uint32_t time,
		OneWire_ChannelTypeDef channel);

extern void HC595_init(void);
extern void HC595_set(uint8_t value);
extern void beep(uint8_t repetions);

HAL_StatusTypeDef DS18B20_init(void) {
	HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_SET);

	//IWDG_delay_ms(5);

	HC595_init();

	return DS18B20_power_channel();
}

HAL_StatusTypeDef DS18B20_power_channel(void) {
	uint8_t fail = 0;

	for (uint8_t i = 0; i < Num_Of_Sensors; i++) {
		if (SENSORS[i].FLAG_ENABLED) {
			HC595_set(1 << i);

			// wait +/-50us:
			for (int j = 0; j < 140; j++) {
				IWDG_delay_ms(0);
			}

			DS18B20_IO_as_INPUT(SENSORS[i].CHANNEL);

			if (HAL_GPIO_ReadPin(SENSORS[i].CHANNEL.GPIOx,
					SENSORS[i].CHANNEL.GPIO_Pin)) {
				HC595_buff |= 1 << i;
				SENSORS[i].FLAG_POWER_FAIL = 0;
			} else {
				HC595_buff &= ~(1 << i);
				SENSORS[i].FLAG_POWER_FAIL = 1;
				fail++;
			}
		} else {
			SENSORS[i].FLAG_POWER_FAIL = 0;
		}
	}

	HC595_set(HC595_buff);

	if (fail != 0) {
		beep(1);
		return HAL_ERROR;
	} else {
		return HAL_OK;
	}
}

static void DS18B20_IO_as_INPUT(OneWire_ChannelTypeDef channel) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	HAL_GPIO_WritePin(channel.GPIOx, channel.GPIO_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = channel.GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(channel.GPIOx, &GPIO_InitStruct);
}

//static void DS18B20_IO_as_HighZ(OneWire_ChannelTypeDef channel)
//{
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//	HAL_GPIO_WritePin(channel.GPIOx, channel.GPIO_Pin, GPIO_PIN_SET);
//	GPIO_InitStruct.Pin = channel.GPIO_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(channel.GPIOx, &GPIO_InitStruct);
//}

static void DS18B20_IO_as_OUTPUT(OneWire_ChannelTypeDef channel) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	HAL_GPIO_WritePin(channel.GPIOx, channel.GPIO_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = channel.GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(channel.GPIOx, &GPIO_InitStruct);
}

static void DS18B20_IO_wait_for_1(uint32_t time, OneWire_ChannelTypeDef channel) {
	DS18B20_IO_as_INPUT(channel);
	DS18B20_delayus(time);
	while (HAL_GPIO_ReadPin(channel.GPIOx, channel.GPIO_Pin) == 0)
		;
	DS18B20_IO_as_OUTPUT(channel);
}

/*
 * Returns 1 for ready bus; returns 0 for busy bus (OK after start conversion)
 */
static uint8_t DS18B20_IO_check_for_1(uint32_t time,
		OneWire_ChannelTypeDef channel) {
	//	uint8_t result = 0;
	//	IO_as_INPUT(channel);
	//	delayus(time);
	//	result = HAL_GPIO_ReadPin(channel.GPIOx, channel.GPIO_Pin);
	//	IO_as_OUTPUT(channel);
	//	return result;
	return DS18B20_read_byte(channel);
}

void DS18B20_delayus(uint32_t us) {
	DWT_Delay_us(us);

	//	volatile uint32_t counter = 8*us;
	//	while(counter--);
}

void DS18B20_LH_signal(uint32_t L_time, uint32_t H_time,
		OneWire_ChannelTypeDef channel) {
	HAL_GPIO_WritePin(channel.GPIOx, channel.GPIO_Pin, GPIO_PIN_RESET);
	DS18B20_delayus(L_time);
	//From pullup_HIGH to GND_LOW:---___
	HAL_GPIO_WritePin(channel.GPIOx, channel.GPIO_Pin, GPIO_PIN_SET);
	DS18B20_delayus(H_time);
	//From GND_LOW to pullup_HIGH:___---
}

void DS18B20_write_bit(uint8_t bit, OneWire_ChannelTypeDef channel) {
	if (bit == 0)
		DS18B20_LH_signal(60, 5, channel);
	else
		DS18B20_LH_signal(5, 60, channel);
}

void DS18B20_write_byte(uint8_t data, OneWire_ChannelTypeDef channel) {
	for (uint8_t i = 0; i < 8; i++) {
		DS18B20_write_bit(data >> i & 1, channel);
	}
}

uint8_t DS18B20_read_bit(OneWire_ChannelTypeDef channel) {
	uint8_t bit = 0;
	DS18B20_LH_signal(1, 10, channel);
	DS18B20_IO_as_INPUT(channel);
	bit = (HAL_GPIO_ReadPin(channel.GPIOx, channel.GPIO_Pin) ? 1 : 0);
	DS18B20_delayus(40);
	DS18B20_IO_as_OUTPUT(channel);
	return bit;
}

uint8_t DS18B20_read_byte(OneWire_ChannelTypeDef channel) {
	uint8_t data = 0;
	for (uint8_t i = 0; i < 8; i++) {
		data += DS18B20_read_bit(channel) << i;
	}
	return data;
}

//HAL_StatusTypeDef DS18B20_reset(DS18B20_TypeDef sensor)
//{
//	uint8_t bit = 0;
//
//	DS18B20_LH_signal(500, 70, sensor.CHANNEL);
//	DS18B20_IO_as_INPUT(sensor.CHANNEL);
//	bit = (HAL_GPIO_ReadPin(sensor.CHANNEL.GPIOx, sensor.CHANNEL.GPIO_Pin) ? 0 : 1); //not ?1:0
//	DS18B20_IO_as_OUTPUT(sensor.CHANNEL);
//	DS18B20_delayus(400);
//	//	return bit;
//
//	if(bit == 1) return HAL_OK;
//	else return HAL_ERROR;
//}

///*
// * Returns 0 for device not found; returns 1 for present device
// */
uint8_t DS18B20_reset(OneWire_ChannelTypeDef channel) {
	uint8_t bit = 0;

	DS18B20_LH_signal(500, 70, channel);
	DS18B20_IO_as_INPUT(channel);
	bit = (HAL_GPIO_ReadPin(channel.GPIOx, channel.GPIO_Pin) ? 0 : 1); //not ?1:0
	DS18B20_IO_as_OUTPUT(channel);
	DS18B20_delayus(400);
	return bit;
}

void DS18B20_restart_ALL(DS18B20_TypeDef *SENSORS) {
	for (uint8_t i = 0; i < maxChannel; i++) {
		DS18B20_IO_as_OUTPUT(SENSORS[i].CHANNEL);
		HAL_GPIO_WritePin(SENSORS[i].CHANNEL.GPIOx, SENSORS[i].CHANNEL.GPIO_Pin,
				GPIO_PIN_RESET);
	}

	IWDG_delay_ms(50);

	for (uint8_t i = 0; i < maxChannel; i++) {
		DS18B20_IO_as_INPUT(SENSORS[i].CHANNEL);
	}
}

/*
 * Returns 0 for device not found; returns 1 for present device
 */

HAL_StatusTypeDef DS18B20_get_presence(DS18B20_TypeDef sensor) {
	if (DS18B20_reset(sensor.CHANNEL) == 1)
		return HAL_OK;
	else
		return HAL_ERROR;
}

//uint8_t DS18B20_get_presence(OneWire_ChannelTypeDef channel)
//{
//	return DS18B20_reset(channel);
//}

HAL_StatusTypeDef DS18B20_start_temperature_conversion(DS18B20_TypeDef sensor) {
	//uint8_t pad_data[] = {0,0,0,0,0,0,0,0,0}; //9 Byte
	if (DS18B20_reset(sensor.CHANNEL) != 1)
		return HAL_ERROR;

	DS18B20_write_byte(0xCC, sensor.CHANNEL); //Skip ROM [CCh]
	DS18B20_write_byte(0x44, sensor.CHANNEL); //Convert Temperature [44h]

	if (DS18B20_IO_check_for_1(20, sensor.CHANNEL) == 0)
		return HAL_OK;
	else
		return HAL_ERROR;
}

///*
// * Returns 1 for ready bus; returns 0 for bus busy or error.
// */
//uint8_t DS18B20_start_temperature_conversion(OneWire_ChannelTypeDef channel)
//{
//	//uint8_t pad_data[] = {0,0,0,0,0,0,0,0,0}; //9 Byte
//	if(DS18B20_reset(channel) == 0)
//	{
//		return 0;
//	}
//
//	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]
//	DS18B20_write_byte(0x44, channel); //Convert Temperature [44h]
//
//	return DS18B20_IO_check_for_1(20, channel);
//}

float DS18B20_bin_to_float(uint8_t *pad_data) {
	uint16_t x = (pad_data[1] << 8) + pad_data[0];
	if ((pad_data[1] >> 7) == 1) {
		x -= 1;
		x = ~x;
		return x / -16.0;
	} else {
		return x / 16.0;
	}
}

void DS18B20_float_to_bin(float data_in, uint8_t *pad_data) {
	uint16_t x;

//	UsrLog("data_in: %04lf", data_in);

	if (data_in >= 0) {

		x = data_in * 16.0;

//		UsrLog("x: %u", x);

	} else {
		x = data_in * -16.0;
		x = ~x;
		x += 1;
	}

	pad_data[0] = x & 0xFF;
	pad_data[1] = (x >> 8) & 0xFF;

}

//fahrenheit = celsius * 1.8 + 32.0;
float DS18B20_celsius_to_fahrenheit(float celsius) {
	return (celsius * 1.8) + 32.0;
}

float DS18B20_get_temperature(OneWire_ChannelTypeDef channel) {
	uint8_t pad_data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //9 Byte
	if (DS18B20_reset(channel) == 0) {
		return -99;
	}
	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]
	DS18B20_write_byte(0x44, channel); //Convert Temperature [44h]
	DS18B20_IO_wait_for_1(20, channel);
	if (DS18B20_reset(channel) == 0) {
		return -99;
	}
	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]
	DS18B20_write_byte(0xBE, channel); //Read Scratchpad [BEh]
	for (uint8_t i = 0; i < 9; i++)
		pad_data[i] = DS18B20_read_byte(channel); //factor out 1/16 and remember 1/16 != 1/16.0
	uint16_t x = (pad_data[1] << 8) + pad_data[0];
	if ((pad_data[1] >> 7) == 1) {
		x -= 1;
		x = ~x;
		return x / -16.0;
	}

	else
		return x / 16.0;
}

HAL_StatusTypeDef DS18B20_get_temperature_no_wait(DS18B20_TypeDef *sensor, float Offset) {
	uint8_t pad_data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //9 Byte

	if (DS18B20_IO_check_for_1(20, sensor->CHANNEL) == 0)
		return HAL_ERROR;

	if (DS18B20_reset(sensor->CHANNEL) == 0)
		return HAL_ERROR; // reset and check presence

	DS18B20_write_byte(0xCC, sensor->CHANNEL); //Skip ROM [CCh]

	DS18B20_write_byte(0xBE, sensor->CHANNEL); //Read Scratchpad [BEh]

	for (uint8_t i = 0; i < 9; i++) {
		pad_data[i] = DS18B20_read_byte(sensor->CHANNEL); //factor out 1/16 and remember 1/16 != 1/16.0
	}

	uint8_t crc1 = DS18B20_CRC8(pad_data, 8);  //calculate crc

	if (crc1 == pad_data[8]) //compare : calculate with received
			{
		//UsrLog("DS18B20 CRC Valid 0x%02X", crc1);

		uint16_t x = (pad_data[1] << 8) + pad_data[0];

		if ((pad_data[1] >> 7) == 1) {
			x -= 1;

			x = ~x;

			sensor->TEMPERATURE = x / -16.0;

			sensor->TEMPERATURE += Offset; // (sensor_offset[sensor->CHANNEL] / 10.0);

			uint16_t x_tmp = sensor->TEMPERATURE * 16.0;

			x_tmp = ~x_tmp;

			x_tmp += 1;

			sensor->TEMPERATURE_RAW = x_tmp;
		} else {
			sensor->TEMPERATURE = x / 16.0;

			sensor->TEMPERATURE += Offset; // (sensor_offset[sensor->CHANNEL] / 10.0);

			sensor->TEMPERATURE_RAW = sensor->TEMPERATURE * 16.0;
		}

		return HAL_OK;
	} else {
		UsrLog("DS18B20 Get Temperature CRC ERROR");
	}

	return HAL_ERROR;
}

//float DS18B20_get_temperature_no_wait(OneWire_ChannelTypeDef channel)
//{
//	uint8_t pad_data[] = {0,0,0,0,0,0,0,0,0}; //9 Byte
//
//	if(DS18B20_IO_check_for_1(20, channel) == 0) return -99;
//
//	if(DS18B20_reset(channel) == 0) return -99; // reset and check presence
//
//	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]
//
//	DS18B20_write_byte(0xBE, channel); //Read Scratchpad [BEh]
//
//	for (uint8_t i = 0; i < 9; i++)
//	{
//		pad_data[i] = DS18B20_read_byte(channel); //factor out 1/16 and remember 1/16 != 1/16.0
//	}
//
//	uint8_t crc1 = DS18B20_CRC8(pad_data, 8);  //calculate crc
//
//	if (crc1 == pad_data[8]) //compare : calculate with received
//	{
//		//UsrLog("DS18B20 CRC Valid 0x%02X", crc1);
//
//		uint16_t x = (pad_data[1] << 8) + pad_data[0];
//
//		if ((pad_data[1] >> 7) == 1 )
//		{
//			x -= 1;
//
//			x = ~x;
//
//			return x / -16.0;
//		}
//
//		else return x / 16.0;
//	}
//	else
//	{
//		UsrLog("DS18B20 CRC ERROR");
//	}
//
//	return -99;
//}

HAL_StatusTypeDef DS18B20_get_scratchpad(uint8_t *data,
		OneWire_ChannelTypeDef channel) {
	//	uint8_t data[] = {0,0,0,0,0,0,0,0,0}; //9 Byte
	if (DS18B20_reset(channel) == 0)
		return HAL_ERROR; // reset and check presence

	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]

	DS18B20_write_byte(0xBE, channel); //Read Scratchpad [BEh]

	for (uint8_t i = 0; i < 9; i++) {
		data[i] = DS18B20_read_byte(channel); //factor out 1/16 and remember 1/16 != 1/16.0
	}

	if (DS18B20_reset(channel) == 0)
		return HAL_ERROR; // reset and check presence

	return HAL_OK;
}

HAL_StatusTypeDef DS18B20_write_scratchpad(ScratchPadTypeDef *pad_data,
		OneWire_ChannelTypeDef channel) {
	//uint8_t pad_data[] = {0,0,0}; //3 Byte
	if (DS18B20_reset(channel) == 0)
		return HAL_ERROR; // reset and check presence

	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]

	DS18B20_write_byte(0x4E, channel); //Write Scratchpad [4Eh]

	DS18B20_write_byte(pad_data->HIGH_ALARM_TEMP, channel); //Write Scratchpad

	DS18B20_write_byte(pad_data->LOW_ALARM_TEMP, channel); //Write Scratchpad

	DS18B20_write_byte(pad_data->CONFIGURATION, channel); //Write Scratchpad

	if (DS18B20_reset(channel) == 0)
		return HAL_ERROR; // reset and check presence

	DS18B20_write_byte(0xCC, channel); //Skip ROM [CCh]

	DS18B20_write_byte(0x48, channel); //Copy Scratchpad [48h]

	IWDG_delay_ms(20);

	return HAL_OK;
}

//uint8_t DS18B20_get_resolution(OneWire_ChannelTypeDef channel, uint8_t *data_res)
//{
//	uint8_t data[8];
//	data[CONFIGURATION] = 0;
//
//	DS18B20_get_scratchpad(data, channel);
//
////	if(DS18B20_get_scratchpad(data, channel) != HAL_OK)
////	{
////		return -99;
////	}
//	//HAL_Delay(0);
//
//	*data_res = 9;
//
//	//
//	//	//	return data[CONFIGURATION];
//	//
//	//	if(data[CONFIGURATION] == (uint8_t)DS18B20_9_BIT)
//	//	{
//	//		data_res[0] = 9;
//	//		return 9;
//	//	}
//	//	else if(data[CONFIGURATION] == (uint8_t)DS18B20_10_BIT)
//	//	{
//	//		data_res[0] = 10;
//	//		return 10;
//	//	}
//	//	else if(data[CONFIGURATION] == (uint8_t)DS18B20_11_BIT)
//	//	{
//	//		data_res[0] = 11;
//	//		return 11;
//	//	}
//	//	else if(data[CONFIGURATION] == (uint8_t)DS18B20_12_BIT)
//	//	{
//	//		data_res[0] = 12;
//	//		return 12;
//	//	}
//
//	return data[CONFIGURATION]; // error
//}

HAL_StatusTypeDef DS18B20_get_resolution(uint8_t *resolution,
		OneWire_ChannelTypeDef channel) {

	uint8_t data[9];  //Read Scratchpad 9 bytes
	data[CONFIGURATION] = 0;

	if (DS18B20_get_scratchpad(data, channel) != HAL_OK) {
		return HAL_ERROR;
	}

	if (data[CONFIGURATION] == (uint8_t) DS18B20_9_BIT) {
		*resolution = 9;
	} else if (data[CONFIGURATION] == (uint8_t) DS18B20_10_BIT) {
		*resolution = 10;
	} else if (data[CONFIGURATION] == (uint8_t) DS18B20_11_BIT) {
		*resolution = 11;
	} else if (data[CONFIGURATION] == (uint8_t) DS18B20_12_BIT) {
		*resolution = 12;
	} else {
		*resolution = 0;

		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef DS18B20_set_resolution(uint8_t res,
		OneWire_ChannelTypeDef channel) {
	int8_t config_data = 0;
	uint8_t resolution_compare = 0;

	if (DS18B20_get_resolution(&resolution_compare, channel) != HAL_OK) {
		//UsrLog("DS18B20_get_resolution fail");
		return HAL_ERROR;
	}

	if (res == resolution_compare) {
		return HAL_OK; // do not need change, return
	}

	if (res == 9)
		config_data = DS18B20_9_BIT;
	else if (res == 10)
		config_data = DS18B20_10_BIT;
	else if (res == 11)
		config_data = DS18B20_11_BIT;
	else if (res == 12)
		config_data = DS18B20_12_BIT;
	else
		config_data = DS18B20_9_BIT;

	ScratchPadTypeDef pad_data;
	pad_data.CONFIGURATION = config_data;
	if (DS18B20_write_scratchpad(&pad_data, channel) != HAL_OK) {
		//UsrLog("DS18B20_write_scratchpad fail");
		return HAL_ERROR;
	}

	if (DS18B20_get_resolution(&resolution_compare, channel) != HAL_OK) {
		//UsrLog("DS18B20_get_resolution fail");
		return HAL_ERROR;
	}

	if (res != resolution_compare) {
		return HAL_ERROR;
	}

	else
		return HAL_OK;
}

void DS18B20_hex_id(uint8_t *sensor_ID, char *str_buff) {
	char str_tmp[18];
	char customer[6];
	char serial[9];
	char family[4];

	// Without CRC:
	sprintf(str_tmp, "%02X%02X%02X%02X%02X%02X%02X", (uint8_t) sensor_ID[1],
			(uint8_t) sensor_ID[2], (uint8_t) sensor_ID[3],
			(uint8_t) sensor_ID[4], (uint8_t) sensor_ID[5],
			(uint8_t) sensor_ID[6], (uint8_t) sensor_ID[7]);

//	0xCC00000SSSSSSSFF
//	where CC is the CRC
//	00000 is the customer code
//	SSSSSSS is the serial number
//	FF is the family code
//
//  00000-SSSSSSS-FF

	for (int i = 0; i < 5; i++) {
		customer[i] = str_tmp[i];
	}

	customer[5] = 0;

	for (int i = 5; i < 12; i++) {
		serial[i - 5] = str_tmp[i];
	}

	serial[7] = 0;

	for (int i = 12; i < 14; i++) {
		family[i - 12] = str_tmp[i];
	}

	family[3] = 0;

	sprintf(str_buff, "%s%s%s", customer, serial, family);

	str_buff[14] = 0;
}

void DS18B20_b64_id(uint8_t *sensor_ID, char *str_buff) {
	int64_t sens_ID = 0;

	// Without CRC:

	sens_ID = sensor_ID[7];
	sens_ID |= sensor_ID[6] << 8;
	sens_ID |= sensor_ID[5] << 16;
	sens_ID |= sensor_ID[4] << 24;
	sens_ID += sensor_ID[3] * 4294967296;
	sens_ID += sensor_ID[2] * 1099511627776;
	sens_ID += sensor_ID[1] * 281474976710656;

	conv_b10_b64(sens_ID, str_buff);

//	0xCC00000SSSSSSSFF
//	where CC is the CRC
//	00000 is the customer code
//	SSSSSSS is the serial number
//	FF is the family code
//
//  00000-SSSSSSS-FF

}

/*
 * DS18B20 read 8 channels
 * mode: 0 = sleep; wake up by timer
 * mode: 1 = start conversion
 * mode: 2 = read temperature
 * returns: 0 = ok
 * returns: 1 = fail
 */
HAL_StatusTypeDef DS18B20_readSensors(int mode, DS18B20_TypeDef *SENSORS) {
	HAL_StatusTypeDef result = HAL_OK;
	uint8_t fail = 0;

	// HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_SET); // sensors on

	//	if((mode != DS18B20_SLEEP_INTERVAL) && (mode != DS18B20_START_CONVERSION_NO_IRQ)) // need disable all interrupts to run OneWire functions
	//	{
	//HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

	if (init_flag == 1) {
		HAL_TIM_Base_Stop_IT(&htim7);
	}
	//	}

	if ((mode == DS18B20_START_CONVERSION)
			| (mode == DS18B20_START_CONVERSION_NO_IRQ)) {

		for (uint8_t i = 0; i < maxChannel; i++) {
			if (DS18B20_get_presence(SENSORS[i]) == HAL_OK) {
				DS18B20_get_resolution(&SENSORS[i].RESOLUTION_BITS,
						SENSORS[i].CHANNEL);

				DS18B20_get_resolution(&SENSORS[i].RESOLUTION_BITS,
						SENSORS[i].CHANNEL);

				if (SENSORS[i].RESOLUTION_BITS != DS18B20_resolution_set) {
					if (SENSORS[i].RESOLUTION_BITS == 255)
						UsrLog("ERROR CH%d device not found", i);

					uint8_t j = 0;

					while (SENSORS[i].RESOLUTION_BITS != DS18B20_resolution_set) {
						DS18B20_set_resolution(DS18B20_resolution_set,
								SENSORS[i].CHANNEL);

						if (j < 9) {
							j++;
						} else {
							UsrLog("ERROR CH%d resolution = %d", i + 1,
									SENSORS[i].RESOLUTION_BITS);
							break;
						}

						IWDG_delay_ms(0);
					}
				}
			}
		}

		for (int i = 0; i < maxChannel; i++) {
			uint8_t status = HAL_BUSY;
			int j = 0;
			// SENSORS[i].FLAG_TIMEOUT = 0;
//			if (SENSORS[i].FLAG_TIMEOUT != 1) {
			while (status != HAL_OK) {
				DS18B20_get_presence(SENSORS[i]); // wake up sensor

				if (DS18B20_get_presence(SENSORS[i]) == HAL_OK) {
					SENSORS[i].FLAG_PRESENT = 1;

					status = DS18B20_start_temperature_conversion(SENSORS[i]);
				} else {
					//UsrLog("Sensor %d: DS18B20_get_presence ERROR", i + 1);
					SENSORS[i].FLAG_PRESENT = 0;

					if (SENSORS[i].FLAG_ENABLED == 0) {
						status = HAL_OK; // skip this sensor

						break;
					}
				}

				if (j < 9) {
					j++;

					if (SENSORS[i].FLAG_ENABLED == 1) {
						SENSORS[i].FLAG_TIMEOUT = 0;

						SENSORS[i].FLAG_START_CONV_ERROR = 0;
					}
				} else {
					if (SENSORS[i].FLAG_ENABLED == 1) {
						fail = 1;

						status = HAL_TIMEOUT;

						SENSORS[i].FLAG_TIMEOUT = 1;

						SENSORS[i].FLAG_START_CONV_ERROR = 1;

//						UsrLog("ERROR start_temperature_conversion %d", i + 1);

//						beep(1, 1);
					}

					break;
				}
			}

			if (status != HAL_OK) {
//				char str[8];
//				if (status == HAL_TIMEOUT)
//					sprintf(str, "Timeout");
//				if (status == HAL_ERROR)
//					sprintf(str, "Error");
//				if (status == HAL_BUSY)
//					sprintf(str, "Busy");
//				UsrLog("Start_temperature_conversion ch: %d, ERROR status: %s",
//						i + 1, str);
			} else {
				if (j > 1) {
//					UsrLog(
//							"Start_temperature_conversion ch: %d; tentatives: %d; OK",
//							i + 1, j);
				}
			}
//			} else {
//				UsrLog("Sensor %d: DS18B20 ERROR, Timeout", i + 1);
//			}
		}

		if (mode == DS18B20_START_CONVERSION_NO_IRQ) {
			//UsrLog("return HAL_OK: mode = DS18B20_START_CONVERSION_NO_IRQ");
			return HAL_OK;
		}
	}

	if (mode == DS18B20_SLEEP_INTERVAL) // low power mode
			{
		Sleep_wakeup_by_Timer();
	}

	if ((mode == DS18B20_SLEEP_INTERVAL) | (mode == DS18B20_READ)) {
		for (int i = 0; i < maxChannel; i++) {
			uint8_t status = HAL_BUSY;
			int k = 0;

//			if (SENSORS[i].FLAG_TIMEOUT != 1) {
			while (status != HAL_OK) {
				if (SENSORS[i].FLAG_PRESENT == 0) {
					if (SENSORS[i].FLAG_ENABLED == 1) {
						status = HAL_TIMEOUT;
						//SENSORS[i].FLAG_TIMEOUT = 1;
					} else {
						status = HAL_OK; // skip this sensor
					}

					break;
				}

				if (DS18B20_get_presence(SENSORS[i]) == HAL_OK) {
					uint8_t id_data[8];

					if (DS18B20_get_ID(id_data, SENSORS[i].CHANNEL) != HAL_OK) {
						UsrLog("ERROR get_ID: Step: %d", k);

						SENSORS[i].TEMPERATURE_RAW = 0;
						SENSORS[i].TEMPERATURE = 0.0;

						SENSORS[i].FLAG_TIMEOUT = 1;
						SENSORS[i].FLAG_ID_ERROR = 1;

						result = HAL_ERROR;
					} else {
						SENSORS[i].FLAG_ID_ERROR = 0;

						for (uint8_t j = 0; j < 8; j++) {
							SENSORS[i].SERIAL_ID[7 - j] = id_data[j];
						}
						//
						//					UsrLog("ID: %02x %02x %02x %02x %02x %02x %02x %02x", id_data[0],
						//							id_data[1], id_data[2], id_data[3], id_data[4],
						//							id_data[5], id_data[6], id_data[7]);

						status = HAL_OK;
					}
				} else {
					for (uint8_t j = 0; j < 8; j++) {
						SENSORS[i].SERIAL_ID[j] = 0;
					}

					if (SENSORS[i].FLAG_ENABLED == 0) {
						status = HAL_OK;

						break; // skip this sensor
					}
				}

				if (k < 9) {
					k++;
				} else {
					if (SENSORS[i].FLAG_ENABLED == 1) {
						fail = 1;

						UsrLog("ERROR get_ID %d", i + 1);

						SENSORS[i].TEMPERATURE_RAW = 0;
						SENSORS[i].TEMPERATURE = 0.0;

						SENSORS[i].FLAG_TIMEOUT = 1;
						SENSORS[i].FLAG_ID_ERROR = 1;
					}

					break;
				}
			}

			status = HAL_BUSY;
			k = 0;

			while (status != HAL_OK) {
				if (SENSORS[i].FLAG_PRESENT == 0) {
					if (SENSORS[i].FLAG_ENABLED == 1) {
						status = HAL_TIMEOUT;
						SENSORS[i].FLAG_TIMEOUT = 1;
					} else {
						status = HAL_OK; // skip this sensor
					}
					break;
				}

				float Offset = sensor_offset[i] / 10.0;

				if (DS18B20_get_temperature_no_wait(&SENSORS[i], Offset) != HAL_OK) {
					result = HAL_ERROR;
				}

				float temp = SENSORS[i].TEMPERATURE;

				if (SENSORS[i].FLAG_PRESENT == 1) {
//					if ((temp >= 80) | (temp <= 5)) {
//						result = HAL_ERROR;
//						UsrLog(
//								"result HAL_ERROR: ((temp >= 80) | (temp <= 5)); temp: %.4f; sensor %d",
//								temp, i + 1);
//					} else {
					if (temperature_unit != TemperatureUnit_C) // TemperatureUnit_F
							{
						temp *= 9; // (°C × 9/5) + 32
						temp /= 5; // (°C × 9/5) + 32
						temp += 32; // (°C × 9/5) + 32
					}

					SENSORS[i].TEMPERATURE = temp;

					status = HAL_OK;
//					}
				} else {
					SENSORS[i].TEMPERATURE = 0.0;

					if (SENSORS[i].FLAG_ENABLED == 0) {
						status = HAL_OK; // ignore Error

						break; // skip this sensor
					}
				}

				if (k < 9) {
					k++;
				} else {
					if (SENSORS[i].FLAG_ENABLED == 1) {
						fail = 1;

						UsrLog("ERROR read sensor %d", i + 1);
						SENSORS[i].FLAG_TIMEOUT = 1;
					}

					break;
				}
			}
//			}
		}

		if (mode == DS18B20_SLEEP_INTERVAL) {
			if (result != HAL_OK) {
				UsrLog("result != HAL_OK");
				return HAL_ERROR;
			} else {
				return HAL_OK;
			}
		}
	}

	//	if(mode == DS18B20_READ)
	//	{
	//		HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_RESET); // sensors off
	//	}

	if ((mode != DS18B20_SLEEP_INTERVAL)
			&& (mode != DS18B20_START_CONVERSION_NO_IRQ)) {
		//HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
		if (init_flag == 1) {
			HAL_TIM_Base_Start_IT(&htim7);
		}
	}

	if (result != HAL_OK) {
		if (fail > 0) {
			UsrLog("result != HAL_OK; End");
			return HAL_ERROR;
		} else {
			return HAL_OK;
		}
	} else {
		//UsrLog("result = HAL_OK; End");
		return HAL_OK;
	}
}

HAL_StatusTypeDef DS18B20_start_conversion(DS18B20_TypeDef *SENSORS,
		uint8_t NumOfSensors) {
	for (int i = 0; i < NumOfSensors; i++) {
		uint8_t status = HAL_BUSY;
		int j = 0;
		SENSORS[i].FLAG_TIMEOUT = 0;

		while (status != HAL_OK) {
			DS18B20_get_presence(SENSORS[i]); // wake up sensor

			if (DS18B20_get_presence(SENSORS[i]) == HAL_OK) {
				status = DS18B20_start_temperature_conversion(SENSORS[i]);

				if (status == HAL_OK) {
					SENSORS[i].FLAG_PRESENT = 1;

					break;
				} else {
					UsrLog(
							"Sensor %d: DS18B20_start_temperature_conversion ERROR",
							i + 1);

					SENSORS[i].FLAG_PRESENT = 0;

					if (SENSORS[i].FLAG_ENABLED == 0) {
						break; // skip this sensor
					}
				}
			} else {
				UsrLog("Sensor %d: DS18B20_get_presence ERROR", i + 1);

				SENSORS[i].FLAG_PRESENT = 0;

				if (SENSORS[i].FLAG_ENABLED == 0) {
					break; // skip this sensor
				}
			}

			if (j < 4) {
				j++;
			} else {
				if (SENSORS[i].FLAG_ENABLED == 1) {
					SENSORS[i].FLAG_TIMEOUT = 1;

					UsrLog(
							"Sensor %d: DS18B20_start_temperature_conversion TIMEOUT",
							i + 1);
				}

				break;
			}
		}
	}

	for (int i = 0; i < NumOfSensors; i++) {
		if (SENSORS[i].FLAG_TIMEOUT == 1) {
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}

HAL_StatusTypeDef DS18B20_read_all(DS18B20_TypeDef *SENSORS,
		uint8_t NumOfSensors) {
	for (int i = 0; i < NumOfSensors; i++) {
		uint8_t status = HAL_BUSY;
		int k = 0;

		if ((SENSORS[i].FLAG_TIMEOUT != 1) && (SENSORS[i].FLAG_PRESENT != 0)
				&& (SENSORS[i].FLAG_ENABLED != 0)) {
			while (status != HAL_OK) {
				DS18B20_get_presence(SENSORS[i]); // wake up sensor

				if (DS18B20_get_presence(SENSORS[i]) == HAL_OK) {
					uint8_t id_data[8];

					//UsrLog("Read ID of sensor: %d", i + 1);

					if (DS18B20_get_ID(id_data, SENSORS[i].CHANNEL) != HAL_OK) {
						UsrLog("ERROR get_ID: Step: %d", k);
					} else {
						for (uint8_t j = 0; j < 8; j++) {
							SENSORS[i].SERIAL_ID[j] = id_data[j];
						}

						//UsrLog("Read Temp. of sensor: %d", i + 1);

						float Offset = sensor_offset[i] / 10.0;

						if (DS18B20_get_temperature_no_wait(&SENSORS[i], Offset)
								!= HAL_OK) {
							UsrLog("ERROR get_temperature %d; Sensor:", i + 1);
						} else {
							float temp = SENSORS[i].TEMPERATURE;

							if ((temp >= 80) | (temp <= 5)) {
								UsrLog(
										"result HAL_ERROR: ((temp >= 80) | (temp <= 5)); temp: %.4f; sensor %d",
										temp, i + 1);

								SENSORS[i].TEMPERATURE = 0;
							} else {
								if (SENSORS[i].FLAG_UNIT != TemperatureUnit_C) // TemperatureUnit_F
										{
									temp *= 9; // (°C × 9/5) + 32
									temp /= 5; // (°C × 9/5) + 32
									temp += 32; // (°C × 9/5) + 32

									SENSORS[i].TEMPERATURE = temp;
								}

								status = HAL_OK;
							}
						}
					}
				} else {
					SENSORS[i].TEMPERATURE = 0;

					for (uint8_t j = 0; j < 8; j++) {
						SENSORS[i].SERIAL_ID[j] = 0;
					}

					if (SENSORS[i].FLAG_ENABLED == 0) {
						break; // skip this sensor
					}
				}

				if (k < 9) {
					k++;
				} else {
					UsrLog("ERROR get_ID %d; Sensor:", i + 1);

					status = HAL_ERROR;

					SENSORS[i].FLAG_TIMEOUT = 1;

					for (uint8_t j = 0; j < 8; j++) {
						SENSORS[i].SERIAL_ID[j] = 0;
					}

					SENSORS[i].TEMPERATURE = 0;

					break;
				}
			}
		}
	}

	for (int i = 0; i < NumOfSensors; i++) {
		if (SENSORS[i].FLAG_TIMEOUT == 1) {
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}

HAL_StatusTypeDef DS18B20_get_ID(uint8_t *id_data,
		OneWire_ChannelTypeDef channel) {
	uint8_t id_data_temp[] = { 0, 0, 0, 0, 0, 0, 0, 0 }; //8 Byte

	if (DS18B20_reset(channel) == 0)
		return HAL_ERROR; // reset and check presence

	DS18B20_write_byte(0x33, channel); //Read ROM [33h]

	for (uint8_t i = 0; i < maxChannel; i++) {
		id_data_temp[i] = DS18B20_read_byte(channel); //id_data[0] = 40 = 0x28
	}

	uint8_t crc1 = DS18B20_CRC8(id_data_temp, 7);  //calculate crc

	if (crc1 == id_data_temp[7]) //compare : calculate with received
			{
		//UsrLog("DS18B20 CRC Valid 0x%02X", crc1);

		for (uint8_t i = 0; i < maxChannel; i++) {
			id_data[i] = id_data_temp[i];
		}

		return HAL_OK;
	} else {
		UsrLog("DS18B20 ID CRC ERROR: %02X %02X %02X %02X %02X %02X %02X %02X",
				id_data_temp[0], id_data_temp[1], id_data_temp[2],
				id_data_temp[3], id_data_temp[4], id_data_temp[5],
				id_data_temp[6], id_data_temp[7]);

		return HAL_ERROR;
	}
}

uint8_t DS18B20_send_ID(uint8_t *id_data, OneWire_ChannelTypeDef channel) {
	//uint8_t id_data[] = {40,25,0,0,120,2,0,136}; //8 Byte ID
	if (DS18B20_reset(channel) == 0)
		return -99; // reset and check presence
	DS18B20_write_byte(0x55, channel); //Match ROM [55h]
	for (uint8_t i = 0; i < maxChannel; i++) {
		DS18B20_write_byte(id_data[i], channel); //id_data[0] = 40 = 0x28
	}
	return 0;
}

float DS18B20_get_temperature_with_ID(uint8_t *id_data,
		OneWire_ChannelTypeDef channel) {
	uint8_t pad_data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //9 Byte
	//if(get_presence(channel) == 0) return -99; // reset and check presence
	if (DS18B20_send_ID(id_data, channel) == -99)
		return -99; // reset, check presence and send ID
	DS18B20_write_byte(0x44, channel); //Convert Temperature [44h]
	DS18B20_IO_wait_for_1(20, channel);
	if (DS18B20_reset(channel) == 0)
		return -99; // reset and check presence
	if (DS18B20_send_ID(id_data, channel) == -99)
		return -99; // reset, check presence and send ID
	DS18B20_write_byte(0xBE, channel); //Read Scratchpad [BEh]
	for (uint8_t i = 0; i < 9; i++)
		pad_data[i] = DS18B20_read_byte(channel); //factor out 1/16 and remember 1/16 != 1/16.0
	return ((pad_data[1] << 8) + pad_data[0]) / 16.0; //DS18B20.pdf: val of bit0: 2^(-4) = 1/16
}

uint8_t DS18B20_CRC8(const uint8_t *addr, uint8_t len) //begins from LS-bit of LS-byte (OneWire.h)
{
	uint8_t crc = 0;
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

