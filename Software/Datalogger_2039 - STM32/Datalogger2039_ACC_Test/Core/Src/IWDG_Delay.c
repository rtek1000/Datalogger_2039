/*
 * IWDG_Delay.c
 *
 *  Created on: Jun 21, 2021
 *      Author: r
 */
#include "main.h"

extern IWDG_HandleTypeDef hiwdg;

void IWDG_reset(void) {
	/* Refresh IWDG: reload counter */
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
		/* Refresh Error */
		Error_Handler();
	}
}

void IWDG_delay_ms(uint32_t delay) {
	IWDG_reset();

	while (delay--) {
		IWDG_reset();
		HAL_Delay(1);
	}
}
