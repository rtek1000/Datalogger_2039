/*
 * SN74HC595.c
 *
 *  Created on: Jun 22, 2021
 *      Author: r
 */
#include "main.h"

uint8_t HC595_buff = 0;

void HC595_init(void);
void HC595_set(uint8_t value);

void HC595_init(void) {
	HAL_GPIO_WritePin(HC595_OE_GPIO_Port, HC595_OE_Pin, GPIO_PIN_SET);

	HC595_set(0);

	HC595_buff = 0;

	HAL_GPIO_WritePin(HC595_OE_GPIO_Port, HC595_OE_Pin, GPIO_PIN_RESET);
}

void HC595_set(uint8_t value) {
	for (int i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(HC595_DAT_GPIO_Port, HC595_DAT_Pin, (value >> (7 - i)) & 1);

		HAL_GPIO_WritePin(HC595_CLK_GPIO_Port, HC595_CLK_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(HC595_CLK_GPIO_Port, HC595_CLK_Pin, GPIO_PIN_RESET);
	}

	HAL_GPIO_WritePin(HC595_LAT_GPIO_Port, HC595_LAT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(HC595_LAT_GPIO_Port, HC595_LAT_Pin, GPIO_PIN_RESET);
}
