/*
 * SCREEN.c
 *
 *  Created on: Jun 21, 2021
 *      Author: r
 */

#include "main.h"
#include "ILI9341.h"
#include <string.h>

int lcd_bright_new = 0;
int lcd_bright_old = 0;

int lcd_bright_set = 0; // 0-100%
int lcd_bright_for_logo = 100;
int lcd_bright_max = 95;
int lcd_bright_mid = 50;
int lcd_bright_min = 5;
int lcd_bright_off = 0;

extern TIM_HandleTypeDef htim8;

void draw_logo(const unsigned short *logo);
void user_pwm_setvalue(uint16_t value);

void draw_logo(const unsigned short *logo) {
//	ILI9341_FillRectangle(0, 0, 320, 37,
//	ILI9341_WHITE);

	ILI9341_Draw_BMP(0, 0, logo, 320, 240);

//	ILI9341_FillRectangle(0, 202, 320, 38,
//	ILI9341_WHITE);
}

void user_pwm_setvalue(uint16_t value) {
	TIM_OC_InitTypeDef sConfigOC;

	memset(&sConfigOC, 0, sizeof(sConfigOC));

	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = value;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
}
