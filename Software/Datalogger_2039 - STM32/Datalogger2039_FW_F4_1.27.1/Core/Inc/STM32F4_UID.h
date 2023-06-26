/*
 * STM32F4_UID.h
 *
 *  Created on: Feb 6, 2020
 *      Author: UserZ
 */

#ifndef INC_STM32F4_UID_H_
#define INC_STM32F4_UID_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define STM32_UUID ((uint32_t *)0x1FFF7A10)

HAL_StatusTypeDef STM32F4_getUID(uint32_t* UID);

#endif /* INC_STM32F4_UID_H_ */
