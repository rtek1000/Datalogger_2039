// https://github.com/afiskon/stm32-ili9341/tree/master/Lib/ili9341
/*
 MIT License

 Copyright (c) 2018 Aleksander Alekseev

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
//https://github.com/adafruit/Adafruit-GFX-Library
/*
 Software License Agreement (BSD License)

 Copyright (c) 2012 Adafruit Industries.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

/* vim: set ai et ts=4 sw=4: */
#include "stm32f4xx_hal.h"
#include "ILI9341.h"
#include "dwt_stm32_delay.h"
#include <stdio.h>

extern char str[80];

extern void IWDG_delay_ms(uint32_t delay);

/*
 #define  UsrLog(...)   do { \
		printf(__VA_ARGS__); \
		printf("\n"); \
} while (0)
 */

void ILI9341_Select() {
	HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
}

void ILI9341_Unselect() {
	HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

void ILI9341_Reset() {
	HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_SET);
}

uint8_t ILI9341_ReadData(void) {
//uint8_t dat[1] = { 0 };
	uint8_t dat[1] = { 0 };
	HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
	HAL_SPI_Receive(&ILI9341_SPI_PORT, dat, 1, HAL_MAX_DELAY);
	return dat[0];
}

void ILI9341_WriteCommand(uint8_t cmd) {
	HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&ILI9341_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

void ILI9341_WriteData(uint8_t *buff, size_t buff_size) {
	HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
	// split data in small chunks because HAL can't send more then 64K at once
	while (buff_size > 0) {
		uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
		HAL_SPI_Transmit(&ILI9341_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
		buff += chunk_size;
		buff_size -= chunk_size;
	}
}

void ILI9341_WriteData16(uint16_t data_in) {
	uint8_t data[] = { data_in >> 8, data_in & 0xFF };
	ILI9341_WriteData(data, sizeof(data));
}

void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1,
		uint16_t y1) {
	// column address set
	ILI9341_WriteCommand(0x2A); // CASET
	{
		uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1
				& 0xFF };
		ILI9341_WriteData(data, sizeof(data));
	}

	// row address set
	ILI9341_WriteCommand(0x2B); // RASET
	{
		uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1
				& 0xFF };
		ILI9341_WriteData(data, sizeof(data));
	}

	// write to RAM
	ILI9341_WriteCommand(0x2C); // RAMWR
}

void ILI9341_Init() {
	HAL_SPI_Transmit(&ILI9341_SPI_PORT, 0x00, 1, HAL_MAX_DELAY);
	//IWDG_delay_ms(120);

	ILI9341_Select();
	ILI9341_Reset();

	IWDG_delay_ms(120);
	// command list is based on https://github.com/martnak/STM32-ILI9341

	// SOFTWARE RESET
	ILI9341_WriteCommand(0x01);
	IWDG_delay_ms(120); // Datasheet page 218 of 233
	// https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf

	// POWER CONTROL A
	ILI9341_WriteCommand(0xCB);
	{
		uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// POWER CONTROL B
	ILI9341_WriteCommand(0xCF);
	{
		uint8_t data[] = { 0x00, 0xC1, 0x30 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL A
	ILI9341_WriteCommand(0xE8);
	{
		uint8_t data[] = { 0x85, 0x00, 0x78 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL B
	ILI9341_WriteCommand(0xEA);
	{
		uint8_t data[] = { 0x00, 0x00 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// POWER ON SEQUENCE CONTROL
	ILI9341_WriteCommand(0xED);
	{
		uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// PUMP RATIO CONTROL
	ILI9341_WriteCommand(0xF7);
	{
		uint8_t data[] = { 0x20 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// POWER CONTROL,VRH[5:0]
	ILI9341_WriteCommand(0xC0);
	{
		//		uint8_t data[] = { 0x23 }; // 0x23
		uint8_t data[] = { 0x21 }; // 0x23
		ILI9341_WriteData(data, sizeof(data));
	}

	// POWER CONTROL,SAP[2:0];BT[3:0]
	ILI9341_WriteCommand(0xC1);
	{
		uint8_t data[] = { 0x10 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// VCM CONTROL
	ILI9341_WriteCommand(0xC5);
	{
		//		uint8_t data[] = { 0x3E, 0x28 }; // 0x3E, 0x28
		uint8_t data[] = { 0x31, 0x3C }; // 0x3E, 0x28
		ILI9341_WriteData(data, sizeof(data));
	}

	// VCM CONTROL 2
	ILI9341_WriteCommand(0xC7);
	{
//		uint8_t data[] = { 0x86 }; // 0x86
		uint8_t data[] = { 0xC0 }; // 0x86
		ILI9341_WriteData(data, sizeof(data));
	}

	// MEMORY ACCESS CONTROL
	ILI9341_WriteCommand(0x36);
	{
		uint8_t data[] = { 0x48 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// PIXEL FORMAT
	ILI9341_WriteCommand(0x3A);
	{
		uint8_t data[] = { 0x55 }; // R5-G6-B5: 0x55 R6-G6-B6: 0x66
		ILI9341_WriteData(data, sizeof(data));
	}

	// FRAME RATIO CONTROL, STANDARD RGB COLOR
	ILI9341_WriteCommand(0xB1);
	{
		//		uint8_t data[] = { 0x00, 0x18 };
		uint8_t data[] = { 0x00, 0x1B }; // 0x18
		ILI9341_WriteData(data, sizeof(data));
	}

	// DISPLAY FUNCTION CONTROL
	ILI9341_WriteCommand(0xB6);
	{
		uint8_t data[] = { 0x08, 0x82, 0x27 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// 3GAMMA FUNCTION DISABLE
	ILI9341_WriteCommand(0xF2);
	{
		uint8_t data[] = { 0x00 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// GAMMA CURVE SELECTED
	ILI9341_WriteCommand(0x26);
	{
		uint8_t data[] = { 0x01 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// POSITIVE GAMMA CORRECTION
	ILI9341_WriteCommand(0xE0);
	{
//		uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37,
//				0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
		uint8_t data[] = { 0x08, 0x31, 0x2B, 0x05, 0x0E, 0x09, 0x4E, 0xF1, 0x37,
				0x0B, 0x10, 0x00, 0x0E, 0x09, 0x00 };
		ILI9341_WriteData(data, sizeof(data));
	}

	// NEGATIVE GAMMA CORRECTION
	ILI9341_WriteCommand(0xE1);
	{
//		uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48,
//				0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
		uint8_t data[] = { 0x08, 0x0E, 0x14, 0x07, 0x11, 0x05, 0x31, 0xC1, 0x48,
				0x04, 0x0F, 0x0F, 0x31, 0x36, 0x0F };
		ILI9341_WriteData(data, sizeof(data));
	}

	// EXIT SLEEP
	ILI9341_WriteCommand(0x11);
	IWDG_delay_ms(120);

	// TURN ON DISPLAY
	ILI9341_WriteCommand(0x29);

	// MADCTL
	ILI9341_WriteCommand(0x36);
	{
		uint8_t data[] = { ILI9341_ROTATION };
		ILI9341_WriteData(data, sizeof(data));
	}

	ILI9341_Unselect();
}

void ILI9341_Draw_BMP(uint16_t x, uint16_t y, const uint16_t *bitmap,
		uint16_t w, uint16_t h) {

	ILI9341_Select();

	for (uint16_t y1 = 0; y1 < h; y1++) {
		for (uint16_t x1 = 0; x1 < w; x1++) {
			ILI9341_WritePixel(x1 + x, y1 + y,
					(uint16_t) *(bitmap + y1 * w + x1));
		}
	}

	ILI9341_Unselect();
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;

	ILI9341_Select();

	ILI9341_SetAddressWindow(x, y, x + 1, y + 1);
	uint8_t data[] = { color >> 8, color & 0xFF };
	ILI9341_WriteData(data, sizeof(data));

	ILI9341_Unselect();
}

#define grave_accent 96
#define acute_accent 180
#define circumflex_accent 94
#define tilde_accent 126
#define diaeresis_accent 168

#define A_grave_accent 192
#define A_acute_accent 193
#define A_circumflex_accent 194
#define A_tilde_accent 195
#define A_diaeresis_accent 196

#define C_cedil 199

#define E_grave_accent 200
#define E_acute_accent 201
#define E_circumflex_accent 202
#define E_diaeresis_accent 203

#define I_grave_accent 204
#define I_acute_accent 205
#define I_circumflex_accent 206
#define I_diaeresis_accent 207

#define O_grave_accent 210
#define O_acute_accent 211
#define O_circumflex_accent 212
#define O_tilde_accent 213
#define O_diaeresis_accent 214

#define U_grave_accent 217
#define U_acute_accent 218
#define U_circumflex_accent 219
#define U_diaeresis_accent 220

#define a_grave_accent 224
#define a_acute_accent 225
#define a_circumflex_accent 226
#define a_tilde_accent 227
#define a_diaeresis_accent 228

#define c_cedil 231

#define e_grave_accent 232
#define e_acute_accent 233
#define e_circumflex_accent 234
#define e_diaeresis_accent 235

#define i_grave_accent 236
#define i_acute_accent 237
#define i_circumflex_accent 238
#define i_diaeresis_accent 239

#define o_grave_accent 242
#define o_acute_accent 243
#define o_circumflex_accent 244
#define o_tilde_accent 245
#define o_diaeresis_accent 246

#define u_grave_accent 249
#define u_acute_accent 250
#define u_circumflex_accent 251
#define u_diaeresis_accent 252

void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font,
		uint16_t color, uint16_t bgcolor) {
	uint32_t i, b, j;

	//UsrLog("char: %d", ch);

	ILI9341_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

	if (ch == C_cedil)
		ch = 127;
	else if (ch == c_cedil)
		ch = 128;
	else if (ch == acute_accent)
		ch = 129;
	else if (ch == diaeresis_accent)
		ch = 130;

	else if (ch == A_grave_accent)
		ch = 131;
	else if (ch == A_acute_accent)
		ch = 132;
	else if (ch == A_circumflex_accent)
		ch = 133;
	else if (ch == A_tilde_accent)
		ch = 134;
	else if (ch == A_diaeresis_accent)
		ch = 135;

	else if (ch == E_grave_accent)
		ch = 136;
	else if (ch == E_acute_accent)
		ch = 137;
	else if (ch == E_circumflex_accent)
		ch = 138;
	else if (ch == E_diaeresis_accent)
		ch = 139;

	else if (ch == I_grave_accent)
		ch = 140;
	else if (ch == I_acute_accent)
		ch = 141;
	else if (ch == I_circumflex_accent)
		ch = 142;
	else if (ch == I_diaeresis_accent)
		ch = 143;

	else if (ch == O_grave_accent)
		ch = 144;
	else if (ch == O_acute_accent)
		ch = 145;
	else if (ch == O_circumflex_accent)
		ch = 146;
	else if (ch == O_tilde_accent)
		ch = 147;
	else if (ch == O_diaeresis_accent)
		ch = 148;

	else if (ch == U_grave_accent)
		ch = 149;
	else if (ch == U_acute_accent)
		ch = 150;
	else if (ch == U_circumflex_accent)
		ch = 151;
	else if (ch == U_diaeresis_accent)
		ch = 152;

	else if (ch == a_grave_accent)
		ch = 153;
	else if (ch == a_acute_accent)
		ch = 154;
	else if (ch == a_circumflex_accent)
		ch = 155;
	else if (ch == a_tilde_accent)
		ch = 156;
	else if (ch == a_diaeresis_accent)
		ch = 157;

	else if (ch == e_grave_accent)
		ch = 158;
	else if (ch == e_acute_accent)
		ch = 159;
	else if (ch == e_circumflex_accent)
		ch = 160;
	else if (ch == e_diaeresis_accent)
		ch = 161;

	else if (ch == i_grave_accent)
		ch = 162;
	else if (ch == i_acute_accent)
		ch = 163;
	else if (ch == i_circumflex_accent)
		ch = 164;
	else if (ch == i_diaeresis_accent)
		ch = 165;

	else if (ch == o_grave_accent)
		ch = 166;
	else if (ch == o_acute_accent)
		ch = 167;
	else if (ch == o_circumflex_accent)
		ch = 168;
	else if (ch == o_tilde_accent)
		ch = 169;
	else if (ch == o_diaeresis_accent)
		ch = 170;

	else if (ch == u_grave_accent)
		ch = 171;
	else if (ch == u_acute_accent)
		ch = 172;
	else if (ch == u_circumflex_accent)
		ch = 173;
	else if (ch == u_diaeresis_accent)
		ch = 174;

	else if (ch >= 127)
		ch = ' ';

	for (i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++) {
			if ((b << j) & 0x8000) {
				uint8_t data[] = { color >> 8, color & 0xFF };
				ILI9341_WriteData(data, sizeof(data));
			} else {
				uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
				ILI9341_WriteData(data, sizeof(data));
			}
		}
	}
}

void ILI9341_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font,
		uint16_t color, uint16_t bgcolor) {
	ILI9341_Select();

	while (*str) {
		if (x + font.width >= ILI9341_WIDTH) {
			x = 0;
			y += font.height;
			if (y + font.height >= ILI9341_HEIGHT) {
				break;
			}

			if (*str == ' ') {
				// skip spaces in the beginning of the new line
				str++;
				continue;
			}
		}

		ILI9341_WriteChar(x, y, *str, font, color, bgcolor);
		x += font.width;
		str++;
	}

	ILI9341_Unselect();
}

void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t color) {
	// clipping
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;
	if ((x + w - 1) >= ILI9341_WIDTH)
		w = ILI9341_WIDTH - x;
	if ((y + h - 1) >= ILI9341_HEIGHT)
		h = ILI9341_HEIGHT - y;

	ILI9341_Select();
	ILI9341_SetAddressWindow(x, y, x + w - 1, y + h - 1);

	uint8_t data[] = { color >> 8, color & 0xFF };
	HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
	for (y = h; y > 0; y--) {
		for (x = w; x > 0; x--) {
			HAL_SPI_Transmit(&ILI9341_SPI_PORT, data, sizeof(data),
			HAL_MAX_DELAY);
		}
	}

	ILI9341_Unselect();
}

void ILI9341_FillScreen(uint16_t color) {
	ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		const uint16_t *data) {
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;
	if ((x + w - 1) >= ILI9341_WIDTH)
		return;
	if ((y + h - 1) >= ILI9341_HEIGHT)
		return;

	ILI9341_Select();
	ILI9341_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	ILI9341_WriteData((uint8_t*) data, sizeof(uint16_t) * w * h);
	ILI9341_Unselect();
}

void ILI9341_InvertColors(bool invert) {
	ILI9341_Select();
	ILI9341_WriteCommand(invert ? 0x21 /* INVON */: 0x20 /* INVOFF */);
	ILI9341_Unselect();
}

void ILI9341_DrawLineH(uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;

	ILI9341_FillRectangle(x, y, w, 1, color);
}

void ILI9341_DrawLineV(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;

	ILI9341_FillRectangle(x, y, 1, h, color);
}

void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t color) {
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;

	ILI9341_FillRectangle(x, y, w, 1, color);
	ILI9341_FillRectangle(x, y + h, w, 1, color);

	ILI9341_FillRectangle(x, y, 1, h, color);
	ILI9341_FillRectangle(x + w, y, 1, h, color);
}

// Source: Adafruit

#define abs(x) ((x)>0?(x):-(x))
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void ILI9341_WritePixel(int16_t x, int16_t y, uint16_t color) {
	if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
		return;

	ILI9341_SetAddressWindow(x, y, x + 1, y + 1);
	uint8_t data[] = { color >> 8, color & 0xFF };
	ILI9341_WriteData(data, sizeof(data));
}

void ILI9341_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		uint16_t color) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	ILI9341_Select();

	for (; x0 <= x1; x0++) {
		if (steep) {
			ILI9341_WritePixel(y0, x0, color);
		} else {
			ILI9341_WritePixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}

	ILI9341_Unselect();
}

/*
 * End code from Adafruit
 */

// Source: https://github.com/stevstrong/Arduino_STM32/tree/master/STM32F1/libraries/Adafruit_ILI9341_STM
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ILI9341_color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t ILI9341_readPixel(int16_t x, int16_t y) {
	//mSPI.beginTransaction(SPISettings(_safe_freq, MSBFIRST, SPI_MODE0, SPI_DATA_SIZE_8BIT));
	ILI9341_Select();

	ILI9341_WriteCommand(ILI9341_CASET); // Column addr set
	ILI9341_WriteData16(x);
	ILI9341_WriteData16(x);
	ILI9341_WriteCommand(ILI9341_PASET); // Row addr set
	ILI9341_WriteData16(y);
	ILI9341_WriteData16(y);
	ILI9341_WriteCommand(ILI9341_RAMRD); // read GRAM
	ILI9341_ReadData();             //dummy read
	uint8_t r = ILI9341_ReadData();
	uint8_t g = ILI9341_ReadData();
	uint8_t b = ILI9341_ReadData();
	ILI9341_Unselect();  // cs_set();

//	mSPI.beginTransaction(
//			SPISettings(_freq, MSBFIRST, SPI_MODE0, SPI_DATA_SIZE_16BIT));

	if ((x % 10) == 0) {
		UsrLog("x: %u, y: %u RGB: %d %d %d", x, y, r, g, b);
	}

	return 0; // ILI9341_color565(r, g, b);
}

uint16_t ILI9341_readPixels16(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t *buf) {
	//mSPI.beginTransaction(SPISettings(_safe_freq, MSBFIRST, SPI_MODE0, SPI_DATA_SIZE_8BIT));
	ILI9341_Select();
	ILI9341_WriteCommand(ILI9341_CASET); // Column addr set
	ILI9341_WriteData16(x1);
	ILI9341_WriteData16(x2);
	ILI9341_WriteCommand(ILI9341_PASET); // Row addr set
	ILI9341_WriteData16(y1);
	ILI9341_WriteData16(y2);
	ILI9341_WriteCommand(ILI9341_RAMRD); // read GRAM
	(void) ILI9341_ReadData();             //dummy read
	uint8_t rgb_buf[3];
	uint16_t len = (x2 - x1 + 1) * (y2 - y1 + 1);
	uint16_t ret = len * 3;
	while (len--) {
		//mSPI.read(rgb, 3);
		HAL_SPI_Receive(&ILI9341_SPI_PORT, rgb_buf, 3, HAL_MAX_DELAY);
		*buf++ = ILI9341_color565(rgb_buf[0], rgb_buf[1], rgb_buf[2]);
	}
	ILI9341_Unselect(); //cs_set();

	//mSPI.beginTransaction(SPISettings(_freq, MSBFIRST, SPI_MODE0, SPI_DATA_SIZE_16BIT));
	return ret;
}

uint16_t ILI9341_readPixels24(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint8_t *buf) {
	ILI9341_Select();

//	mSPI.beginTransaction(
//			SPISettings(_safe_freq, MSBFIRST, SPI_MODE0, SPI_DATA_SIZE_8BIT));

	ILI9341_WriteCommand(ILI9341_CASET); // Column addr set
	ILI9341_WriteData16(x1);
	ILI9341_WriteData16(x2);
	ILI9341_WriteCommand(ILI9341_PASET); // Row addr set
	ILI9341_WriteData16(y1);
	ILI9341_WriteData16(y2);
	ILI9341_WriteCommand(ILI9341_RAMRD); // read GRAM
	// (void) spiread();             //dummy read
	ILI9341_ReadData();
	uint16_t len = (x2 - x1 + 1) * (y2 - y1 + 1);
	uint16_t ret = len * 3;
	// mSPI.dmaTransfer(buf, buf, len * 3);
	HAL_SPI_Receive(&ILI9341_SPI_PORT, buf, ret, HAL_MAX_DELAY);
	ILI9341_Unselect(); //cs_set();

//	mSPI.beginTransaction(
//			SPISettings(_freq, MSBFIRST, SPI_MODE0, SPI_DATA_SIZE_16BIT));
	return ret;
}

/*
 * https://github.com/stevstrong/Arduino_STM32/tree/master/STM32F1/libraries/Adafruit_ILI9341_STM
 */

// Read Display Identification Information - RDDIDIF
uint32_t ILI9341_Status(void) {
	uint32_t ret = 0;

	ILI9341_Select();

	//HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
	ILI9341_WriteCommand(ILI9341_RDDST); // 0x09: Read Display Status

	//HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
	ILI9341_ReadData(); // dummy read

	// HAL_SPI_Receive(&ILI9341_SPI_PORT, _buf, 3, HAL_MAX_DELAY);

	ret = ILI9341_ReadData() << 24;
	ret |= ILI9341_ReadData() << 16;
	ret |= ILI9341_ReadData() << 8;
	ret |= ILI9341_ReadData();

	ILI9341_Unselect(); //cs_set();

	return ret;
}
