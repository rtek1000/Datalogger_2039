// Based on source: https://ee-programming-notepad.blogspot.com/2017/06/reading-stm32f4-unique-device-id-from.html

/*
 * STM32F4_UID.c
 *
 *  Created on: Feb 6, 2020
 *      Author: UserZ
 */

#include "STM32F4_UID.h"

HAL_StatusTypeDef STM32F4_getUID(uint32_t* UID)
{
	if(sizeof UID < 3)
	{
		return HAL_ERROR;
	}
	else if(STM32_UUID[0] == 0)
	{
		return HAL_ERROR;
	}
	else if(STM32_UUID[1] == 0)
	{
		return HAL_ERROR;
	}
	else if(STM32_UUID[2] == 0)
	{
		return HAL_ERROR;
	}

	UID[0] = STM32_UUID[0];
	UID[1] = STM32_UUID[1];
	UID[2] = STM32_UUID[2];

	return HAL_OK;
}

