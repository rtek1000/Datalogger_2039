/*
 * IWDG_Delay.c
 *
 *  Created on: Jun 21, 2021
 *      Author: r
 */
#include "main.h"

extern IWDG_HandleTypeDef hiwdg;
extern uint8_t save_BMP_enabled;
extern int lcd_screen;

extern void MX_USB_HOST_Process_FS(void);
extern void MX_USB_HOST_Process_HS(void);

void IWDG_reset(void) {
	/* Refresh IWDG: reload counter */
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
		/* Refresh Error */
		Error_Handler();
	}
}

void IWDG_delay_ms(uint32_t delay) {
	IWDG_reset();

	MX_USB_HOST_Process_FS();

	if ((save_BMP_enabled == 1) || (lcd_screen == LcdScreenUsbExport)) {
		MX_USB_HOST_Process_HS();
	}

	while (delay--) {
		IWDG_reset();
		HAL_Delay(1);

		MX_USB_HOST_Process_FS();
	}
}
