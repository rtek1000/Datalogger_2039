/*
 * SCREEN.h
 *
 *  Created on: Feb 7, 2020
 *      Author: UserZ
 */

#ifndef INC_SCREEN_H_
#define INC_SCREEN_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "ILI9341.h"
#include "DS3231.h"
#include "DS18B20.h"
#include "w25qxx.h"
//#include "MPU6050.h"
#include "usbh_hid_keybd.h"
#include "stm32f4xx_hal_adc.h"
//#include "language.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
	menuListExportData = 0,
	menuListGraphic,
	menuListSetPassword,
	menuListSensors,
	menuListClockAdjust,
	menuListMemoryTest
} MenuListTypeDef;

// LcdScreenMenu,
// LcdScreenLangExport,

#ifndef sepChr
#define sepChr 59  	// 9: horizontal tab "	", 59: char ";"
#define degChr '*' //42 	// deg symbol "°"
#endif // #ifndef sepChr

void add_char_to_text(uint8_t keycode, char *labelChar);
//void text_editor(HID_KEYBD_Info_TypeDef *keybd_info, uint8_t key,
//		uint8_t keycode, char *labelChar);
void text_editor(char *labelChar);
void draw_logo(const unsigned short *logo);
void lcd_menu_keyboard(HID_KEYBD_Info_TypeDef *keybd_info);
void lcd_menu_keyboard_async(void);
void make_menu_str(uint8_t channel);
uint8_t str_add_spaces(char *str_data, int length);
//uint16_t size_of_char(const char *char_in);
uint8_t lcd_main(int phase);
void lcd_menu(void);
void lcd_config_clock_cursor_move(void);
void lcd_config_clock(void);
void lcd_config_clock_load(void);
//int lcd_temp_check(int update_window, int phase, float* temperatures);
int lcd_batt_charging(void);
int lcd_sensors_monitor(int update_window, int phase);
void lcd_sensors_monitor_offset(void);
int lcd_lang_export(void);
int lcd_temp_check(int update_window, int phase);
//int lcd_adc_check(int update_window, int phase);
void lcd_config_ch_load_sensor_value(void);
void ch_config_start(int index);
void ch_config_stop(int index);
void lcd_config_ch(void);
void lcd_config_ch_blink_status(void);
void lcd_config_ch_blink_status2(void);
void lcd_config_ch_confirm_export(void);
void lcd_config_language(void);
void lcd_config_ch_erase(void);
void lcd_config_ch_label(int label_index, uint16_t color);
void lcd_config_ch_str_start(uint8_t channel);
void lcd_config_ch_str_stop(uint8_t channel);
void lcd_config_ch_confirm(void);
void lcd_button(int left, int top, int w, int h, char *title, int title_size,
		uint16_t color, uint16_t font_color);
void lcd_window(int left, int top, int w, int h, const char *title,
		uint16_t color, uint16_t font_color);
uint8_t lcd_loop(void);
uint8_t lcd_usb_export(void);
uint8_t lcd_progress_barX10(void);
uint8_t lcd_progress_bar(void);
void lcd_draw_progress_bars(uint8_t progress);
//uint8_t lcd_memory_test_progress_bar(uint16_t progress, uint16_t max);
uint8_t lcd_memory_test_status(uint8_t ch, uint8_t step, uint8_t max);
uint8_t lcd_memory_test(void);
uint8_t lcd_password_config(void);
uint8_t lcd_password(void);
void lcd_password_blink(uint16_t color);
//void lcd_graph_test(void);
void lcd_data_process(int channel);
uint8_t lcd_graph(int phase);
uint8_t drawLegends(int phase);
void drawInfo(void);
uint8_t drawGrids(int phase);
void drawGrid(int left, int top, int width, int height);
void drawLines(int left, int top, int width, int height, int *dataBuff,
		int lineColor);

#endif /* INC_SCREEN_H_ */
