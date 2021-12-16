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

/* vim: set ai et ts=4 sw=4: */

#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_

#include "fonts.h"
#include <stdbool.h>
#include "main.h"

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

/*** Redefine if necessary ***/
#define ILI9341_SPI_PORT hspi3
extern SPI_HandleTypeDef ILI9341_SPI_PORT;

#define ILI9341_RES_Pin       LCD_RST_Pin
#define ILI9341_RES_GPIO_Port LCD_RST_GPIO_Port
#define ILI9341_CS_Pin        LCD_CS_Pin
#define ILI9341_CS_GPIO_Port  LCD_CS_GPIO_Port
#define ILI9341_DC_Pin        LCD_DC_Pin
#define ILI9341_DC_GPIO_Port  LCD_DC_GPIO_Port

// default orientation
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)
*/

// rotate right
/*
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
*/

// rotate left
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)

// upside down
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)
*/

/****************************/

// Color definitions
#define	ILI9341_BLACK       0x0000
#define	ILI9341_BLUE        0x001F
#define	ILI9341_RED         0xF800
#define	ILI9341_GREEN       0x07E0
#define ILI9341_CYAN        0x07FF
#define ILI9341_MAGENTA     0xF81F
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_WHITE       0xFFFF
#define ILI9341_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define ILI9341_GREY        0x8410 // R:128, G:128, B:128

#define ILI9341_DARKGRAY    0x2104 // R:32, G:32, B:32
#define ILI9341_DARKGREY    0x2104 // R:32, G:32, B:32
#define ILI9341_DARKBLUE    0x0010 // R:0, G:0, B:128
#define ILI9341_DARKRED     0x7800 // R:128, G:0, B:0
#define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
#define ILI9341_DARKYELLOW  0x8C40  ///< 137, 138, 0

#define	ILI9341_BLUESKY     0x047F
#define	ILI9341_BROWN       0xC408
#define	ILI9341_LIME        0xBFE0
#define	ILI9341_PURPLE      0x801F

#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK        0xFC18  ///< 255, 130, 198

// call before initializing any SPI devices
void ILI9341_Unselect();

void ILI9341_Select();
void ILI9341_Reset();
void ILI9341_WriteCommand(uint8_t cmd);
void ILI9341_WriteData(uint8_t* buff, size_t buff_size);
void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9341_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

void ILI9341_Init(void);
void ILI9341_Draw_BMP(uint16_t x, uint16_t y, const uint16_t *bitmap, uint16_t w, uint16_t h);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_WritePixel(int16_t x, int16_t y, uint16_t color);
void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_InvertColors(bool invert);
void ILI9341_DrawLineH(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void ILI9341_DrawLineV(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif /* INC_ILI9341_H_ */
