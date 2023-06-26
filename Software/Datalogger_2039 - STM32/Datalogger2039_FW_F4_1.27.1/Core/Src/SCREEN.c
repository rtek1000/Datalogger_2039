/*
 * SCREEN.c
 *
 *  Created on: Feb 7, 2020
 *      Author: UserZ
 */

#include "SCREEN.h"
#include "main.h"
#include "usb_host.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "channels_data.h"
#include "ILI9341.h"
#include "EEPROM_EXTERN.h"
#include "w25qxx.h"
#include "language.h"
#include "key_map.h"
#include "screen_menu.h"

#define PI 3.14159265

#define dataLength 320

uint8_t save_new_passwd = 0;
uint8_t save_channel_label = 0;
uint8_t save_channel_stop = 0;

uint8_t lcd_status_cnt = 0;

int accent1 = 0;
int maxVal[maxChannel];
int minVal[maxChannel];

uint8_t batt_error_esc = 0;

int cnt_timeout_reset_USB = 0;

int dataChart[maxChannel][dataLength];

int dataChartOld[maxChannel][dataLength];

uint8_t chart_zoom[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t chart_zoom_week[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t chart_zoom_day[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t chart_zoom_hour[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t chart_zoom_enable[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t chart_shift = 0;

uint8_t clock1_update = TRUE;

uint32_t tick_USB_reset = 0;

uint16_t dataColor[maxChannel] = { ILI9341_RED, ILI9341_BROWN, ILI9341_ORANGE,
ILI9341_YELLOW,
ILI9341_GREEN, ILI9341_CYAN, ILI9341_BLUESKY, ILI9341_MAGENTA };

const uint16_t dataColorRecall[maxChannel] = { ILI9341_RED, ILI9341_BROWN,
ILI9341_ORANGE, ILI9341_YELLOW,
ILI9341_GREEN, ILI9341_CYAN, ILI9341_BLUESKY, ILI9341_MAGENTA };

uint8_t channelSelected = 0;

int menu_button = 0;

uint8_t hide_batt100 = FALSE;
uint8_t hide_batt10 = FALSE;

uint8_t cycle_count = 0;
uint8_t secondOld = 0;

char clock_date[14]; // "Seg. 31/12/99"
char clock_time[12]; // "23:59:59"

int clock_index_edit = 0;
int clock_index_edit_mode = 0;

int ch_config_index_edit = 0;
#define ch_config_index_edit_mode_default 1
#define ch_config_index_edit_mode_text 0
#define ch_config_index_edit_mode_status 1
int ch_config_index_edit_mode = ch_config_index_edit_mode_default;

uint32_t drawInfo_tick = 0;
uint8_t seconds_old = 0;
uint8_t second, minute, hour, day, date, month, year;

int update_window = 0;

int pwm_value = 0;
int step = 0;

int lcd_bright_new = 0;
int lcd_bright_old = 0;

int update_info_seconds_old = 0;

char txtData1_old[22] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0 };

int week_old_update_info = 0;
int keyboard_old_update_info = 0;
uint8_t hide_keyboard_cross = FALSE;
int charger_old_update_info = 0;
//int bat100_old_update_info = 0;
int bat10_old_update_info = 0;
int bat1_old_update_info = 0;

int ch_edit = 1;

int label_index_edit = 0;
int label_index_edit2 = 0;
uint8_t clock1_setup = FALSE;
int lcd_phase = 0;
int ch_config_index = 0;
char str[2048];
uint8_t refresh_info = TRUE;
int lcd_bright_set = 0;

uint16_t progress_bar_usb_export_value;

uint16_t progress_bar_mem_test_value;

char start_str_old[maxChannel];

uint8_t passwd_esc_cnt = 0;
uint8_t passwd_esc_cnt0 = 0;

ApplicationTypeDef Appli_state_FS_old = APPLICATION_START;
uint8_t caps_lock_state_old = FALSE;

char start_str[12] = { '-', '-', ':', '-', '-', ':', '-', '-', 0 }; // sprintf needs 12

uint8_t export_ch[maxChannel] = { 1, 1, 1, 1, 1, 1, 1, 1 };
uint8_t test_ch[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };

//79934932950175 SHA256:
//598e114c0a39cbffd8aeb0e6acaa329bf1165493d527c99be349b3e295a01c75
//598 114 0 39     8   0 6    329  1165493 527 99  349 3 295 01 75
//598 114 039 806 329 116 549 352 799 349 329 501 75

// 1d7b816a868b568e0f3e3d3ae3ab8d7eca5d5831a5ed8747c3a4dde6147708cd
// 1 7 816 868 568 0 3 3 3  3  8 7   5 5831 5  8747 3 4   6147708
// 178 168 685 680 333 387 558 315 874 734 614 770 8
// 1781686856803333875583158747346147708
//
// 178 168
// 685 680
// 333 387
// 558 315
// 874 734
// 614 770
// 8

uint8_t passwd_recall[] = { "1781686856803333875583158747346147708" };
uint8_t passwd_recall_count = 0;

uint8_t sensor_value_refresh = 0;

uint16_t progress_bar_count = 0;

uint8_t sysTick15000_Old = 0;

uint8_t ctrl_atl_del_cnt_timeout = 0;
uint8_t esc_cnt_timeout = 0;

uint8_t main_keyboard_chars_num = 0;
uint8_t main_udisk_chars_num = 0;

uint8_t USB_FS_OC_Error_old = 0;
uint8_t USB_HS_OC_Error_old = 0;

uint8_t USB_OC_Error_blink = 0;
uint8_t USB_OC_Error_blink_cnt = 0;

uint8_t DC_DC_Error_blink_cnt = 0;

uint8_t LcdScreenMonChOffsetCh = 1;
int16_t sensor_offset_tmp = 0;

extern char channelLabel_ctrl_c[maxChar_x2];
extern char channelLabel_selected[maxChar_x2];

extern int16_t sensor_offset[maxChannel];

extern uint32_t lcd_status_reference;

extern uint8_t scanning_sensor_ID;

extern uint8_t USB_FS_OC_Error;
extern uint8_t USB_HS_OC_Error;

extern uint8_t DC_DC_Error;

extern uint8_t esc_cnt;

extern uint8_t ctrl_atl_del_timeout;

extern uint8_t ctrl_atl_del_cnt;

extern uint8_t sysTick15000;

extern int back_to_main_timeout;
extern const uint16_t back_to_main_timeout_recall;

extern uint8_t file_type;
extern uint16_t file_cnt_csv;
extern uint16_t file_cnt_dat;

extern uint8_t language_current;
extern uint8_t language_current_tmp;
extern uint8_t language_screen_index;
extern uint8_t decimal_separator;
extern uint8_t date_format;
extern uint8_t temperature_unit;

extern uint8_t kbd_num_lock_state;

extern uint32_t TP4056_charger_freq;
extern uint32_t battery_failure_timeout;
extern volatile uint32_t battery_failure_cnt;
extern volatile HAL_StatusTypeDef battery_failure_flag;

extern uint8_t battery_status;
extern uint8_t button1_flag;
extern uint8_t button1_flag_old;

extern float batt_percent_up;
extern float batt_percent_down;

extern float batt_percent;

extern uint8_t save_BMP_enabled;

extern uint32_t abort_operation;
extern uint32_t userFunction_phase;
extern uint8_t export_data_enabled;
extern uint32_t progress_remaining;

extern uint32_t write_data_steps;
extern uint32_t write_mark_steps;
extern uint16_t erase_steps1;
extern uint16_t erase_steps2;
extern uint32_t read_data_steps;

extern uint32_t ch_mark_cnt[maxChannel];
extern DadaStruct_TypeDef mem_data_buffer;

extern w25qxx_t w25qxx;

extern DATAHEADER_TypeDef data_header;

extern int lcd_screen_old;

extern uint8_t config_ch_erase_enable;
extern uint8_t config_ch_erase_step;

extern uint8_t batt_val10;
extern uint8_t batt_val1;

extern int usb_power_restart_timeout;

extern ApplicationTypeDef Appli_state_HS;
extern ApplicationTypeDef Appli_state_FS;

extern uint8_t export_in_progress;
extern uint8_t export_error;

extern uint8_t channelSelected;

extern uint8_t lcd_inter_busy;
extern uint8_t acc_busy;

extern int ch_status[maxChannel];
extern int ch_status_tmp[maxChannel];
extern int ch_status_tmp_confirm;
extern int ch_status_tmp_confirm2;

extern uint8_t menu_list_begin;
extern uint8_t menu_index_end;
extern uint8_t menu_index_position;
extern uint8_t menu_index_max;

extern DadaStruct3_TypeDef channel_header_current[maxChannel];
extern DadaStruct3_TypeDef channel_header_tmp[maxChannel];

extern int lcd_screen;
//typedef enum {
//	LcdScreenGraphic = 0,
//	LcdScreenMenu,
//	LcdScreenCfgCh,
//	LcdScreenCfgChCheck,
//	LcdScreenCfgClk,
//	LcdScreenMonCh,
//  LcdScreenMonChOffset,
//	LcdScreenBattChr,
//	LcdScreenLangExport,
//	LcdScreenPassword,
//  LcdScreenUsbExport,
//  LcdScreenMain,
//  LcdScreenPasswordConfig,
//  LcdScreenMemTest
//}LcdScreenTypeDef;

uint8_t screen_first_load = 1;

extern uint8_t mem_test_enabled;
extern uint8_t mem_test_channel;
extern float mem_test_channel_progress_value;
extern float mem_test_channel_progress_value_max;
extern uint8_t mem_test_channel_progress;
extern uint8_t mem_test_channel_progress_old;

extern char memTestStatus[maxChannel];

extern uint8_t lcd_show_logo;

extern uint8_t lcd_screen_previews;
extern uint8_t lcd_screen_next;

extern uint8_t password_request;

extern char password_code[6];
extern char password_code_tmp[6];
extern char password_code_tmp_str[6][4];

extern char password_config_code_tmp_str0[6][4];
extern char password_config_code_tmp_str1[6][4];
extern char password_config_code_tmp_str2[6][4];

extern char password_code_tmp0[3][6];

extern uint8_t password_error_timeout;
extern uint8_t password_error_timeout0;

extern uint8_t passwd_config_cnt;

extern uint8_t menu_config_index;

extern uint32_t uC_ID[3];

extern float V_Bat_val;
extern float V_Charger_val;
extern float V_DC_DC_val;

//extern uint32_t MEM_ID[maxChannel];
extern int MPU6050_AccAngleX;
extern int MPU6050_AccAngleY;

//extern char channelLabel[maxChannel][maxChar];
//extern char channelLabel_tmp[maxChar_x2];

//uint32_t datacsv0_size = sizeof datacsv0;
//uint32_t datacsv1_size = sizeof datacsv1;
//uint32_t datacsv2_size = sizeof datacsv2;
//uint32_t datacsv3_size = sizeof datacsv3;
//uint32_t datacsv4_size = sizeof datacsv4;
//uint32_t datacsv5_size = sizeof datacsv5;
//uint32_t datacsv6_size = sizeof datacsv6;
//uint32_t datacsv7_size = sizeof datacsv7;

extern uint32_t datacsv0_index;
extern uint32_t datacsv1_index;
extern uint32_t datacsv2_index;
extern uint32_t datacsv3_index;
extern uint32_t datacsv4_index;
extern uint32_t datacsv5_index;
extern uint32_t datacsv6_index;
extern uint32_t datacsv7_index;

extern uint8_t checksumStatusError;
extern DS18B20_TypeDef SENSORS[maxChannel];
extern DS3231_Time_t clock1;
extern DS3231_Time_t tm1;
extern ADC_HandleTypeDef hadc1;
extern uint8_t ADC_reading;
extern int read_interval[maxChannel];
extern uint32_t ADC_raw[10];
extern int ch_config_index;
extern uint8_t lcd_refresh;
extern int keyboard_timeout;
extern int keyboard_timeout2;
extern int usb_pwr_off_timeout;
extern const uint16_t Font11x18;
extern uint8_t blink;
//extern uint8_t button1_info;
extern uint8_t button1_stage;
extern int start_MSC_Application1;
extern int start_MSC_Application2;
extern uint16_t progress_usb_export;
extern uint8_t standby_mode;
extern uint32_t save_BMP_timeout;
extern uint8_t save_BMP_phase;

// for code from IT7 start
extern char str2[5];
extern uint8_t blink_cnt1;
extern uint8_t blink_cnt2;
extern uint8_t blink_passwd;
extern uint8_t blink_text;

extern uint8_t button_old;
extern uint32_t tick100ms;
extern uint32_t tick250ms;
extern uint32_t tick500ms;
extern uint32_t tick1000ms;
extern uint8_t update_info;
extern HID_KEYBD_Info_TypeDef *keybd_info1;

extern uint8_t language_current;

extern int usb_power_restart_timeout;
extern int usb_power_restart_count;
extern int usb_power_recycle_count;

extern uint16_t progress_usb_export; // 0-100

extern USBH_ClassTypeDef HID_Class;
#define USBH_HID_CLASS    &HID_Class

extern uint8_t pwrOffLcdBack;
extern uint8_t button1_restart_enabled;
extern uint8_t blink;

extern I2C_HandleTypeDef hi2c2;
//extern bool USB_FS_enabled;
extern uint8_t get_temperatures;
extern uint32_t timeout_temperature_read;
extern float temperatures[maxChannel];

extern int pwm_value;
extern int step;
extern uint32_t enumTimeout;

extern ADC_HandleTypeDef hadc1;
extern int lcd_bright_max;
extern int lcd_bright_med;
extern int lcd_bright_min;
extern int lcd_bright_off;
extern int lcd_bright_new;
extern int lcd_bright_set;
//extern int lcd_bright_old;
extern uint8_t lcd_inter_busy;
extern uint8_t lcd_loop_busy;
extern uint8_t acc_busy;
extern DS3231_Time_t tm1;
extern int update_info_seconds_old;
//extern bool update_info;
//extern bool standby_mode;
//extern bool lcd_restart;
extern uint8_t timer7_tick;
extern int pwr_off_timeout;

extern int clock_index_edit;
extern int clock_index_edit_mode;
extern int ch_config_index_edit;
extern int ch_config_index_edit_mode;
extern int ch_status_tmp[maxChannel];
//extern char str[80];

//extern int keyboard_status;
extern int keyboard_timeout_reset;
extern bool keyboard_insert;
extern int ch_config_index;

extern USBH_HandleTypeDef hUsbHostFS;

extern int keyboard_timeout;
extern int keyboard_timeout2;

extern bool lcd_updateInfo;
extern uint8_t lcd_refresh;
extern uint8_t lcd_refresh_new;
extern int lcd_phase;
extern bool lcd_enabled;
extern uint8_t init_flag;

extern const uint16_t lcd_timeout0recall;
//extern const uint16_t lcd_timeout0recall2;
extern uint8_t lcd_timeout0;

//extern uint8_t button1_info;
//extern bool blink;
extern uint8_t milisec;
//extern uint8_t milisec2;
extern uint8_t hr10;
extern uint8_t hr1;
extern uint8_t min10;
extern uint8_t min1;
extern uint8_t sec10;
extern uint8_t sec1;

extern uint8_t cycle_count;

extern int label_index_edit;
extern int label_index_edit2;
extern int lcd_screen;
extern uint8_t password_request;
extern char channelLabel[maxChannel][maxChar];
extern char channelLabel_tmp[maxChar_x2];
extern char password_code_tmp_str[6][4];
extern char password_config_code_tmp_str0[6][4];
extern char password_config_code_tmp_str1[6][4];
extern char password_config_code_tmp_str2[6][4];
extern uint8_t passwd_config_cnt;
extern uint8_t password_error_timeout;

extern int usb_pwr_off_timeout;
extern uint8_t standby_mode;

extern uint8_t mem_test_enabled;

extern uint8_t ch_exported[8];
// for code from IT7 end

extern void ADC_get(void);

extern void beep(uint8_t repetions, uint8_t is_error);

//extern USBH_StatusTypeDef USB_Force_Recovery(void);

extern void get_data_from_mem_for_main(void);
extern float NTC_Get_Temperature(int raw_value);
extern float map(float x, float in_min, float in_max, float out_min,
		float out_max);

extern uint8_t USBH_HID_GetASCIICode_Custom(HID_KEYBD_Info_TypeDef *info);

extern void password_user_clear(void);
extern void password_config_clear(void);
//float get_data0(uint32_t addr);

void draw_logo(const unsigned short *logo) {
	ILI9341_Draw_BMP(0, 0, logo, 320, 240);
}

//volatile uint8_t key_ascii_lcd = 0;
//volatile uint8_t key_lcd = 0;
//volatile uint8_t keybd_info_lshift = 0;
//volatile uint8_t keybd_info_rshift = 0;

volatile HID_KEYBD_Info_TypeDef keybd_info_lcd;

void lcd_menu_keyboard(HID_KEYBD_Info_TypeDef *keybd_info) {
	if (lcd_screen == LcdScreenUsbExport) {
		if (keybd_info_lcd.keys[0] == KEY_ESCAPE) {
			if (abort_operation == 1) {
				abort_operation = 0;
				export_error = 0;

				lcd_refresh = 3;
			} else if (export_in_progress == 1) {
				if (export_error != 0) {
					export_error = 0;
				} else {
					abort_operation = 1;
				}
			} else {
				export_in_progress = 0;
				userFunction_phase = 0;
				export_data_enabled = 0;
				progress_usb_export = 0;
				start_MSC_Application1 = 0;
				export_error = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				lcd_screen = LcdScreenMain;

				screen_first_load = 1;

				lcd_refresh = 3;
			}
		}
	}

	beep(1, 0);
}

void lcd_menu_keyboard_async(void) {
	if ((keybd_info_lcd.key_ascii == 0) && (keybd_info_lcd.keys[0] == 0)
			&& (keybd_info_lcd.rshift == 0) && (keybd_info_lcd.lshift == 0)) {
		return;
	}

	UsrLog("keybd_info_lcd key_ascii: 0x%02X", keybd_info_lcd.key_ascii);

	UsrLog("keybd_info_lcd keys[0]: 0x%02X", keybd_info_lcd.keys[0]);

	UsrLog("keybd_info_lcd rshift: 0x%02X", keybd_info_lcd.rshift);

	UsrLog("keybd_info_lcd lshift: 0x%02X", keybd_info_lcd.lshift);

	uint8_t keycode = keybd_info_lcd.key_ascii;

	uint8_t key = keybd_info_lcd.keys[0];

	back_to_main_timeout = 0;

//	if ((keybd_info->lshift == 1U) || (keybd_info->rshift == 1U)) {
//		if (key == KEY_EQUAL_PLUS) {
//			key = KEY_KEYPAD_PLUS;
//		} else if (key == KEY_MINUS_UNDERSCORE) {
//			key = KEY_KEYPAD_MINUS;
//		} else if (key == KEY_8_ASTERISK) {
//			key = KEY_KEYPAD_ASTERIKS;
//		} else if (key == KEY_8_ASTERISK) {
//			key = KEY_KEYPAD_ASTERIKS;
//		}
//	}

	uint8_t main_index_enter = 0;

	if (key != KEY_ESCAPE) {
		abort_operation = 0;
		export_error = 0;
	}

	if (export_data_enabled == 0) {
		progress_usb_export = 0;
	}

	if (keybd_info_lcd.led_num_lock == 1) {
		if ((key >= KEY_KEYPAD_1_END) && (key <= KEY_KEYPAD_0_INSERT)) {
			key -= KEY_KEYPAD_1_END - KEY_1_EXCLAMATION_MARK;
		}
	} else {
		if (key == KEY_KEYPAD_SLASH) {
			key = KEY_SLASH_QUESTION;
		} else if (key == KEY_KEYPAD_ASTERIKS) {
//			key = KEY_UPARROW;
		} else if (key == KEY_KEYPAD_MINUS) {
			key = KEY_MINUS_UNDERSCORE;
		} else if (key == KEY_KEYPAD_PLUS) {
//			key = KEY_EQUAL_PLUS;
		} else if (key == KEY_KEYPAD_ENTER) {
			key = KEY_ENTER;
		} else if (key == KEY_KEYPAD_1_END) {
			key = KEY_END1;
		} else if (key == KEY_KEYPAD_2_DOWN_ARROW) {
			key = KEY_DOWNARROW;
		} else if (key == KEY_KEYPAD_3_PAGEDN) {
			key = KEY_DOWNARROW;
		} else if (key == KEY_KEYPAD_4_LEFT_ARROW) {
			key = KEY_LEFTARROW;
		} else if (key == KEY_KEYPAD_5) {
//			key = KEY_UPARROW;
		} else if (key == KEY_KEYPAD_6_RIGHT_ARROW) {
			key = KEY_RIGHTARROW;
		} else if (key == KEY_KEYPAD_7_HOME) {
			key = KEY_UPARROW;
		} else if (key == KEY_KEYPAD_8_UP_ARROW) {
			key = KEY_UPARROW;
		} else if (key == KEY_KEYPAD_9_PAGEUP) {
			key = KEY_UPARROW;
		} else if (key == KEY_KEYPAD_0_INSERT) {
			key = KEY_INSERT;
		} else if (key == KEY_KEYPAD_DECIMAL_SEPARATOR_DELETE) {
			key = KEY_DELETE;
		}
	}

	int cnt_test_enabled = 0;

	if ((export_data_enabled == 0) && (save_BMP_phase == 0)
			&& (key == KEY_PRINTSCREEN)) {

		beep(3, 0);

		start_MSC_Application2 = 2;

		UsrLog("Save BMP");

		HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin, GPIO_PIN_RESET); // USB on

		save_BMP_timeout = HAL_GetTick();
	}

	if (lcd_screen == LcdScreenMain) {
		if ((key >= KEY_1_EXCLAMATION_MARK) && (key <= KEY_8_ASTERISK)) {
			label_index_edit = 0;

			label_index_edit2 = label_index_edit;

			ch_config_index = main_index_enter;

			ch_config_index_edit_mode = ch_config_index_edit_mode_default;

			ch_config_index = key - KEY_1_EXCLAMATION_MARK;

			channelSelected = ch_config_index;

			for (int i = 0; i < maxChar; i++) {
				channelLabel_tmp[i] = channelLabel[ch_config_index][i];
			}

			for (int i = 0; i < maxChannel; i++) {
				ch_status_tmp[i] = ch_status[i];
			}

			for (int i = 0; i < maxChannel; i++) {
				channel_header_tmp[i].start_day =
						channel_header_current[i].start_day;
				channel_header_tmp[i].start_month =
						channel_header_current[i].start_month;
				channel_header_tmp[i].start_year =
						channel_header_current[i].start_year;
				channel_header_tmp[i].start_hour =
						channel_header_current[i].start_hour;
				channel_header_tmp[i].start_minute =
						channel_header_current[i].start_minute;

				channel_header_tmp[i].stop_day =
						channel_header_current[i].stop_day;
				channel_header_tmp[i].stop_month =
						channel_header_current[i].stop_month;
				channel_header_tmp[i].stop_year =
						channel_header_current[i].stop_year;
				channel_header_tmp[i].stop_hour =
						channel_header_current[i].stop_hour;
				channel_header_tmp[i].stop_minute =
						channel_header_current[i].stop_minute;
			}

			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			// if (ch_exported[ch_config_index] == 1) {
			// lcd_screen_next = LcdScreenCfgChConfirm; // lcd_config_ch_confirm_exported();

			// ch_status_tmp_confirm = 4;

			// ch_status_tmp_confirm2 = 0;

			// lcd_refresh = 3;
			//} else {
			lcd_screen_next = LcdScreenCfgChConfirm; // LcdScreenCfgCh;

			ch_status_tmp_confirm = 4;

			ch_status_tmp_confirm2 = 0;
			//}

			screen_first_load = 1;
		} else if (key == KEY_T) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			lcd_screen_next = LcdScreenCfgClk;

			screen_first_load = 1;
		} else if ((key == KEY_C) || (key == KEY_G)) {
			int j = 0;

			for (int i = 0; i < maxChannel; i++) {
				j += ch_status[i];
			}

			if (j == 0) {
				for (int i = 0; i < maxChannel; i++) {
					dataColor[i] = dataColorRecall[i];
				}
			} else {
				for (int i = 0; i < maxChannel; i++) {
					if (ch_status[i] == 1) {
						dataColor[i] = dataColorRecall[i];
					} else {
						dataColor[i] = ILI9341_DARKGREY;
					}
				}
			}

			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			lcd_screen_next = LcdScreenGraphic;

			screen_first_load = 1;
		} else if (key == KEY_S) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			lcd_screen_next = LcdScreenMonCh;

			screen_first_load = 1;
		} else if (key == KEY_E) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			lcd_screen_next = LcdScreenUsbExport;

			screen_first_load = 1;
		} else if (key == KEY_P) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			lcd_screen_next = LcdScreenPasswordConfig;

			screen_first_load = 1;

			password_config_clear();

			passwd_config_cnt = 0;
		} else if (key == KEY_V) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			for (int i = 0; i < maxChannel; i++) {
				memTestStatus[i] = 0;

				test_ch[i] = 0;
			}

			lcd_screen_next = LcdScreenMemTest;

			screen_first_load = 1;
		} else if (key == KEY_L) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenPassword;

			password_user_clear();

			language_current_tmp = language_current;

			lcd_screen_next = LcdScreenLanguage;

			screen_first_load = 1;
		} else if (key == KEY_ESCAPE) {
			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenMain;

			lcd_screen_next = LcdScreenMain;

			screen_first_load = 1;

			if (menu_list_begin == 0) {
				menu_list_begin = (menu_index_max - 3);
			} else if (menu_list_begin == (menu_index_max - 3)) {
				menu_list_begin = 0;
			} else {
				menu_list_begin = 0;
			}
		} else if (key == KEY_UPARROW) {
			if (menu_index_position > 0) {
				menu_index_position--;
			} else {
				if (menu_list_begin > 0) {
					menu_list_begin--;
				} else {
					menu_index_position = 7;
					menu_list_begin = (menu_index_max - 3);
				}
			}
		} else if (key == KEY_RIGHTARROW) {
			menu_index_position = 7;
			menu_list_begin = (menu_index_max - 3);
		} else if (key == KEY_LEFTARROW) {
			menu_index_position = 0;
			menu_list_begin = 0;
		} else if (key == KEY_DOWNARROW) {
			if (menu_index_position < 7) {
				menu_index_position++;
			} else {
				if (menu_list_begin < (menu_index_max - 3)) {
					menu_list_begin++;
				} else {
					menu_index_position = 0;
					menu_list_begin = 0;
				}
			}
		} else if (key == KEY_PAGEUP) {
			menu_list_begin = 0;
		} else if (key == KEY_PAGEDOWN) {
			menu_list_begin = (menu_index_max - 3);
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			main_index_enter = menu_list_begin + menu_index_position;

			if ((main_index_enter >= 0) && (main_index_enter <= 7)) {
				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				lcd_screen_next = LcdScreenCfgChConfirm; // LcdScreenCfgCh;

				ch_status_tmp_confirm = 4;

				ch_status_tmp_confirm2 = 0;

				screen_first_load = 1;

				label_index_edit = 0;

				label_index_edit2 = label_index_edit;

				ch_config_index_edit_mode = ch_config_index_edit_mode_default;

				ch_config_index = main_index_enter;

				channelSelected = ch_config_index;

				for (int i = 0; i < maxChar; i++) {
					channelLabel_tmp[i] = channelLabel[ch_config_index][i];
				}

				for (int i = 0; i < maxChannel; i++) {
					channel_header_tmp[i].start_day =
							channel_header_current[i].start_day;
					channel_header_tmp[i].start_month =
							channel_header_current[i].start_month;
					channel_header_tmp[i].start_year =
							channel_header_current[i].start_year;
					channel_header_tmp[i].start_hour =
							channel_header_current[i].start_hour;
					channel_header_tmp[i].start_minute =
							channel_header_current[i].start_minute;

					channel_header_tmp[i].stop_day =
							channel_header_current[i].stop_day;
					channel_header_tmp[i].stop_month =
							channel_header_current[i].stop_month;
					channel_header_tmp[i].stop_year =
							channel_header_current[i].stop_year;
					channel_header_tmp[i].stop_hour =
							channel_header_current[i].stop_hour;
					channel_header_tmp[i].stop_minute =
							channel_header_current[i].stop_minute;
				}
			} else if (main_index_enter == (menuListClockAdjust + 8)) {
				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				lcd_screen_next = LcdScreenCfgClk;

				screen_first_load = 1;
			} else if (main_index_enter == (menuListGraphic + 8)) {
				int j = 0;

				for (int i = 0; i < maxChannel; i++) {
					j += ch_status[i];
				}

				if (j == 0) {
					for (int i = 0; i < maxChannel; i++) {
						dataColor[i] = dataColorRecall[i];
					}
				} else {
					for (int i = 0; i < maxChannel; i++) {
						if (ch_status[i] == 1) {
							dataColor[i] = dataColorRecall[i];
						} else {
							dataColor[i] = ILI9341_DARKGREY;
						}
					}
				}

				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				lcd_screen_next = LcdScreenGraphic;

				screen_first_load = 1;
			} else if (main_index_enter == (menuListSensors + 8)) {
				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				lcd_screen_next = LcdScreenMonCh;

				screen_first_load = 1;
			} else if (main_index_enter == (menuListExportData + 8)) {
				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				lcd_screen_next = LcdScreenUsbExport;

				screen_first_load = 1;
			} else if (main_index_enter == (menuListSetPassword + 8)) {
				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				lcd_screen_next = LcdScreenPasswordConfig;

				screen_first_load = 1;

				password_config_clear();

				passwd_config_cnt = 0;
			} else if (main_index_enter == (menuListMemoryTest + 8)) {
				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				for (int i = 0; i < maxChannel; i++) {
					memTestStatus[i] = 0;

					test_ch[i] = 0;
				}

				lcd_screen_next = LcdScreenMemTest;

				screen_first_load = 1;

			} else if (main_index_enter == (menuListMemoryTest + 9)) {

				lcd_screen_previews = lcd_screen;

				lcd_screen = LcdScreenPassword;

				password_user_clear();

				language_current_tmp = language_current;

				lcd_screen_next = LcdScreenLanguage;

				screen_first_load = 1;
			}
		}

		lcd_refresh = 3;
	} else if (lcd_screen == LcdScreenLanguage) {
		// language_current  language_current_tmp
		if (key == KEY_DOWNARROW) {
			if (language_screen_index < 5) {
				if (language_screen_index == 4) {
					language_screen_index = 0;
				} else {
					language_screen_index++;
				}
			} else {
				if (language_screen_index == 8) {
					language_screen_index = 5;
				} else {
					language_screen_index++;
				}
			}

			lcd_refresh = 3;
		} else if (key == KEY_UPARROW) {
			if (language_screen_index >= 5) {
				if (language_screen_index == 5) {
					language_screen_index = 8;
				} else {
					language_screen_index--;
				}
			} else {
				if (language_screen_index == 0) {
					language_screen_index = 4;
				} else {
					language_screen_index--;
				}
			}

			lcd_refresh = 3;
		} else if (key == KEY_RIGHTARROW) {
			if (language_screen_index < 5) {
				if (language_screen_index == 0) {
					language_screen_index = 5;
				} else {
					language_screen_index += 4;
				}
			} else if (language_screen_index > 4) {
				language_screen_index -= 4;
			}
		} else if (key == KEY_LEFTARROW) {
			if (language_screen_index < 5) {
				if (language_screen_index == 0) {
					language_screen_index = 5;
				} else {
					language_screen_index += 4;
				}
			} else if (language_screen_index > 4) {
				language_screen_index -= 4;
			}
		} else if (key == KEY_ESCAPE) {
			lcd_screen_previews = LcdScreenMain;

			lcd_screen = LcdScreenMain;

			lcd_screen_next = LcdScreenMain;

			screen_first_load = 1;

			lcd_refresh = 3;
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			if (language_screen_index < 3) {
				language_current = language_screen_index;
			} else {
				if (language_screen_index == 3) {
					decimal_separator = 0;
				} else if (language_screen_index == 4) {
					decimal_separator = 1;
				} else if (language_screen_index == 5) {
					date_format = 0;
				} else if (language_screen_index == 6) {
					date_format = 1;
				} else if (language_screen_index == 7) {
					temperature_unit = 0;
				} else if (language_screen_index == 8) {
					temperature_unit = 1;
				}
			}

//			ConfigStruct_TypeDef conf_dat;
//
//			W25Q64_read_config(&conf_dat);
//
//			conf_dat.DateFormat = date_format;
//
//			conf_dat.DecimalSeparator = decimal_separator;
//
//			conf_dat.TemperatureUnit = temperature_unit;
//
//			conf_dat.Language = language_current;
//
//			W25Q64_update_config(&conf_dat);

			lcd_refresh = 3;
		}
	} else if (lcd_screen == LcdScreenMemTest) {
		if ((key >= KEY_1_EXCLAMATION_MARK) && (key <= KEY_8_ASTERISK)) {
			export_error = 0;

			if (test_ch[key - KEY_1_EXCLAMATION_MARK] == 0) {
				test_ch[key - KEY_1_EXCLAMATION_MARK] = 1;
			} else {
				test_ch[key - KEY_1_EXCLAMATION_MARK] = 0;
			}

			lcd_refresh = 3;
		} else if ((key == KEY_A) || (key == KEY_T)) {
			export_error = 0;

			for (int i = 0; i < maxChannel; i++) {
				test_ch[i] = 1;
			}

			lcd_refresh = 3;
		} else if (key == KEY_N) {
			export_error = 0;

			for (int i = 0; i < maxChannel; i++) {
				test_ch[i] = 0;
			}

			lcd_refresh = 3;
		} else if (key == KEY_ESCAPE) {
			if (mem_test_enabled == 1) {
				mem_test_enabled = 0;

				mem_test_channel = 0;

				for (int i = 0; i < maxChannel; i++) {
					memTestStatus[i] = 0;
				}
			} else {
				lcd_screen_previews = LcdScreenMain;

				lcd_screen = LcdScreenMain;

				lcd_screen_next = LcdScreenMain;

				screen_first_load = 1;
			}

			lcd_refresh = 3;
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			for (int i = 0; i < maxChannel; i++) {
				if (test_ch[i] == 1) {
					cnt_test_enabled++;
				}
			}

			if (cnt_test_enabled > 0) {
				if (mem_test_enabled == 0) {
					mem_test_enabled = 1;
					mem_test_channel = 0;
				}
			}

			lcd_refresh = 3;
		}
	} else if (lcd_screen == LcdScreenUsbExport) {
		if ((key >= KEY_1_EXCLAMATION_MARK) && (key <= KEY_8_ASTERISK)) {
			export_error = 0;

			if (export_ch[key - KEY_1_EXCLAMATION_MARK] == 0) {
				export_ch[key - KEY_1_EXCLAMATION_MARK] = 1;
			} else {
				export_ch[key - KEY_1_EXCLAMATION_MARK] = 0;
			}

			lcd_refresh = 3;
		} else if ((key == KEY_A) || (key == KEY_T)) {
			export_error = 0;

			for (int i = 0; i < maxChannel; i++) {
				export_ch[i] = 1;
			}

			lcd_refresh = 3;
		} else if (key == KEY_N) {
			export_error = 0;

			for (int i = 0; i < maxChannel; i++) {
				export_ch[i] = 0;
			}

			lcd_refresh = 3;
		} else if (key == KEY_F) {
			export_error = 0;

			if (file_type == datFile16) {
				file_type = datFile64;
#ifdef output_csv_file
			} else if (file_type == datFile64) {
				file_type = csvFile;

				file_cnt_csv = 15;
#endif
			} else {
				file_type = datFile16;
			}

			lcd_refresh = 3;
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			if (save_BMP_enabled == 0) {
				if (Appli_state_HS == APPLICATION_READY) {
					export_error = 0;

					progress_usb_export = 0;

					while (lcd_inter_busy == TRUE) {
						IWDG_delay_ms(0);
					}

					lcd_progress_bar();

					start_MSC_Application1 = 2;

					UsrLog("Export data");
				} else {
					export_error = 99;

					lcd_progress_bar();
				}
			} else {
				beep(2, 0);
			}
		}
	} else if (lcd_screen == LcdScreenPasswordConfig) {
		if (key == KEY_ESCAPE) {
			if (passwd_esc_cnt0 < 1) {
				passwd_esc_cnt0++;

				password_config_clear();

				lcd_refresh = 3;

				passwd_config_cnt = 0;
			} else {
				passwd_esc_cnt0 = 0;

				lcd_screen = LcdScreenMain;

				screen_first_load = 1;

				lcd_refresh = 3;
			}

		} else {
			passwd_esc_cnt0 = 0;

			if ((keycode >= '0') && (keycode <= '9')) {
				if (password_error_timeout0 == 0) {
					for (int i = 0; i < 6; i++) {
						if (password_code_tmp0[passwd_config_cnt][i] == 0) {

							if (passwd_config_cnt == 0) {
								sprintf(password_config_code_tmp_str0[i],
										"[*]");
								password_code_tmp0[passwd_config_cnt][i] =
										keycode;
							} else if (passwd_config_cnt == 1) {
								sprintf(password_config_code_tmp_str1[i],
										"[*]");
								password_code_tmp0[passwd_config_cnt][i] =
										keycode;
							} else {
								sprintf(password_config_code_tmp_str2[i],
										"[*]");
								password_code_tmp0[passwd_config_cnt][i] =
										keycode;
							}

							if (i == 5) {
								if (passwd_config_cnt == 0) {
									if ((password_code_tmp0[passwd_config_cnt][0]
											== password_code[0])
											&& (password_code_tmp0[passwd_config_cnt][1]
													== password_code[1])
											&& (password_code_tmp0[passwd_config_cnt][2]
													== password_code[2])
											&& (password_code_tmp0[passwd_config_cnt][3]
													== password_code[3])
											&& (password_code_tmp0[passwd_config_cnt][4]
													== password_code[4])
											&& (password_code_tmp0[passwd_config_cnt][5]
													== password_code[5])) {
										passwd_config_cnt = 1;

										lcd_refresh = 3;
									} else {
										passwd_config_cnt = 4;

										lcd_refresh = 3;
									}
								} else {
									if (passwd_config_cnt < 2) {
										passwd_config_cnt++;
									} else {
										if ((password_code_tmp0[1][0]
												== password_code_tmp0[2][0])
												&& (password_code_tmp0[1][1]
														== password_code_tmp0[2][1])
												&& (password_code_tmp0[1][2]
														== password_code_tmp0[2][2])
												&& (password_code_tmp0[1][3]
														== password_code_tmp0[2][3])
												&& (password_code_tmp0[1][4]
														== password_code_tmp0[2][4])
												&& (password_code_tmp0[1][5]
														== password_code_tmp0[2][5])) {
											passwd_config_cnt = 3;

											for (int j = 0; j < 6; j++) {
												password_code[j] =
														password_code_tmp0[1][j];

												save_new_passwd = 1;
											}
										} else {
											passwd_config_cnt = 4;
										}
										lcd_refresh = 3;
									}

									lcd_refresh = 3;
								}
							}
							break;
						}
					}
				}
			}
		}
	} else if (lcd_screen == LcdScreenPassword) {
		if (key == KEY_ESCAPE) {
			if (passwd_esc_cnt < 1) {
				passwd_esc_cnt++;

				password_user_clear();

				lcd_refresh = 3;
			} else {
				passwd_esc_cnt = 0;

				lcd_screen = lcd_screen_previews;

				lcd_refresh = 3;
			}

		} else {
			passwd_esc_cnt = 0;

			if ((keycode >= '0') && (keycode <= '9')) {
				if (password_error_timeout == 0) {
					for (int i = 0; i < 6; i++) {
						if (password_code_tmp[i] == 0) {
							sprintf(password_code_tmp_str[i], "[*]");
							password_code_tmp[i] = keycode;

							if (i == 5) {
								if ((password_code_tmp[0] == password_code[0])
										&& (password_code_tmp[1]
												== password_code[1])
										&& (password_code_tmp[2]
												== password_code[2])
										&& (password_code_tmp[3]
												== password_code[3])
										&& (password_code_tmp[4]
												== password_code[4])
										&& (password_code_tmp[5]
												== password_code[5])) {
									lcd_screen = lcd_screen_next;

									screen_first_load = 1;

									lcd_refresh = 3;
								} else {
									password_error_timeout = 20;
								}
							}
							break;
						}
					}
				}
			}

			if ((keycode >= '0') && (keycode <= '9')) {
				if (keycode == passwd_recall[passwd_recall_count]) {
					if (passwd_recall_count == ((sizeof passwd_recall) - 2)) {
						UsrLog("Recall password: 0-1-2-3-4-5");

						for (int i = 0; i <= 5; i++) {
							password_code[i] = '1';
						}

						save_new_passwd = 1;

						passwd_recall_count = 0;
					}
					passwd_recall_count++;
					//UsrLog("Recall password: %d", passwd_recall_count);
				} else {
					passwd_recall_count = 0;
					//UsrLog("Recall password: %d", passwd_recall_count);
				}
			}
		}
	} else if (lcd_screen == LcdScreenGraphic) {
		if ((keycode >= '1') && (keycode <= '8')) {
			if (dataColor[keycode - '1'] == dataColorRecall[keycode - '1']) {
				dataColor[keycode - '1'] = ILI9341_DARKGREY;
			} else {
				dataColor[keycode - '1'] = dataColorRecall[keycode - '1'];
			}

			channelSelected = keycode - '1';
		} else if (keycode == '9') {
			for (int i = 0; i < maxChannel; i++) {
				dataColor[i] = dataColorRecall[i];
			}
		} else if (keycode == '0') {
			for (int i = 0; i < maxChannel; i++) {
				dataColor[i] = ILI9341_DARKGREY;
			}
		} else if (key == KEY_KEYPAD_PLUS) {
			for (uint8_t i = 0; i < maxChannel; i++) {
				if (dataColor[i] == dataColorRecall[i]) {
					if (chart_zoom[i] == Zoom_Week) {
						if (chart_zoom_week[i] < 9) {
							chart_zoom_week[i]++;
						}
					} else if (chart_zoom[i] == Zoom_Day) {
						if (chart_zoom_day[i] < 5) {
							chart_zoom_day[i]++;
						}
					} else if (chart_zoom[i] == Zoom_Hour) {
						if (chart_zoom_hour[i] < 22) {
							chart_zoom_hour[i]++;
						}
					}
				}
			}
		} else if (key == KEY_KEYPAD_MINUS) {
			for (uint8_t i = 0; i < maxChannel; i++) {
				if (dataColor[i] == dataColorRecall[i]) {
					if (chart_zoom[i] == Zoom_Week) {
						if (chart_zoom_week[i] > 0) {
							chart_zoom_week[i]--;
						}
					} else if (chart_zoom[i] == Zoom_Day) {
						if (chart_zoom_day[i] > 0) {
							chart_zoom_day[i]--;
						}
					} else if (chart_zoom[i] == Zoom_Hour) {
						if (chart_zoom_hour[i] > 0) {
							chart_zoom_hour[i]--;
						}
					}
				}
			}
		} else if (key == KEY_Z) {
			for (uint8_t i = 0; i < maxChannel; i++) {
				if (dataColor[i] == dataColorRecall[i]) {
					if (chart_zoom_enable[i] > 0) {
						chart_zoom_enable[i] = 0;
					} else {
						chart_zoom_enable[i] = 1;
					}
				}
			}
		} else if (key == KEY_W) {
			for (uint8_t i = 0; i < maxChannel; i++) {
				if (dataColor[i] == dataColorRecall[i]) {
					chart_zoom[i] = Zoom_Week;
				}
			}
		} else if (key == KEY_D) {
			for (uint8_t i = 0; i < maxChannel; i++) {
				if (dataColor[i] == dataColorRecall[i]) {
					chart_zoom[i] = Zoom_Day;
				}
			}
		} else if (key == KEY_H) {
			for (uint8_t i = 0; i < maxChannel; i++) {
				if (dataColor[i] == dataColorRecall[i]) {
					chart_zoom[i] = Zoom_Hour;
				}
			}
		} else if (key == KEY_ESCAPE) {
			lcd_screen = LcdScreenMain;

			lcd_screen_next = LcdScreenMain;

			screen_first_load = 1;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off
		}

		lcd_refresh = 0xFF;
	} else if (lcd_screen == LcdScreenCfgCh) {
		if (key == KEY_ESCAPE) {
			int j = 0;

			for (int i = 0; i < maxChar; i++) {
				if (channelLabel[ch_config_index][i] != channelLabel_tmp[i]) {
					j++;
				}
			}

			if (j > 0) {
				lcd_screen = LcdScreenCfgChConfirm;

				ch_status_tmp_confirm = 3;

				ch_status_tmp_confirm2 = 0;

				lcd_refresh = 3;
			} else {
				lcd_screen = LcdScreenMain;

				lcd_screen_next = lcd_screen;

				screen_first_load = 1;

				lcd_refresh = 3;
			}
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			if (ch_config_index_edit_mode == ch_config_index_edit_mode_text) {
				lcd_screen = LcdScreenCfgChConfirm;

				ch_status_tmp_confirm = 2;

				ch_status_tmp_confirm2 = 0;

				lcd_refresh = 3;
			} else {
				if (ch_status_tmp[ch_config_index] == 0) {
					ch_status_tmp_confirm = 1;
				} else {
					ch_status_tmp_confirm = 0;
				}

				lcd_screen = LcdScreenCfgChConfirm;

				ch_status_tmp_confirm2 = 0;

				lcd_refresh = 3;
			}
		} else if ((key >= KEY_F1) && (key <= KEY_F8)) {
			for (int i = 0; i < maxChar; i++) {
				channelLabel[ch_config_index][i] = channelLabel_tmp[i];
			}

			lcd_screen = LcdScreenCfgChConfirm;

			ch_status_tmp_confirm = 4;

			screen_first_load = 1;

			label_index_edit = 0;

			ch_config_index = key - KEY_F1;

			ch_config_index_edit_mode = ch_config_index_edit_mode_default;

			channelSelected = ch_config_index;

			for (int i = 0; i < maxChar; i++) {
				channelLabel_tmp[i] = channelLabel[ch_config_index][i];
			}

			for (int i = 0; i < maxChannel; i++) {
				ch_status_tmp[i] = ch_status[i];
			}

			lcd_refresh = 3;

		} else if (key == KEY_TAB) {
			if (ch_status[ch_config_index] == 0) {
				if (ch_config_index_edit_mode == ch_config_index_edit_mode_text) {
					ch_config_index_edit = 0;

					ch_config_index_edit_mode =
					ch_config_index_edit_mode_status;

					UsrLog("ch_config_index_edit_mode = 1");
				} else if (ch_config_index_edit_mode
						== ch_config_index_edit_mode_status) {
					ch_config_index_edit = 0;

					ch_config_index_edit_mode = ch_config_index_edit_mode_text;

					UsrLog("ch_config_index_edit_mode = 0");
				}
			}

			lcd_refresh = 3;
		} else {
			if (ch_config_index_edit_mode == ch_config_index_edit_mode_text) {
				text_editor(channelLabel_tmp);

				label_index_edit2 = 99; // force refresh
			}
		}
	} else if (lcd_screen == LcdScreenCfgChConfirm) {
		if (key == KEY_ESCAPE) {
			lcd_screen = LcdScreenCfgCh;
		} else if ((key == KEY_RIGHTARROW) || (key == KEY_LEFTARROW)) {
			if (ch_status_tmp_confirm2 == 0) {
				ch_status_tmp_confirm2 = 1;
			} else {
				ch_status_tmp_confirm2 = 0;
			}
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			if (ch_config_index_edit_mode == ch_config_index_edit_mode_text) {
				if (ch_status_tmp_confirm2 == 1) {
					for (int i = 0; i < maxChar; i++) {
						channelLabel[ch_config_index][i] = channelLabel_tmp[i];
					}

					save_channel_label = 1;

				} else {
					if (ch_status_tmp_confirm == 3) {
						for (int i = 0; i < maxChar; i++) {
							channelLabel_tmp[i] =
									channelLabel[ch_config_index][i];
						}
					}
				}

				lcd_screen = LcdScreenCfgCh;
			} else {
				if (ch_status_tmp_confirm2 == 1) {
					if (ch_status_tmp_confirm == 1) {
						lcd_screen = LcdScreenCfgChMemErase;

						config_ch_erase_enable = 1;

						config_ch_erase_step = 0;
					} else {
						lcd_screen = LcdScreenCfgCh;

						save_channel_stop = 1;
					}

					ch_status_tmp[ch_config_index] = ch_status_tmp_confirm;

					channel_header_current[ch_config_index].start_day =
							channel_header_tmp[ch_config_index].start_day;
					channel_header_current[ch_config_index].start_month =
							channel_header_tmp[ch_config_index].start_month;
					channel_header_current[ch_config_index].start_year =
							channel_header_tmp[ch_config_index].start_year;
					channel_header_current[ch_config_index].start_hour =
							channel_header_tmp[ch_config_index].start_hour;
					channel_header_current[ch_config_index].start_minute =
							channel_header_tmp[ch_config_index].start_minute;

					channel_header_current[ch_config_index].stop_day =
							channel_header_tmp[ch_config_index].stop_day;
					channel_header_current[ch_config_index].stop_month =
							channel_header_tmp[ch_config_index].stop_month;
					channel_header_current[ch_config_index].stop_year =
							channel_header_tmp[ch_config_index].stop_year;
					channel_header_current[ch_config_index].stop_hour =
							channel_header_tmp[ch_config_index].stop_hour;
					channel_header_current[ch_config_index].stop_minute =
							channel_header_tmp[ch_config_index].stop_minute;

					ch_status[ch_config_index] = ch_status_tmp[ch_config_index];

					SENSORS[ch_config_index].FLAG_ENABLED =
							ch_status_tmp[ch_config_index];
				} else { // ch_status_tmp_confirm == 4 & other
					lcd_screen = LcdScreenCfgCh;
				}
			}
		}

		lcd_refresh = 3;
	} else if (lcd_screen == LcdScreenCfgChMemErase) {
		if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			if (config_ch_erase_enable == 0) {
				lcd_screen = LcdScreenCfgCh;

				lcd_refresh = 3;
			}
		}
	} else if (lcd_screen == LcdScreenCfgClk) {
		if (key == KEY_ESCAPE) {
			lcd_screen = LcdScreenMain;

			lcd_screen_next = lcd_screen;

			screen_first_load = 1;

			lcd_refresh = 3;

			if (batt_error_esc < 3) {
				batt_error_esc++;
			}
		} else if (key == KEY_TAB) {
			if (clock_index_edit_mode == 0) {
				clock_index_edit = 0;

				clock_index_edit_mode = 1;
			} else if (clock_index_edit_mode == 1) {
				clock_index_edit = 0;

				clock_index_edit_mode = 0;
			}
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			lcd_config_clock_cursor_move();
		} else if (key == KEY_RIGHTARROW) {
			if (clock_index_edit_mode == 0) {
				if (clock_index_edit == 0) {
					clock_index_edit = 5;
				} else if (clock_index_edit == 5) {
					clock_index_edit = 6;
				} else if (clock_index_edit == 6) {
					clock_index_edit = 8;
				} else if (clock_index_edit == 8) {
					clock_index_edit = 9;
				} else if (clock_index_edit == 9) {
					clock_index_edit = 11;
				} else if (clock_index_edit == 11) {
					clock_index_edit = 12;
				} else if (clock_index_edit == 12) {
					clock_index_edit = 0;
				}
			} else if (clock_index_edit_mode == 1) {
				if (clock_index_edit == 0) {
					clock_index_edit = 1;
				} else if (clock_index_edit == 1) {
					clock_index_edit = 3;
				} else if (clock_index_edit == 3) {
					clock_index_edit = 4;
				} else if (clock_index_edit == 4) {
					clock_index_edit = 6;
				} else if (clock_index_edit == 6) {
					clock_index_edit = 7;
				} else if (clock_index_edit == 7) {
					clock_index_edit = 0;
				}
			}
		} else if (key == KEY_LEFTARROW) {
			if (clock_index_edit_mode == 0) {
				if (clock_index_edit == 0) {
					clock_index_edit = 12;
				} else if (clock_index_edit == 12) {
					clock_index_edit = 11;
				} else if (clock_index_edit == 11) {
					clock_index_edit = 9;
				} else if (clock_index_edit == 9) {
					clock_index_edit = 8;
				} else if (clock_index_edit == 8) {
					clock_index_edit = 6;
				} else if (clock_index_edit == 6) {
					clock_index_edit = 5;
				} else if (clock_index_edit == 5) {
					clock_index_edit = 0;
				}
			} else if (clock_index_edit_mode == 1) {
				if (clock_index_edit == 0) {
					clock_index_edit = 7;
				} else if (clock_index_edit == 7) {
					clock_index_edit = 6;
				} else if (clock_index_edit == 6) {
					clock_index_edit = 4;
				} else if (clock_index_edit == 4) {
					clock_index_edit = 3;
				} else if (clock_index_edit == 3) {
					clock_index_edit = 1;
				} else if (clock_index_edit == 1) {
					clock_index_edit = 0;
				}
			}
		} else if (key == KEY_UPARROW) {
			if (clock_index_edit_mode == 0) {
				if (clock_index_edit == 0) {
					if (clock1.day < 7) {
						clock1.day++;
					} else {
						clock1.day = 1;
					}
				} else if (clock_index_edit == 5) {
					if ((clock1.date + 10) < 31) {
						clock1.date += 10;
					} else {
						clock1.date = 31;
					}
				} else if (clock_index_edit == 6) {
					if (clock1.date < 31) {
						clock1.date++;
					} else {
						clock1.date = 1;
					}
				} else if (clock_index_edit == 8) {
					if ((clock1.month + 10) < 12) {
						clock1.month += 10;
					} else {
						clock1.month = 12;
					}
				} else if (clock_index_edit == 9) {
					if (clock1.month < 12) {
						clock1.month++;
					} else {
						clock1.month = 1;
					}
				} else if (clock_index_edit == 11) {
					if ((clock1.year + 10) < 99) {
						clock1.year += 10;
					} else {
						clock1.year = 99;
					}
				} else if (clock_index_edit == 12) {
					if (clock1.year < 99) {
						clock1.year++;
					} else {
						clock1.year = 0;
					}
				}
			} else if (clock_index_edit_mode == 1) {
				if (clock_index_edit == 0) {
					if ((clock1.hours + 10) < 23) {
						clock1.hours += 10;
					} else {
						clock1.hours = 23;
					}
				} else if (clock_index_edit == 1) {
					if (clock1.hours < 23) {
						clock1.hours++;
					} else {
						clock1.hours = 0;
					}
				} else if (clock_index_edit == 3) {
					if ((clock1.minutes + 10) < 59) {
						clock1.minutes += 10;
					} else {
						clock1.minutes = 59;
					}
				} else if (clock_index_edit == 4) {
					if (clock1.minutes < 59) {
						clock1.minutes++;
					} else {
						clock1.minutes = 0;
					}
				} else if (clock_index_edit == 6) {
					if ((clock1.seconds + 10) < 59) {
						clock1.seconds += 10;
					} else {
						clock1.seconds = 59;
					}
				} else if (clock_index_edit == 7) {
					if (clock1.seconds < 59) {
						clock1.seconds++;
					} else {
						clock1.seconds = 0;
					}
				}
			}
		} else if (key == KEY_DOWNARROW) {
			if (clock_index_edit_mode == 0) {
				if (clock_index_edit == 0) {
					if (clock1.day > 1) {
						clock1.day--;
					} else {
						clock1.day = 7;
					}
				} else if (clock_index_edit == 5) {
					if ((clock1.date - 10) >= 1) {
						clock1.date -= 10;
					} else {
						clock1.date = 1;
					}
				} else if (clock_index_edit == 6) {
					if (clock1.date > 1) {
						clock1.date--;
					} else {
						clock1.date = 31;
					}
				} else if (clock_index_edit == 8) {
					if ((clock1.month - 10) > 1) {
						clock1.month -= 10;
					} else {
						clock1.month = 1;
					}
				} else if (clock_index_edit == 9) {
					if (clock1.month > 1) {
						clock1.month--;
					} else {
						clock1.month = 12;
					}
				} else if (clock_index_edit == 11) {
					if ((clock1.year - 10) > 0) {
						clock1.year -= 10;
					} else {
						clock1.year = 0;
					}
				} else if (clock_index_edit == 12) {
					if (clock1.year > 0) {
						clock1.year--;
					} else {
						clock1.year = 99;
					}
				}
			} else if (clock_index_edit_mode == 1) {
				if (clock_index_edit == 0) {
					if ((clock1.hours - 10) > 0) {
						clock1.hours -= 10;
					} else {
						clock1.hours = 0;
					}
				} else if (clock_index_edit == 1) {
					if (clock1.hours > 0) {
						clock1.hours--;
					} else {
						clock1.hours = 23;
					}
				} else if (clock_index_edit == 3) {
					if ((clock1.minutes - 10) > 0) {
						clock1.minutes -= 10;
					} else {
						clock1.minutes = 0;
					}
				} else if (clock_index_edit == 4) {
					if (clock1.minutes > 0) {
						clock1.minutes--;
					} else {
						clock1.minutes = 59;
					}
				} else if (clock_index_edit == 6) {
					if ((clock1.seconds - 10) > 0) {
						clock1.seconds -= 10;
					} else {
						clock1.seconds = 0;
					}
				} else if (clock_index_edit == 7) {
					if (clock1.seconds > 0) {
						clock1.seconds--;
					} else {
						clock1.seconds = 59;
					}
				}
			}
		} else if ((keycode >= '0') && (keycode <= '9')) {
			uint8_t keycode_num = keycode - 48;
			if (clock_index_edit_mode == 0) {
				if (clock_index_edit == 0) {
					if ((keycode_num >= 1) && (keycode_num <= 7)) {
						clock1.day = keycode_num;
					}
				} else if (clock_index_edit == 5) {
					if ((keycode_num >= 0) && (keycode_num <= 3)) {
						uint8_t date10_tmp = clock1.date / 10;
						uint8_t date1_tmp = clock1.date - (date10_tmp * 10);
						clock1.date = date1_tmp + (keycode_num * 10);
						if (clock1.date > 31)
							clock1.date = 31;
					}
				} else if (clock_index_edit == 6) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t date10_tmp = clock1.date / 10;
						clock1.date = (date10_tmp * 10) + keycode_num;
						if (clock1.date > 31)
							clock1.date = 31;
					}
				} else if (clock_index_edit == 8) {
					if ((keycode_num >= 0) && (keycode_num <= 1)) {
						uint8_t month10_tmp = clock1.month / 10;
						uint8_t month1_tmp = clock1.month - (month10_tmp * 10);
						clock1.month = month1_tmp + (keycode_num * 10);
						if (clock1.month > 12)
							clock1.month = 12;
					}
				} else if (clock_index_edit == 9) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t month10_tmp = clock1.month / 10;
						clock1.month = (month10_tmp * 10) + keycode_num;
						if (clock1.month > 12)
							clock1.month = 12;
					}
				} else if (clock_index_edit == 11) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t year10_tmp = clock1.year / 10;
						uint8_t year1_tmp = clock1.year - (year10_tmp * 10);
						clock1.year = year1_tmp + (keycode_num * 10);
					}
				} else if (clock_index_edit == 12) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t year10_tmp = clock1.year / 10;
						clock1.year = (year10_tmp * 10) + keycode_num;
					}
				}
			} else if (clock_index_edit_mode == 1) {
				if (clock_index_edit == 0) {
					if ((keycode_num >= 0) && (keycode_num <= 2)) {
						uint8_t hours10_tmp = clock1.hours / 10;
						uint8_t hours1_tmp = clock1.hours - (hours10_tmp * 10);
						clock1.hours = hours1_tmp + (keycode_num * 10);
						if (clock1.hours > 23)
							clock1.hours = 23;
					}
				} else if (clock_index_edit == 1) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t hours10_tmp = clock1.hours / 10;
						clock1.hours = (hours10_tmp * 10) + keycode_num;
						if (clock1.hours > 23)
							clock1.hours = 23;
					}
				} else if (clock_index_edit == 3) {
					if ((keycode_num >= 0) && (keycode_num <= 5)) {
						uint8_t minutes10_tmp = clock1.minutes / 10;
						uint8_t minutes1_tmp = clock1.minutes
								- (minutes10_tmp * 10);
						clock1.minutes = minutes1_tmp + (keycode_num * 10);
						if (clock1.minutes > 59)
							clock1.minutes = 59;
					}
				} else if (clock_index_edit == 4) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t minutes10_tmp = clock1.minutes / 10;
						clock1.minutes = (minutes10_tmp * 10) + keycode_num;
						if (clock1.minutes > 59)
							clock1.minutes = 59;
					}
				} else if (clock_index_edit == 6) {
					if ((keycode_num >= 0) && (keycode_num <= 5)) {
						uint8_t seconds10_tmp = clock1.seconds / 10;
						uint8_t seconds1_tmp = clock1.seconds
								- (seconds10_tmp * 10);
						clock1.seconds = seconds1_tmp + (keycode_num * 10);
						if (clock1.seconds > 59)
							clock1.seconds = 59;
					}
				} else if (clock_index_edit == 7) {
					if ((keycode_num >= 0) && (keycode_num <= 9)) {
						uint8_t seconds10_tmp = clock1.seconds / 10;
						clock1.seconds = (seconds10_tmp * 10) + keycode_num;
						if (clock1.seconds > 59)
							clock1.seconds = 59;
					}
				}
			}

			lcd_config_clock_cursor_move();
		}

		//lcd_refresh = 0xFF;
	} else if (lcd_screen == LcdScreenMonCh) {
		if (key == KEY_ESCAPE) {
			lcd_screen = LcdScreenMain;

			lcd_screen_next = lcd_screen;

			screen_first_load = 1;

			lcd_refresh = 3;
		} else if ((keycode >= '1') && (keycode <= '8')) {
			LcdScreenMonChOffsetCh = keycode - 48;

			lcd_screen_previews = lcd_screen;

			lcd_screen = LcdScreenMonChOffset;

			lcd_refresh = 3;

			sensor_offset_tmp = sensor_offset[LcdScreenMonChOffsetCh - 1];
		}
	} else if (lcd_screen == LcdScreenMonChOffset) {
		if (key == KEY_ESCAPE) {
			lcd_screen = LcdScreenMonCh;

			lcd_screen_next = lcd_screen;

			screen_first_load = 1;
		} else if (key == KEY_Z) {
			sensor_offset_tmp = 0;
		} else if (key == KEY_U) {
			sensor_offset_tmp = 150;
		} else if (key == KEY_D) {
			sensor_offset_tmp = -150;
		} else if ((key == KEY_ENTER) || (key == KEY_KEYPAD_ENTER)) {
			sensor_offset[LcdScreenMonChOffsetCh - 1] = sensor_offset_tmp;

			ConfigStruct_TypeDef conf_dat;

			W25Q64_read_config(&conf_dat);

			int16_t conv_tmp = sensor_offset_tmp; // * 10.0;

			conv_tmp += 150;

			conf_dat.SensorOffset[LcdScreenMonChOffsetCh - 1] = conv_tmp;

			W25Q64_update_config(&conf_dat);

			lcd_screen = LcdScreenMonCh;

			lcd_screen_next = lcd_screen;

			screen_first_load = 1;

			lcd_refresh = 3;
		} else if ((keycode >= '1') && (keycode <= '8')) {
			LcdScreenMonChOffsetCh = keycode - 48;

			lcd_screen_previews = LcdScreenMonCh;

			lcd_screen = LcdScreenMonChOffset;

			lcd_refresh = 3;

			sensor_offset_tmp = sensor_offset[LcdScreenMonChOffsetCh - 1];
		} else if (key == KEY_KEYPAD_MINUS) {
			if (sensor_offset_tmp > -150) {
				sensor_offset_tmp--;
			}
		} else if (key == KEY_KEYPAD_PLUS) {
			if (sensor_offset_tmp < 150) {
				sensor_offset_tmp++;
			}
		}

		ILI9341_FillRect(102, 45, 77, 18, ILI9341_BLACK);

		float tmp_val = sensor_offset_tmp / 10.0;

		sprintf(str, " %.1f ", tmp_val);

		if (sensor_offset_tmp == sensor_offset[LcdScreenMonChOffsetCh - 1]) {
			ILI9341_WriteString(102, 45, str, Font_11x18,
			ILI9341_GREEN, ILI9341_BLACK);
		} else {
			if (sensor_offset_tmp > sensor_offset[LcdScreenMonChOffsetCh - 1]) {
				ILI9341_WriteString(102, 45, str, Font_11x18,
				ILI9341_YELLOW, ILI9341_BLACK);
			} else if (sensor_offset_tmp
					< sensor_offset[LcdScreenMonChOffsetCh - 1]) {
				ILI9341_WriteString(102, 45, str, Font_11x18,
				ILI9341_ORANGE, ILI9341_BLACK);
			}
		}
	}

	if (key != KEY_PRINTSCREEN) {
		if ((lcd_screen != LcdScreenPassword)
				&& (lcd_screen != LcdScreenMonChOffset)) {
			lcd_refresh = 3;
		}

		lcd_phase = 0;
	}

	keybd_info_lcd.key_ascii = 0;
	keybd_info_lcd.keys[0] = 0;
	keybd_info_lcd.rshift = 0;
	keybd_info_lcd.lshift = 0;
}

void add_char_to_text(uint8_t keycode, char *labelChar) {
	for (int i = (maxChar_x2 - 2); i > label_index_edit; i--) {
		labelChar[i] = labelChar[i - 1];
	}

	labelChar[label_index_edit] = keycode;  // 239:´

	if (label_index_edit < (maxChar_x2 - 2)) {
		label_index_edit++;
	}
}

//extern void text_editor(uint8_t key_lshift, uint8_t key_rshift, uint8_t key,
//		uint8_t keycode, char *labelChar);
void text_editor(char *labelChar) {
//UsrLog("keycode: %d; key: %d", keycode, key);

	uint8_t key = keybd_info_lcd.keys[0];

	uint8_t lshift = keybd_info_lcd.lshift;
	uint8_t rshift = keybd_info_lcd.rshift;

	uint8_t keycode = keybd_info_lcd.key_ascii;

	uint8_t rctrl = keybd_info_lcd.rctrl;
	uint8_t lctrl = keybd_info_lcd.lctrl;

	int i = 0;

	if ((rctrl == 1U) || (lctrl == 1U)) {

		switch (key) {
		case KEY_C:
			sprintf(channelLabel_ctrl_c, "%s", labelChar);
			break;
		case KEY_V:
			sprintf(labelChar, "%s", channelLabel_ctrl_c);
			break;
		case KEY_RIGHTARROW:
			i = label_index_edit;

			while ((i < (maxChar_x2 - 2)) & (labelChar[i] != 0)
					& (labelChar[i] != 32)) {
				i++;
			}

			if (((i + 1) < (maxChar_x2 - 2)) & (labelChar[i + 1] != 0)) {
				while (label_index_edit2 != label_index_edit) {
				}

				label_index_edit = i + 1;
			}
			break;
		case KEY_LEFTARROW:
			i = 0;

			if (label_index_edit > 0) {
				while (label_index_edit2 != label_index_edit) {
				}

				i = label_index_edit;

				int j = 1;

				if (labelChar[label_index_edit - 1] == 32) {
					j = 0;
				}

				if (labelChar[label_index_edit] == 0) {
					i--;
				}

				while ((i >= 0) & (labelChar[i] != 0) & (j < 2)) {
					i--;
					if (labelChar[i] == 32) {
						j++;
					}
				}

				if (i < 0) {
					label_index_edit = 0;
				} else if (((i + 1) < (maxChar_x2 - 2))
						& (labelChar[i + 1] != 0)) {
					while (label_index_edit2 != label_index_edit) {
					}

					label_index_edit = i + 1;
				}
			} else {

			}
			break;
		default:
			// statements
			break;
		}
	} else if (key == KEY_OBRACKET_AND_OBRACE) {
		if ((lshift == 1U) || (rshift == 1U)) {
			if (accent1 == 0) {
				accent1 = grave_accent;
			} else {
				add_char_to_text(accent1, labelChar);
				add_char_to_text(grave_accent, labelChar);

				accent1 = 0;
			}
		} else {
			if (accent1 == 0) {
				accent1 = acute_accent;
			} else {
				add_char_to_text(accent1, labelChar);
				add_char_to_text(acute_accent, labelChar);

				accent1 = 0;
			}
		}
	} else if (key == KEY_SINGLE_AND_DOUBLE_QUOTE) {
		if ((lshift == 1U) || (rshift == 1U)) {
			if (accent1 == 0) {
				accent1 = circumflex_accent;
			} else {
				add_char_to_text(accent1, labelChar);
				add_char_to_text(circumflex_accent, labelChar);

				accent1 = 0;
			}
		} else {
			if (accent1 == 0) {
				accent1 = tilde_accent;
			} else {
				add_char_to_text(accent1, labelChar);
				add_char_to_text(tilde_accent, labelChar);

				accent1 = 0;
			}
		}
	} else if ((key == KEY_6_CARET) & ((lshift == 1U) || (rshift == 1U))) {
		if (accent1 == 0) {
			accent1 = diaeresis_accent;
		} else {
			add_char_to_text(accent1, labelChar);
			add_char_to_text(diaeresis_accent, labelChar);

			accent1 = 0;
		}
	} else if (key == KEY_BACKSPACE) {
		if ((label_index_edit > 0) & (label_index_edit <= (maxChar_x2 - 2))) {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit--;

			for (int i = label_index_edit; i < (maxChar_x2 - 1); i++) {
				labelChar[i] = labelChar[i + 1];
			}
		}
	} else if (key == KEY_DELETE) {
		for (int i = label_index_edit; i < (maxChar_x2 - 1); i++) {
			labelChar[i] = labelChar[i + 1];
		}
	} else if ((keycode >= 32) & (keycode <= 127)) {
		if (accent1 != 0) {
			if ((accent1 == grave_accent)) {
				if (keycode == 'A')
					keycode = A_grave_accent;
				else if (keycode == 'E')
					keycode = E_grave_accent;
				else if (keycode == 'I')
					keycode = I_grave_accent;
				else if (keycode == 'O')
					keycode = O_grave_accent;
				else if (keycode == 'U')
					keycode = U_grave_accent;

				else if (keycode == 'a')
					keycode = a_grave_accent;
				else if (keycode == 'e')
					keycode = e_grave_accent;
				else if (keycode == 'i')
					keycode = i_grave_accent;
				else if (keycode == 'o')
					keycode = o_grave_accent;
				else if (keycode == 'u')
					keycode = u_grave_accent;
				else {
					add_char_to_text(accent1, labelChar);
				}
			} else if ((accent1 == acute_accent)) {
				if (keycode == 'A')
					keycode = A_acute_accent;
				else if (keycode == 'E')
					keycode = E_acute_accent;
				else if (keycode == 'I')
					keycode = I_acute_accent;
				else if (keycode == 'O')
					keycode = O_acute_accent;
				else if (keycode == 'U')
					keycode = U_acute_accent;

				else if (keycode == 'a')
					keycode = a_acute_accent;
				else if (keycode == 'e')
					keycode = e_acute_accent;
				else if (keycode == 'i')
					keycode = i_acute_accent;
				else if (keycode == 'o')
					keycode = o_acute_accent;
				else if (keycode == 'u')
					keycode = u_acute_accent;
				else {
					add_char_to_text(accent1, labelChar);
				}
			} else if ((accent1 == circumflex_accent)) {
				if (keycode == 'A')
					keycode = A_circumflex_accent;
				else if (keycode == 'E')
					keycode = E_circumflex_accent;
				else if (keycode == 'I')
					keycode = I_circumflex_accent;
				else if (keycode == 'O')
					keycode = O_circumflex_accent;
				else if (keycode == 'U')
					keycode = U_circumflex_accent;

				else if (keycode == 'a')
					keycode = a_circumflex_accent;
				else if (keycode == 'e')
					keycode = e_circumflex_accent;
				else if (keycode == 'i')
					keycode = i_circumflex_accent;
				else if (keycode == 'o')
					keycode = o_circumflex_accent;
				else if (keycode == 'u')
					keycode = u_circumflex_accent;
				else {
					add_char_to_text(accent1, labelChar);
				}
			} else if ((accent1 == tilde_accent)) {
				if (keycode == 'A')
					keycode = A_tilde_accent;
				else if (keycode == 'O')
					keycode = O_tilde_accent;

				else if (keycode == 'a')
					keycode = a_tilde_accent;
				else if (keycode == 'o')
					keycode = o_tilde_accent;
				else {
					add_char_to_text(accent1, labelChar);
				}
			} else if ((accent1 == diaeresis_accent)) {
				if (keycode == 'A')
					keycode = A_diaeresis_accent;
				else if (keycode == 'E')
					keycode = E_diaeresis_accent;
				else if (keycode == 'I')
					keycode = I_diaeresis_accent;
				else if (keycode == 'O')
					keycode = O_diaeresis_accent;
				else if (keycode == 'U')
					keycode = U_diaeresis_accent;

				else if (keycode == 'a')
					keycode = a_diaeresis_accent;
				else if (keycode == 'e')
					keycode = e_diaeresis_accent;
				else if (keycode == 'i')
					keycode = i_diaeresis_accent;
				else if (keycode == 'o')
					keycode = o_diaeresis_accent;
				else if (keycode == 'u')
					keycode = u_diaeresis_accent;
				else {
					add_char_to_text(accent1, labelChar);
				}
			}

			add_char_to_text(keycode, labelChar);

			accent1 = 0;
		} else {
			add_char_to_text(keycode, labelChar);
		}

		USBH_DbgLog("Key pressed: %c", keycode);
	} else if (key == KEY_RIGHTARROW) {
		if ((labelChar[label_index_edit] != 0)
				& (label_index_edit < (maxChar_x2 - 2))) {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit++;
		} else {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit = 0;
		}
	} else if (key == KEY_LEFTARROW) {
		if (label_index_edit > 0) {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit--;
		} else {
			int i = 0;

			while (labelChar[i] != 0) {
				i++;
			}

			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit = i;
		}
	} else if (key == KEY_UPARROW) {
		if (label_index_edit > 27) {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit -= 28;
		} else {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit = 0;
		}
	} else if (key == KEY_DOWNARROW) {
		int i = 0;

		while (labelChar[i] != 0) {
			i++;
		}

		if ((label_index_edit + 28) < i) {
			if (label_index_edit <= 27) {
				while (label_index_edit2 != label_index_edit) {
				}

				label_index_edit += 28;
			}
		} else {
			while (label_index_edit2 != label_index_edit) {
			}

			label_index_edit = i;
		}
	} else if (key == KEY_PAGEUP) {
		label_index_edit = 0;
	} else if (key == KEY_PAGEDOWN) {
		int i = 0;

		while (labelChar[i] != 0) {
			i++;
		}

		while (label_index_edit2 != label_index_edit) {
		}

		label_index_edit = i;
	}
}

void drawInfo(void) {
	uint8_t i = 0;
	char txtData1[50];

#ifdef screen_test
	return;
#endif

	if (lcd_show_logo == 1) {
		return;
	}

	if (lcd_screen == LcdScreenBattChr) {
		return;
	}

	//UsrLog("%lu: drawInfo", HAL_GetTick());

	//	} else if (seconds_old != tm1.seconds) {
	//		seconds_old = tm1.seconds;
	//
	//		drawInfo_tick = HAL_GetTick(); // sync

	//if ((HAL_GetTick() - drawInfo_tick) >= 500) {
	//drawInfo_tick = HAL_GetTick();

	if (seconds_old != tm1.seconds) {

		seconds_old = tm1.seconds;

#ifdef check_lcd_status
		// Checks for hardware failure
		uint32_t lcd_status = ILI9341_Status();

		// UsrLog("lcd_status: 0x%02lX", lcd_status);

		if ((lcd_status == 0) || (lcd_status != lcd_status_reference)) { // lcd_status_reference) { // 0x29820000
			// Tries to initialize the LCD again
			ILI9341_Init();
			ILI9341_FillScreen(ILI9341_BLACK);
			lcd_refresh = 3;

			screen_first_load = 1;

			if (lcd_status_cnt < 10) {
				beep(1, 0);
			}

			UsrLog("Error lcd status: 0x%02lX", lcd_status);

			if (lcd_status_cnt <= 10) {
				lcd_status_cnt++;
			}

			if (lcd_status == 0x29820000) {
				lcd_status_reference = lcd_status;
			}
		} else {
			if (lcd_status_cnt < 10) {
				lcd_status_cnt = 0;
			}
		}
#endif // #ifdef check_lcd_status

		if (esc_cnt > 0) {
			if (esc_cnt_timeout < 10) {
				esc_cnt_timeout++;
			} else {
				esc_cnt_timeout = 0;

				esc_cnt = 0;
			}
		}

		if (ctrl_atl_del_cnt > 0) {
			if (ctrl_atl_del_cnt_timeout < 5) {
				ctrl_atl_del_cnt_timeout++;
			} else {
				ctrl_atl_del_cnt_timeout = 0;

				ctrl_atl_del_cnt = 0;
			}
		}

		if (ctrl_atl_del_timeout > 0) {
			beep(1, 0);
		}

		if ((sysTick15000 > 10) && (sysTick15000 != sysTick15000_Old)) {
			sysTick15000_Old = sysTick15000;
			beep(sysTick15000 - 10, 0);
		}

//		if (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_SET) {
//
//			if (sysTick15000 > 10) {
//				beep(sysTick15000 - 10);
//			} else {
//				beep(1);
//			}
//		}

		if ((lcd_screen != LcdScreenMain) && (export_data_enabled == 0)) {
			if (back_to_main_timeout < back_to_main_timeout_recall) {
				back_to_main_timeout++;
			} else {
				back_to_main_timeout = 0;

				lcd_screen = LcdScreenMain;

				screen_first_load = 1;

				lcd_refresh = 3;
			}
		}

		if (clock1_update == TRUE) {
			clock1_update = FALSE;

			clock1.day = tm1.day;
			clock1.date = tm1.date;
			clock1.month = tm1.month;
			clock1.year = tm1.year;
			clock1.hours = tm1.hours;
			clock1.minutes = tm1.minutes;
			clock1.seconds = tm1.seconds;
		}

		if ((week_old_update_info != tm1.day) || (refresh_info == TRUE)) {
			week_old_update_info = tm1.day;

			if ((tm1.day - 1) >= 0) {
				ILI9341_WriteString(1, 1, txtDay[language_current][tm1.day - 1],
						Font_11x18,
						ILI9341_GREY, ILI9341_BLACK);
			} else {
				ILI9341_WriteString(1, 1, txtDay[language_current][7],
						Font_11x18,
						ILI9341_GREY,
						ILI9341_BLACK);
			}
		}

		if (date_format == mmddyy) {
			sprintf(txtData1, ". %02d-%02d-%02d %02d:%02d:%02d", tm1.month,
					tm1.date, tm1.year, tm1.hours, tm1.minutes, tm1.seconds);
		} else {
			sprintf(txtData1, ". %02d/%02d/%02d %02d:%02d:%02d", tm1.date,
					tm1.month, tm1.year, tm1.hours, tm1.minutes, tm1.seconds);
		}

		for (i = 0; i < 20; i++) {
			if ((txtData1_old[i] != txtData1[i]) || (refresh_info == TRUE)) {
				txtData1_old[i] = txtData1[i];

				char str1[2];
				str1[0] = txtData1[i];
				str1[1] = 0;

				ILI9341_WriteString(33 + (i * 11), 1, str1, Font_11x18,
				ILI9341_GREY, ILI9341_BLACK);
			}

			IWDG_delay_ms(0);
		}

		//battery_status = Battery_charging;

		uint16_t color1 = ILI9341_GREY;

		if (battery_status == Battery_in_use) {
			if (blink == TRUE) {
				if ((charger_old_update_info != 1) || (refresh_info == TRUE)) {
					charger_old_update_info = 1;

					ILI9341_FillRect(285, 1, 33, 18, ILI9341_BLACK);

					char batt_val10_chr[2] = { batt_val10 + 48, 0 };
					char batt_val1_chr[2] = { batt_val1 + 48, 0 };

					ILI9341_WriteString(292, 1, batt_val10_chr, Font_11x18,
					ILI9341_GREY, ILI9341_BLACK);
					ILI9341_WriteString(304, 1, batt_val1_chr, Font_11x18,
					ILI9341_GREY, ILI9341_BLACK);

					color1 = ILI9341_YELLOW;

					ILI9341_DrawLine(275, 10, 280, 2, color1);
					ILI9341_DrawLine(276, 10, 280, 2, color1);
					ILI9341_DrawLine(277, 10, 280, 2, color1);

					ILI9341_DrawLine(275, 16, 278, 10, color1);
					ILI9341_DrawLine(275, 16, 279, 10, color1);
					ILI9341_DrawLine(275, 16, 280, 10, color1);
				}
			} else {
				if ((charger_old_update_info != 0) || (refresh_info == TRUE)) {
					charger_old_update_info = 0;
				}
			}
		} else if (battery_status == Battery_charging) {
			if (blink == TRUE) {
				if ((charger_old_update_info != 1) || (refresh_info == TRUE)) {
					charger_old_update_info = 1;

					ILI9341_FillRect(285, 1, 33, 18, ILI9341_BLACK);

					char batt_val10_chr[2] = { batt_val10 + 48, 0 };
					char batt_val1_chr[2] = { batt_val1 + 48, 0 };

					ILI9341_WriteString(292, 1, batt_val10_chr, Font_11x18,
					ILI9341_GREY, ILI9341_BLACK);
					ILI9341_WriteString(304, 1, batt_val1_chr, Font_11x18,
					ILI9341_GREY, ILI9341_BLACK);

					color1 = ILI9341_ORANGE;

					ILI9341_DrawLine(275, 10, 280, 2, color1);
					ILI9341_DrawLine(276, 10, 280, 2, color1);
					ILI9341_DrawLine(277, 10, 280, 2, color1);

					ILI9341_DrawLine(275, 16, 278, 10, color1);
					ILI9341_DrawLine(275, 16, 279, 10, color1);
					ILI9341_DrawLine(275, 16, 280, 10, color1);
				}
			} else {
				if ((charger_old_update_info != 0) || (refresh_info == TRUE)) {
					charger_old_update_info = 0;

					ILI9341_DrawLine(275, 10, 280, 2, ILI9341_DARKGREY);
					ILI9341_DrawLine(276, 10, 280, 2, ILI9341_DARKGREY);
					ILI9341_DrawLine(277, 10, 280, 2, ILI9341_DARKGREY);

					ILI9341_DrawLine(275, 16, 278, 10, ILI9341_DARKGREY);
					ILI9341_DrawLine(275, 16, 279, 10, ILI9341_DARKGREY);
					ILI9341_DrawLine(275, 16, 280, 10, ILI9341_DARKGREY);
				}
			}
		} else if (battery_status == Battery_charged) {
			if (blink == TRUE) {
				if ((charger_old_update_info != 1) || (refresh_info == TRUE)) {
					charger_old_update_info = 1;

					ILI9341_FillRect(285, 1, 33, 18, ILI9341_BLACK);

					ILI9341_WriteString(285, 1, "1", Font_11x18,
					ILI9341_GREY,
					ILI9341_BLACK);
					ILI9341_WriteString(295, 1, "0", Font_11x18,
					ILI9341_GREY,
					ILI9341_BLACK);
					ILI9341_WriteString(307, 1, "0", Font_11x18,
					ILI9341_GREY,
					ILI9341_BLACK);

					if (button1_flag == 0) {
						ILI9341_FillRect(274, 1, 11, 18,
						ILI9341_BLACK);

						color1 = ILI9341_GREEN;

						ILI9341_DrawLine(275, 10, 280, 2, color1);
						ILI9341_DrawLine(276, 10, 280, 2, color1);
						ILI9341_DrawLine(277, 10, 280, 2, color1);

						ILI9341_DrawLine(275, 16, 278, 10, color1);
						ILI9341_DrawLine(275, 16, 279, 10, color1);
						ILI9341_DrawLine(275, 16, 280, 10, color1);
					}
				}
			} else {
				if ((charger_old_update_info != 0) || (refresh_info == TRUE)) {
					charger_old_update_info = 0;
				}
			}
		} else if (battery_status == Battery_error) {
			if (blink == TRUE) {

				if (lcd_screen == LcdScreenMain) {
					beep(1, 1);
				}

				if ((charger_old_update_info != 1) || (refresh_info == TRUE)) {
					charger_old_update_info = 1;

					if (button1_flag == 0) {

						color1 = ILI9341_RED;

						ILI9341_DrawLine(275, 10, 280, 2, color1);
						ILI9341_DrawLine(276, 10, 280, 2, color1);
						ILI9341_DrawLine(277, 10, 280, 2, color1);

						ILI9341_DrawLine(275, 16, 278, 10, color1);
						ILI9341_DrawLine(275, 16, 279, 10, color1);
						ILI9341_DrawLine(275, 16, 280, 10, color1);

					}

					ILI9341_WriteString(285, 1, "BAT", Font_11x18,
					ILI9341_BLACK, ILI9341_YELLOW);
				}
			} else {
				if ((charger_old_update_info != 0) || (refresh_info == TRUE)) {
					charger_old_update_info = 0;

					if (button1_flag == 0) {
						color1 = ILI9341_YELLOW;

						ILI9341_DrawLine(275, 10, 280, 2, color1);
						ILI9341_DrawLine(276, 10, 280, 2, color1);
						ILI9341_DrawLine(277, 10, 280, 2, color1);

						ILI9341_DrawLine(275, 16, 278, 10, color1);
						ILI9341_DrawLine(275, 16, 279, 10, color1);
						ILI9341_DrawLine(275, 16, 280, 10, color1);
					}

					ILI9341_WriteString(285, 1, "ERR", Font_11x18,
					ILI9341_BLACK, ILI9341_RED);
				}
			}
		}

		Appli_state_FS_old = Appli_state_FS;

		caps_lock_state_old = keybd_info_lcd.led_caps_lock;

		uint16_t icon_keyboard_color = ILI9341_RED;

		if (Appli_state_FS == APPLICATION_READY) {
			if (keybd_info_lcd.led_caps_lock == TRUE) {
				icon_keyboard_color = ILI9341_DARKCYAN;
			} else {
				icon_keyboard_color = ILI9341_DARKGREEN;
			}
		} else {
			if (usb_power_restart_timeout > 0) {
				icon_keyboard_color = ILI9341_DARKYELLOW;
			}
		}

		if (Appli_state_FS == APPLICATION_READY) {
			if (keybd_info_lcd.led_caps_lock == TRUE) {
				str[0] = 'A';
			} else {
				str[0] = 'a';
			}

			cnt_timeout_reset_USB = 0;
		} else {
//			if (usb_power_restart_timeout == 0) {
			str[0] = '?';
		}

		str[1] = 0;

		if (Appli_state_FS == APPLICATION_READY) {
			ILI9341_FillRect(251, 0, 12, 17, ILI9341_BLACK);

			uint8_t left = 253;

			if (keybd_info_lcd.led_caps_lock == TRUE) {
				left = 251;
			}

			ILI9341_WriteString(left, 0, str, Font_11x18,
			ILI9341_BLACK, icon_keyboard_color);

			if (keybd_info_lcd.led_caps_lock == TRUE) {
				ILI9341_DrawLine(262, 0, 263, 0, icon_keyboard_color);
				ILI9341_DrawLine(262, 17, 263, 17, icon_keyboard_color);
				ILI9341_DrawLine(263, 0, 263, 17, icon_keyboard_color);
				ILI9341_DrawLine(252, 16, 262, 16, ILI9341_BLACK);
			} else {
				ILI9341_DrawLine(251, 0, 252, 0, icon_keyboard_color);
				ILI9341_DrawLine(251, 17, 252, 17, icon_keyboard_color);
				ILI9341_DrawLine(251, 0, 251, 17, icon_keyboard_color);
				ILI9341_DrawLine(252, 1, 262, 1, ILI9341_BLACK);
			}
		} else {
			ILI9341_DrawLineV(252, 0, 17, ILI9341_BLACK);

			ILI9341_WriteString(253, 0, str, Font_11x18, icon_keyboard_color,
			ILI9341_BLACK);

			ILI9341_DrawRect(251, 0, 12, 17, icon_keyboard_color);
		}
//	}

		if (tm1.seconds == 0) {
			if ((lcd_screen == LcdScreenGraphic)
					|| (lcd_screen == LcdScreenMain)) {
				screen_first_load = 1;

				lcd_refresh = 3;
			}
		}
	}

	if (sensor_value_refresh == 1) {
		sensor_value_refresh = 0;

		lcd_config_ch_load_sensor_value();
	}

	// from IT7:

	if (init_flag == 1) {
		if ((HAL_GetTick() - tick100ms) >= 250) {
			tick100ms = HAL_GetTick();

			if (DC_DC_Error == 1) {
				beep(1, 1);

				if (DC_DC_Error_blink_cnt < 40) { // 40x 0.25: 10s
					DC_DC_Error_blink_cnt++;
				} else {
					DC_DC_Error_blink_cnt = 0;

					if (DC_DC_Error == 1) {
						DC_DC_Error = 0;

						HAL_GPIO_WritePin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin,
								GPIO_PIN_SET); // DC-DC On

						IWDG_delay_ms(200);

						HAL_NVIC_DisableIRQ(EXTI0_IRQn);
						HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
						HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

//						HAL_TIM_Base_Stop_IT(&htim7);

						ILI9341_Unselect();
						IWDG_delay_ms(5);
						ILI9341_Init();

						ILI9341_Unselect();
						IWDG_delay_ms(5);
						ILI9341_Init();

//						MX_SPI3_Init2();

						IWDG_delay_ms(5);

						ILI9341_FillScreen(ILI9341_BLACK);

						HAL_NVIC_EnableIRQ(EXTI0_IRQn);
						HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
						HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

						lcd_refresh = 3;

//						HAL_TIM_Base_Start_IT(&htim7);
					}
//					USB_FS_OC_Error = 0;
				}
			}

			if ((USB_FS_OC_Error == 1) || (USB_HS_OC_Error == 1)) {
				beep(1, 1);

				if (USB_OC_Error_blink_cnt < 40) { // 40x 0.25: 10s
					USB_OC_Error_blink_cnt++;
				} else {
					USB_OC_Error_blink_cnt = 0;

					if (USB_HS_OC_Error == 1) {
						USB_HS_OC_Error = 0;

						HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port,
						USB_HS_PWR_Pin, GPIO_PIN_RESET); // USB on
					}

//					USB_FS_OC_Error = 0;
				}
			}

			if (lcd_screen == LcdScreenUsbExport) {
				if (USB_HS_OC_Error == 1) {
					//USB_HS_OC_Error = 0;
					USB_HS_OC_Error_old = 1;

					uint16_t color = ILI9341_YELLOW;

					if (USB_OC_Error_blink < 1) {
						USB_OC_Error_blink++;

						color = ILI9341_RED;
					} else {
						USB_OC_Error_blink = 0;
					}

					ILI9341_WriteString(319 - (8 * 11), 100, "USB FAIL",
							Font_11x18,
							ILI9341_BLACK, color);
				} else {
					if (USB_HS_OC_Error_old == 1) {
						USB_HS_OC_Error_old = 0;

						ILI9341_FillRect(319 - (8 * 11), 100, (8 * 11), 18,
						ILI9341_BLACK);
					}
				}
			} else if (lcd_screen == LcdScreenMain) {
				//if(USB_FS_OC_Error_old != USB_FS_OC_Error){
				if (USB_FS_OC_Error == 1) {
					//USB_FS_OC_Error = 0;
					USB_FS_OC_Error_old = 1;

					if ((main_keyboard_chars_num + 2) > 10) {
						ILI9341_FillRect(
								319 - ((main_keyboard_chars_num + 2) * 11), 218,
								((main_keyboard_chars_num + 2) * 11), 18,
								ILI9341_BLACK);
					}

					sprintf(str, "[USB FAIL]");

					uint16_t color = ILI9341_YELLOW;

					if (USB_OC_Error_blink < 1) {
						USB_OC_Error_blink++;

						color = ILI9341_RED;
					} else {
						USB_OC_Error_blink = 0;
					}

					ILI9341_WriteString(319 - ((8 + 2) * 11), 218, str,
							Font_11x18, color,
							ILI9341_BLACK);
				} else {
					if (USB_FS_OC_Error_old == 1) {
						USB_FS_OC_Error_old = 0;

						sprintf(str, "[%s]", main_keyboard[language_current]);

						if ((main_keyboard_chars_num + 2) < 10) {
							ILI9341_FillRect(319 - ((8 + 2) * 11), 218,
									((8 + 2) * 11), 18, ILI9341_BLACK);
						}

						ILI9341_WriteString(
								319 - ((main_keyboard_chars_num + 2) * 11), 218,
								str, Font_11x18, ILI9341_BLUE,
								ILI9341_BLACK);
					}
				}

				if (USB_HS_OC_Error == 1) {
					//USB_HS_OC_Error = 0;
					USB_HS_OC_Error_old = 1;

					uint16_t color = ILI9341_YELLOW;

					if (USB_OC_Error_blink < 1) {
						USB_OC_Error_blink++;

						color = ILI9341_RED;
					} else {
						USB_OC_Error_blink = 0;
					}

					if ((main_udisk_chars_num + 2) > 10) {
						ILI9341_FillRect(1, 218,
								((main_udisk_chars_num + 2) * 11), 18,
								ILI9341_BLACK);
					}

					ILI9341_WriteString(1, 218, "[USB FAIL]", Font_11x18, color,
					ILI9341_BLACK);
				} else {
					if (USB_HS_OC_Error_old == 1) {
						USB_HS_OC_Error_old = 0;

						sprintf(str, "[%s]", main_u_disk[language_current]);

						if ((main_udisk_chars_num + 2) < 10) {
							ILI9341_FillRect(1, 218, ((8 + 2) * 11), 18,
							ILI9341_BLACK);
						}

						ILI9341_WriteString(1, 218, str, Font_11x18,
						ILI9341_BLUE,
						ILI9341_BLACK);
					}
				}
			}

			if (milisec < 9) {
				milisec++;
			} else {
				milisec = 0;
			}

			if ((lcd_loop_busy == FALSE) && (acc_busy == FALSE)) {
				lcd_inter_busy = TRUE;

				if (update_info == FALSE) {
					update_info = TRUE;
				} else {
					update_info = FALSE;

					if (lcd_enabled == TRUE) {
						if (lcd_refresh != 1) {

							if (tm1.seconds != update_info_seconds_old) {
								milisec = 0;

								blink = !blink;

								IWDG_delay_ms(0);

								if (lcd_screen == LcdScreenMonCh) {
									get_temperatures = 1;

									lcd_phase = 1;

									lcd_refresh = 3;
								}
							}

							if ((tm1.seconds != update_info_seconds_old)
									| (milisec == 5)) {
								update_info_seconds_old = tm1.seconds;
							}
						}
					}
				}

				lcd_inter_busy = FALSE;
			}

			timer7_tick = TRUE;
		}

		if ((HAL_GetTick() - tick250ms) >= 500) {
			tick250ms = HAL_GetTick();

			if (blink_cnt2 == 0) {
				blink_cnt2 = 1;

			} else {
				blink_cnt2 = 0;

			}

			if ((lcd_loop_busy == FALSE) && (acc_busy == FALSE)) {
				lcd_inter_busy = TRUE;

				if (lcd_refresh == 0) {
					if (lcd_screen == LcdScreenCfgCh) {
						if (blink_cnt2 == 0) {
							if (ch_config_index_edit_mode == 0) {
								str2[0] = channelLabel_tmp[label_index_edit];
								str2[1] = 0;

								if (str2[0] == 0) {
									str2[0] = 32;
								}

								if (label_index_edit < 28) {
									ILI9341_WriteString(
											5 + (label_index_edit * 11), 185,
											str2, Font_11x18,
											ILI9341_GREEN,
											ILI9341_BLACK);
								} else {
									ILI9341_WriteString(
											5 + ((label_index_edit - 28) * 11),
											205, str2, Font_11x18,
											ILI9341_GREEN,
											ILI9341_BLACK);
								}
							} else if (ch_config_index_edit_mode == 1) {
								lcd_config_ch_blink_status();
							}
						} else if ((blink_cnt2 == 1)
								& (label_index_edit == label_index_edit2)) {
							if (ch_config_index_edit_mode == 0) {
								str2[0] = channelLabel_tmp[label_index_edit];
								str2[1] = 0;

								if (str2[0] == 0) {
									str2[0] = 32;
								}

								if (label_index_edit < 28) {
									ILI9341_WriteString(
											5 + (label_index_edit * 11), 185,
											str2, Font_11x18,
											ILI9341_BLACK,
											ILI9341_RED);
								} else {
									ILI9341_WriteString(
											5 + ((label_index_edit - 28) * 11),
											205, str2, Font_11x18,
											ILI9341_BLACK,
											ILI9341_RED);
								}
							} else if (ch_config_index_edit_mode == 1) {
								lcd_config_ch_blink_status2();
							}
						}
					} else if (lcd_screen == LcdScreenCfgClk) {
						if (blink_cnt2 == 0) {
							if (clock_index_edit_mode == 0) {
								if (clock_index_edit == 0) {
									str2[0] = clock_date[clock_index_edit];
									str2[1] = clock_date[clock_index_edit + 1];
									str2[2] = clock_date[clock_index_edit + 2];
									str2[3] = clock_date[clock_index_edit + 3];
									str2[4] = 0;

									ILI9341_WriteString(
											5 + (clock_index_edit * 11), 65,
											str2, Font_11x18,
											ILI9341_YELLOW,
											ILI9341_DARKGREY);
								} else if (clock_index_edit >= 5) {
									str2[0] = clock_date[clock_index_edit];
									str2[1] = 0;

									ILI9341_WriteString(
											5 + (clock_index_edit * 11), 65,
											str2, Font_11x18,
											ILI9341_YELLOW,
											ILI9341_DARKGREY);
								}
							} else if (clock_index_edit_mode == 1) {
								str2[0] = clock_time[clock_index_edit];
								str2[1] = 0;

								ILI9341_WriteString(5 + (clock_index_edit * 11),
										171, str2, Font_11x18,
										ILI9341_YELLOW,
										ILI9341_DARKGREY);
							}
						} else if ((blink_cnt2 == 1)
								& (label_index_edit == label_index_edit2)) {
							if (clock_index_edit_mode == 0) {
								if (clock_index_edit == 0) {
									str2[0] = clock_date[clock_index_edit];
									str2[1] = clock_date[clock_index_edit + 1];
									str2[2] = clock_date[clock_index_edit + 2];
									str2[3] = clock_date[clock_index_edit + 3];
									str2[4] = 0;

									ILI9341_WriteString(
											5 + (clock_index_edit * 11), 65,
											str2, Font_11x18,
											ILI9341_DARKGREY,
											ILI9341_LIGHTGREY);
								} else if (clock_index_edit >= 5) {
									str2[0] = clock_date[clock_index_edit];
									str2[1] = 0;

									ILI9341_WriteString(
											5 + (clock_index_edit * 11), 65,
											str2, Font_11x18,
											ILI9341_DARKGREY,
											ILI9341_LIGHTGREY);
								}
							} else if (clock_index_edit_mode == 1) {
								str2[0] = clock_time[clock_index_edit];
								str2[1] = 0;

								ILI9341_WriteString(5 + (clock_index_edit * 11),
										171, str2, Font_11x18, ILI9341_DARKGREY,
										ILI9341_LIGHTGREY);
							}
						}
					}
				}

				lcd_inter_busy = FALSE;
			}
		}

		if ((HAL_GetTick() - tick500ms) >= 500) {
			tick500ms = HAL_GetTick();

			if (blink_cnt1 == 0) {
				blink_cnt1 = 1;

			} else {
				blink_cnt1 = 0;

			}

			if ((lcd_loop_busy == FALSE) && (acc_busy == FALSE)) {
				lcd_inter_busy = TRUE;

				if (lcd_refresh == 0) {

//					if (lcd_screen == LcdScreenMemTest) {
//
//					} else
					if (lcd_screen == LcdScreenPasswordConfig) {
						for (int i = 0; i < 6; i++) {
							if (passwd_config_cnt == 0) {
								ILI9341_WriteString(5 + (i * 55), 75,
										password_config_code_tmp_str0[i],
										Font_11x18,
										ILI9341_LIGHTGREY,
										ILI9341_BLACK);
							} else if (passwd_config_cnt == 1) {
								ILI9341_WriteString(5 + (i * 55), 135,
										password_config_code_tmp_str1[i],
										Font_11x18,
										ILI9341_LIGHTGREY,
										ILI9341_BLACK);
							} else {
								ILI9341_WriteString(5 + (i * 55), 195,
										password_config_code_tmp_str2[i],
										Font_11x18,
										ILI9341_LIGHTGREY,
										ILI9341_BLACK);
							}
						}
					} else if (lcd_screen == LcdScreenPassword) {
						for (int i = 0; i < 6; i++) {
							ILI9341_WriteString(5 + (i * 55), 125,
									password_code_tmp_str[i], Font_11x18,
									ILI9341_LIGHTGREY,
									ILI9341_BLACK);
						}
						if (blink_cnt1 == 0) {
							if (blink_passwd == 0) {
								blink_passwd = 1;

								if (password_error_timeout == 0) {
									lcd_password_blink(ILI9341_YELLOW);
								}

								if (password_error_timeout >= 1) {
									sprintf(str, "%s%d",
											drawInfo_Password_label1[language_current],
											password_error_timeout);
									ILI9341_WriteString(5, 160, str, Font_11x18,
									ILI9341_BLACK,
									ILI9341_BLACK);

									password_error_timeout--;

									if (password_error_timeout >= 1) {
										sprintf(str, "%s%d",
												drawInfo_Password_label1[language_current],
												password_error_timeout);
										ILI9341_WriteString(5, 160, str,
												Font_11x18,
												ILI9341_BLACK,
												ILI9341_RED);
									} else {
										password_user_clear();
									}
								}
							}
						} else if (blink_cnt1 == 1) {
							if (blink_passwd == 1) {
								blink_passwd = 0;

								lcd_password_blink(ILI9341_GREY);
							}
						}
					}
				}

				lcd_inter_busy = FALSE;
			}
		}

		if ((HAL_GetTick() - 1000) > tick1000ms) {
			tick1000ms = HAL_GetTick();

			if (usb_power_restart_timeout > 1) {
				usb_power_restart_timeout--;
			} else {
				if (usb_power_restart_timeout == 1) {
					usb_power_restart_timeout--;
				}
			}

			if (usb_power_restart_count < 10) {
				usb_power_restart_count++;
			} else {
				usb_power_restart_count = 0;

				if (usb_power_recycle_count >= 3) {
					usb_power_restart_timeout = 4;
				}

				usb_power_recycle_count = 0;
			}

			if (lcd_timeout0 > 0) {
				if (standby_mode == FALSE) {
					if ((lcd_timeout0 > lcd_timeout0recall)
							&& (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
							AC_DC_STATE_Pin) == GPIO_PIN_RESET)) {
						lcd_timeout0 = lcd_timeout0recall;
					}
					if (export_data_enabled == 0) {
						lcd_timeout0--;
					}
				} else {
					if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
					AC_DC_STATE_Pin) == GPIO_PIN_RESET) {
						if (lcd_timeout0 > lcd_timeout0recall) {
							lcd_timeout0 = lcd_timeout0recall;
						}
						if (export_data_enabled == 0) {
							lcd_timeout0--;
						}
					}
				}
			} else if ((tm1.seconds > 10) && (tm1.seconds < 50)) {
				if (standby_mode == FALSE) {
					if (lcd_screen == LcdScreenBattChr) {
						lcd_bright_set = lcd_bright_off;

						if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
						AC_DC_STATE_Pin) == GPIO_PIN_RESET) {
							UsrLog("%02d/%02d/%02d %02d:%02d:%02d", tm1.date,
									tm1.month, tm1.year, tm1.hours, tm1.minutes,
									tm1.seconds);
							UsrLog("Power Off");

							HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin,
									GPIO_PIN_RESET); // Turn uC Off

							IWDG_delay_ms(2000);

							HAL_NVIC_SystemReset();
						}
					} else {
						if ((mem_test_enabled == 0)
								|| (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
								AC_DC_STATE_Pin) == GPIO_PIN_RESET)) {
							if (pwrOffLcdBack == 0) {
								lcd_bright_set = lcd_bright_min;

								button1_stage = ScreenAutoDimm;
							}

							standby_mode = TRUE;

							if (password_request != password_request_yes) {
								if ((lcd_screen != LcdScreenMonCh)
										&& (lcd_screen != LcdScreenGraphic)) {
									lcd_screen = LcdScreenGraphic;

									lcd_refresh = 3;
								}
							}
						}
					}
				} else {
					if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
					AC_DC_STATE_Pin) == GPIO_PIN_RESET) {
						UsrLog("%02d/%02d/%02d %02d:%02d:%02d", tm1.date,
								tm1.month, tm1.year, tm1.hours, tm1.minutes,
								tm1.seconds);
						UsrLog("Power Off");

						HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin,
								GPIO_PIN_RESET); // Turn uC Off

						IWDG_delay_ms(2000);

						HAL_NVIC_SystemReset();
					}
				}

				lcd_timeout0 = 5;
			}
		}
	}

	if ((HAL_GetTick() - battery_failure_timeout) >= 1000) {
		battery_failure_timeout = HAL_GetTick();

		TP4056_charger_freq = battery_failure_cnt;

		if ((tm1.day == 1) && (tm1.month == 1) && (tm1.year == 0)
				&& (tm1.date == 1) && (tm1.hours == 0) && (tm1.minutes < 15)) {
			battery_failure_flag = 1;

			if (batt_error_esc < 3) {
				if (lcd_screen == LcdScreenMain) {
					lcd_screen = LcdScreenCfgClk;

					lcd_refresh = 3;
				}
			}
		} else if (TP4056_charger_freq > 1) {
			battery_failure_flag = 1;
		} else {
			battery_failure_flag = 0;
		}

		battery_failure_cnt = 0;

		// battery_failure_flag:
		// interruption on pin PA5 (frequency counter)
		// ---> see stm32f4xx_it.c file

		if (battery_failure_flag == 1) {
			battery_status = Battery_error;

			//UsrLog("Battery: error; tick: %lu", HAL_GetTick());
		} else {
			if (lcd_screen != LcdScreenMonCh) {
				ADC_get();
			}

			if (HAL_GPIO_ReadPin(CHARGER_STATE_GPIO_Port, CHARGER_STATE_Pin)
					== GPIO_PIN_RESET) {
				if (V_Charger_val < 8.0) {
					battery_status = Battery_in_use;
				} else {
					battery_status = Battery_charged;
				}
			} else {
				if (V_Charger_val >= 8.0) {
					battery_status = Battery_charging;
				}
			}
		}
	}
}

void lcd_config_clock_cursor_move(void) {
	if (clock_index_edit_mode == 0) {
		if (clock_index_edit == 0) {
			clock_index_edit = 5;
		} else if (clock_index_edit == 5) {
			clock_index_edit = 6;
		} else if (clock_index_edit == 6) {
			clock_index_edit = 8;
		} else if (clock_index_edit == 8) {
			clock_index_edit = 9;
		} else if (clock_index_edit == 9) {
			clock_index_edit = 11;
		} else if (clock_index_edit == 11) {
			clock_index_edit = 12;
		} else if (clock_index_edit == 12) {
			clock_index_edit_mode = 1;
			clock_index_edit = 0;
		}
	} else if (clock_index_edit_mode == 1) {
		if (clock_index_edit == 0) {
			clock_index_edit = 1;
		} else if (clock_index_edit == 1) {
			clock_index_edit = 3;
		} else if (clock_index_edit == 3) {
			clock_index_edit = 4;
		} else if (clock_index_edit == 4) {
			clock_index_edit = 6;
		} else if (clock_index_edit == 6) {
			clock_index_edit = 7;
		} else if (clock_index_edit == 7) {
			clock1_setup = true;

			clock_index_edit = 0;

			clock_index_edit_mode = 0;
		}
	}
}

void lcd_config_clock_load(void) {
	clock1_update = TRUE;

	clock_index_edit = 0;
}

void lcd_config_clock(void) {
	sprintf(str, "%s", config_clock_title[language_current]);

//	if (language_current == lang_ptBr) {
//		str[14] = O_acute_accent; // 211:Ó
//	}

	if (screen_first_load == 1) {
		lcd_window(0, 20, 320, 200, str, ILI9341_DARKBLUE, ILI9341_LIGHTGREY);

		screen_first_load = 0;
	}

	sprintf(str, "%s: (%s  dd/mm/yy)", config_clock_date[language_current],
			config_clock_date2[language_current]);

	ILI9341_WriteString(5, 45, str, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	ILI9341_DrawRect(4, 64, 311, 21, ILI9341_LIGHTGREY);

	ILI9341_FillRect(5, 65, 310, 20, ILI9341_DARKGREY);

	if (clock1.day > 0) {
		sprintf(clock_date, "%s. %02d/%02d/%02d",
				txtDay[language_current][clock1.day - 1], clock1.date,
				clock1.month, clock1.year);
	} else {
		sprintf(clock_date, "%s. %02d/%02d/%02d", txtDay[language_current][0],
				clock1.date, clock1.month, clock1.year);
	}

	ILI9341_WriteString(5, 65, clock_date, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_DARKGREY);

	sprintf(str, "%s: (hh:MM:ss)", config_clock_time[language_current]);

	ILI9341_WriteString(5, 150, str, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	ILI9341_DrawRect(4, 169, 311, 21, ILI9341_LIGHTGREY);

	ILI9341_FillRect(5, 170, 310, 20, ILI9341_DARKGREY);

	sprintf(clock_time, "%02d:%02d:%02d", clock1.hours, clock1.minutes,
			clock1.seconds);

	ILI9341_WriteString(5, 171, clock_time, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_DARKGREY);

	sprintf(str, "(%s: 1-7)", config_clock_date2[language_current]);

	ILI9341_WriteString(5, 95, str, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	sprintf(str, "(TAB: %s)", config_clock_tab[language_current]);

	ILI9341_WriteString(5, 120, str, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	sprintf(str, "(%s)", config_clock_enter[language_current]);

	ILI9341_WriteString(5, 210, str, Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_BLACK);
}

int lcd_batt_charging(void) {
	ILI9341_FillRect(25, 25, 250, 25, ILI9341_LIGHTGREY);

	ILI9341_FillRect(25, 190, 250, 25, ILI9341_LIGHTGREY);

	ILI9341_FillRect(25, 50, 25, 140, ILI9341_LIGHTGREY);

	ILI9341_FillRect(250, 50, 25, 140, ILI9341_LIGHTGREY);

	ILI9341_FillRect(275, 80, 25, 80, ILI9341_LIGHTGREY);

	// draw steps
	ILI9341_FillRect(60, 60, 28, 120, ILI9341_LIGHTGREY);

	ILI9341_FillRect(98, 60, 28, 120, ILI9341_LIGHTGREY);

	ILI9341_FillRect(136, 60, 28, 120, ILI9341_LIGHTGREY);

	ILI9341_FillRect(174, 60, 28, 120, ILI9341_LIGHTGREY);

	ILI9341_FillRect(212, 60, 28, 120, ILI9341_LIGHTGREY);

	return 0;
}

int lcd_sensors_monitor(int update_window, int phase) {
	if (phase == 0) {
		if (update_window > 0) {
			lcd_window(0, 20, 320, 200, sensors_monitor_title[language_current],
			ILI9341_DARKBLUE,
			ILI9341_LIGHTGREY);

			sprintf(str, "%s     SENSOR ID       TEMPER",
					sensors_monitor_desc[language_current]);
			ILI9341_WriteString(6, 45, str, Font_7x10,
			ILI9341_WHITE, ILI9341_BLACK);
		}
	} else if (phase == 1) {
		for (int i = 0; i < maxChannel; i++) {
			float temp_calc = SENSORS[i].TEMPERATURE;

			int temp = (int) temp_calc;
			int temp_dec = (int) ((float) (temp_calc - temp) * 10000.0);

			char str2[17];

			uint16_t sensor_color = ILI9341_YELLOW;

			uint8_t sensor_ID[8];

			for (int j = 0; j < maxChannel; j++) {
				sensor_ID[j] = SENSORS[i].SERIAL_ID[j];
			}

			DS18B20_hex_id(sensor_ID, str2);

			str2[14] = 0;

			sprintf(str, "%d  %s  %d.%04d", i + 1, str2, temp, temp_dec);

			char str3[5]; // desativado

			if (SENSORS[i].FLAG_ENABLED == 1) {
				sprintf(str3, sensors_monitor_yes[language_current]);

				sensor_color = ILI9341_GREEN;
			} else {
				sprintf(str3, sensors_monitor_no[language_current]);
			}

			char str4[5]; // Yes / No

			if (DS18B20_get_presence(SENSORS[i]) == HAL_OK) {
				sprintf(str4, sensors_monitor_yes[language_current]);
			} else {
				sprintf(str4, sensors_monitor_no[language_current]);

				sensor_color = ILI9341_RED;
			}

			char dec_char = '.';

			if (decimal_separator == 1) {
				dec_char = ',';
			}

			uint8_t temp_unit = 67; // ASCII "C": 67

			if (temperature_unit == TemperatureUnit_F) {
				temp_unit = 70; // ASCII "F": 70
			}

			sprintf(str, "%d   %s   %s  %s  %02d%c%04d*%c ", i + 1, str4, str3,
					str2, temp, dec_char, temp_dec, temp_unit);

			ILI9341_WriteString(14, 60 + (i * 15), str, Font_7x10, sensor_color,
			ILI9341_BLACK);
		}

		sprintf(str, "%s: %.3fV", sensors_monitor_bat[language_current],
				V_Bat_val);
		ILI9341_WriteString(14, 180, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);

		sprintf(str, "%s: %.1fV", sensors_monitor_charg[language_current],
				V_Charger_val);
		ILI9341_WriteString(140, 180, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);

		sprintf(str, "%s: X: %d*  ", sensors_monitor_ang[language_current],
				MPU6050_AccAngleX);
		ILI9341_WriteString(14, 195, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);

		sprintf(str, "Y: %d*  ", MPU6050_AccAngleY);
		ILI9341_WriteString(130, 195, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);

		if (checksumStatusError == 0) {
			sprintf(str, "uC ROM: OK");
		} else {
			sprintf(str, "uC ROM: ERR");
		}
		ILI9341_WriteString(200, 195, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);

		sprintf(str, "uC UID:  %08lX %08lX %08lX", uC_ID[0], uC_ID[1],
				uC_ID[2]);
		ILI9341_WriteString(14, 210, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);

		sprintf(str, "MEM UID: %02X%02X%02X%02X %02X%02X%02X%02X (1-8: OffSet)",
				w25qxx.UniqID[0], w25qxx.UniqID[1], w25qxx.UniqID[2],
				w25qxx.UniqID[3], w25qxx.UniqID[4], w25qxx.UniqID[5],
				w25qxx.UniqID[6], w25qxx.UniqID[7]);

		ILI9341_WriteString(14, 225, str, Font_7x10, ILI9341_WHITE,
		ILI9341_BLACK);
	} else if (phase == 2) {
		return 2;
	}

	return 0;
}

void lcd_sensors_monitor_offset(void) {
	sprintf(str, "%s CH: %d", sensors_monitor_offset_title[language_current],
			LcdScreenMonChOffsetCh);

	lcd_window(0, 20, 320, 200, str,
	ILI9341_DARKBLUE,
	ILI9341_LIGHTGREY);

//	if(sensor_offset[LcdScreenMonChOffsetCh - 1] > 9.99) {
//		sprintf(str, "Offset: +%.1f", sensor_offset[LcdScreenMonChOffsetCh - 1]);
//	} else if(sensor_offset[LcdScreenMonChOffsetCh - 1] > 0) {
//		sprintf(str, "Offset: +0%.1f", sensor_offset[LcdScreenMonChOffsetCh - 1]);
//	} else if(sensor_offset[LcdScreenMonChOffsetCh - 1] == 0.0) {
//		sprintf(str, "Offset: 0%.1f", sensor_offset[LcdScreenMonChOffsetCh - 1]);
//	} else if(sensor_offset[LcdScreenMonChOffsetCh - 1] > -9.99) {
//		float neg_offset = sensor_offset[LcdScreenMonChOffsetCh - 1];
//		neg_offset *= -1;
//
//		sprintf(str, "Offset: -0%.1f", neg_offset);
//	} else {
	sprintf(str, "Offset:");
//	}

	ILI9341_WriteString(6, 45, str, Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	float tmp_val = sensor_offset_tmp / 10.0;

	sprintf(str, " %.1f ", tmp_val);
//	}

	if (sensor_offset_tmp == sensor_offset[LcdScreenMonChOffsetCh - 1]) {
		ILI9341_WriteString(102, 45, str, Font_11x18,
		ILI9341_GREEN, ILI9341_BLACK);
	} else {
		if (sensor_offset_tmp > sensor_offset[LcdScreenMonChOffsetCh - 1]) {
			ILI9341_WriteString(102, 45, str, Font_11x18,
			ILI9341_YELLOW, ILI9341_BLACK);
		} else if (sensor_offset_tmp
				< sensor_offset[LcdScreenMonChOffsetCh - 1]) {
			ILI9341_WriteString(102, 45, str, Font_11x18,
			ILI9341_ORANGE, ILI9341_BLACK);
		}
	}

	if (SENSORS[LcdScreenMonChOffsetCh - 1].FLAG_ENABLED == 1) {
		sprintf(str, " IN USE ");

		ILI9341_WriteString(220, 45, str, Font_11x18,
		ILI9341_BLACK, ILI9341_RED);
	}

	ILI9341_WriteString(6, 85, "[+]: +0.1", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	ILI9341_WriteString(6, 105, "[-]: -0.1", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	ILI9341_WriteString(6, 125, "[Z]: 0.0 (Zero)", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	ILI9341_WriteString(6, 145, "[U]: +15.0 (Max)", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	ILI9341_WriteString(6, 165, "[D]: -15.0 (Min)", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	ILI9341_WriteString(6, 185, "[ENTER]: Save & exit", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	ILI9341_WriteString(6, 205, "[ESC]: Do not save & exit", Font_11x18,
	ILI9341_WHITE, ILI9341_BLACK);

	//ILI9341_DrawRect(4, 120, 311, 21, ILI9341_LIGHTGREY);

//	ConfigStruct_TypeDef conf_dat;
//
//	W25Q64_read_config(&conf_dat);
//
//	conf_dat.DateFormat = date_format;
//
//	conf_dat.DecimalSeparator = decimal_separator;
//
//	conf_dat.TemperatureUnit = temperature_unit;
//
//	conf_dat.Language = language_current;
//
//	W25Q64_update_config(&conf_dat);
}

void lcd_config_ch_load_sensor_value(void) {
	uint8_t sensor_ID[8];
	uint16_t color;
	uint16_t x_pos = 0;

//	if (ch_status_tmp[ch_config_index] == 0) {
//		color = ILI9341_BLUE;
//	} else {
//		color = ILI9341_GREEN;
//	}

	if (ch_status_tmp[ch_config_index] == 0) {
		color = ILI9341_YELLOW; // ILI9341_BLUE
	} else {
		color = ILI9341_ORANGE; // ILI9341_GREEN;
	}

	if ((SENSORS[ch_config_index].FLAG_PRESENT == TRUE)
			&& (SENSORS[ch_config_index].FLAG_TIMEOUT == FALSE)) {

		for (int j = 0; j < maxChannel; j++) {
			sensor_ID[j] = SENSORS[ch_config_index].SERIAL_ID[j];
		}

		DS18B20_hex_id(sensor_ID, str);

		ILI9341_WriteString(139, 105, str, Font_11x18, color,
		ILI9341_BLACK);

		sprintf(str, "%.4f*C", SENSORS[ch_config_index].TEMPERATURE);

		if (SENSORS[ch_config_index].TEMPERATURE >= 100.0) { // > 100.0
			if (decimal_separator == sepComma)
				str[3] = ',';

			x_pos = 205;
		} else if (SENSORS[ch_config_index].TEMPERATURE >= 10.0) { // 99.9 ~ 10.0
			if (decimal_separator == sepComma)
				str[2] = ',';

			x_pos = 216;
		} else if (SENSORS[ch_config_index].TEMPERATURE > 0.0) { // 9.9 ~ 0.0
			if (decimal_separator == sepComma)
				str[1] = ',';

			x_pos = 227;
		} else if (SENSORS[ch_config_index].TEMPERATURE > -10.0) { // 0.0 ~ -10.0
			if (decimal_separator == sepComma)
				str[1] = ',';

			x_pos = 227;
		} else { // -10.0 ~ -50.0
			if (decimal_separator == sepComma)
				str[2] = ',';

			x_pos = 216;
		}

		ILI9341_WriteString(x_pos, 125, str, Font_11x18, color,
		ILI9341_BLACK);

	} else {
		ILI9341_WriteString(139, 105, "0000000000000000", Font_11x18, color,
		ILI9341_BLACK);

		char decSep = '.';

		if (decimal_separator == sepComma)
			decSep = ',';

		char tempUnit = 'C';

		if (temperature_unit == TemperatureUnit_F)
			tempUnit = 'F';

		sprintf(str, "0%c0000*%c", decSep, tempUnit);

		ILI9341_WriteString(227, 125, str, Font_11x18, color,
		ILI9341_BLACK);
	}
}

void ch_config_start(int index) {
	channel_header_tmp[index].start_day = tm1.date;
	channel_header_tmp[index].start_month = tm1.month;
	channel_header_tmp[index].start_year = tm1.year;
	channel_header_tmp[index].start_hour = tm1.hours;
	channel_header_tmp[index].start_minute = tm1.minutes;

	channel_header_current[index].stop_day = 0;
	channel_header_current[index].stop_month = 0;
	channel_header_current[index].stop_year = 0;
	channel_header_current[index].stop_hour = 0;
	channel_header_current[index].stop_minute = 0;

	channel_header_tmp[index].stop_day = 0;
	channel_header_tmp[index].stop_month = 0;
	channel_header_tmp[index].stop_year = 0;
	channel_header_tmp[index].stop_hour = 0;
	channel_header_tmp[index].stop_minute = 0;

	data_header.FLAG_ENABLED = 1;

	data_header.START_DAY = channel_header_tmp[index].start_day;
	data_header.START_MONTH = channel_header_tmp[index].start_month;
	data_header.START_YEAR = channel_header_tmp[index].start_year;
	data_header.START_HOUR = channel_header_tmp[index].start_hour;
	data_header.START_MINUTE = channel_header_tmp[index].start_minute;

	data_header.STOP_DAY = 0; // canal_stop_day_tmp[index];
	data_header.STOP_MONTH = 0; // canal_stop_month_tmp[index];
	data_header.STOP_YEAR = 0; // canal_stop_year_tmp[index];
	data_header.STOP_HOUR = 0; // canal_stop_hour_tmp[index];
	data_header.STOP_MINUTE = 0; // canal_stop_minute_tmp[index];

	W25Q64_update_header(index, &data_header);
}

void ch_config_stop(int index) {
	channel_header_tmp[index].stop_day = tm1.date;
	channel_header_tmp[index].stop_month = tm1.month;
	channel_header_tmp[index].stop_year = tm1.year;
	channel_header_tmp[index].stop_hour = tm1.hours;
	channel_header_tmp[index].stop_minute = tm1.minutes;

//						W25Q64_read_header(index, &data_header);

	data_header.FLAG_ENABLED = 0;

	data_header.START_DAY = channel_header_tmp[index].start_day;
	data_header.START_MONTH = channel_header_tmp[index].start_month;
	data_header.START_YEAR = channel_header_tmp[index].start_year;
	data_header.START_HOUR = channel_header_tmp[index].start_hour;
	data_header.START_MINUTE = channel_header_tmp[index].start_minute;

	data_header.STOP_DAY = channel_header_tmp[index].stop_day;
	data_header.STOP_MONTH = channel_header_tmp[index].stop_month;
	data_header.STOP_YEAR = channel_header_tmp[index].stop_year;
	data_header.STOP_HOUR = channel_header_tmp[index].stop_hour;
	data_header.STOP_MINUTE = channel_header_tmp[index].stop_minute;

//						UsrLog("w25qxx.Lock %d", w25qxx.Lock);

	W25Q64_update_header(index, &data_header);
}

void lcd_config_ch(void) {
	uint16_t color = ILI9341_LIGHTGREY;

	if (save_channel_label == 1) {
		W25Q64_update_name(ch_config_index, channelLabel[ch_config_index]);

		save_channel_label = 0;
	}

	if (screen_first_load == 1) {
		screen_first_load = 0;

		sprintf(str, "%s %d", sensors_config_ch_title[language_current],
				ch_config_index + 1);
		lcd_window(0, 20, 320, 200, str, ILI9341_DARKBLUE, ILI9341_LIGHTGREY);
	}

	// STATUS

	ILI9341_WriteString(5, 45, sensors_config_rec[language_current], Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	if (ch_config_index_edit_mode == ch_config_index_edit_mode_text) {
		if (ch_status_tmp[ch_config_index] == 0) {
			color = ILI9341_GREEN; // ILI9341_BLUE;

			ILI9341_WriteString(180, 45, sensors_config_inac[language_current],
					Font_11x18,
					ILI9341_BLACK, color);
		} else {
			color = ILI9341_YELLOW; // ILI9341_GREEN

			ILI9341_WriteString(180, 45, sensors_config_act[language_current],
					Font_11x18,
					ILI9341_BLACK, color);

//			ILI9341_WriteString(214, 45, "  ", Font_11x18, color,
//			ILI9341_BLACK);
		}
	}

	ILI9341_WriteString(5, 65, sensors_config_start[language_current],
			Font_11x18, ILI9341_LIGHTGREY,
			ILI9341_BLACK); // "Inicio:"

	ILI9341_WriteString(5, 85, sensors_config_stop[language_current],
			Font_11x18, ILI9341_LIGHTGREY,
			ILI9341_BLACK); // "Fim:"

	if (ch_status_tmp[ch_config_index] == 0) {
		color = ILI9341_YELLOW; // ILI9341_DARKBLUE;
	} else {
		color = ILI9341_ORANGE; // ILI9341_DARKGREEN;
	}

	lcd_config_ch_str_start(ch_config_index);

	ILI9341_WriteString(159, 65, str, Font_11x18, color,
	ILI9341_BLACK);

	lcd_config_ch_str_stop(ch_config_index);

	ILI9341_WriteString(159, 85, str, Font_11x18, color,
	ILI9341_BLACK);

	// Start / Stop

	ILI9341_WriteString(5, 105, "Sensor ID:", Font_11x18, ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	ILI9341_WriteString(5, 125, "Sensor Temp.:", Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	lcd_config_ch_load_sensor_value();

	// LABEL

	ILI9341_WriteString(5, 165, sensors_config_label[language_current],
			Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_BLACK); // "Label: (Max. 56 caract.)"

	if (ch_config_index_edit_mode == ch_config_index_edit_mode_text) {
		ILI9341_DrawRect(4, 183, 311, 41, ILI9341_ORANGE);
	} else {
		ILI9341_DrawRect(4, 183, 311, 41, ILI9341_GREY);
	}

	if (ch_status_tmp[ch_config_index] == 0) {
		color = ILI9341_YELLOW; // ILI9341_BLUE
	} else {
		color = ILI9341_ORANGE; // ILI9341_GREEN;
	}

	lcd_config_ch_label(ch_config_index, color);

	if (save_channel_stop == 1) {
		ch_config_stop(ch_config_index);

		save_channel_stop = 0;
	}
}

void lcd_config_ch_blink_status(void) {
	if (ch_status_tmp[ch_config_index] != 0) {
		sprintf(str, sensors_config_act[language_current]); // "  Ativo  "

		ILI9341_WriteString(180, 45, str, Font_11x18,
		ILI9341_BLACK, ILI9341_ORANGE);

//		ILI9341_WriteString(180, 45, "  ",
//				Font_11x18,
//				ILI9341_RED, ILI9341_BLACK);
	} else {
		sprintf(str, sensors_config_inac[language_current]); // "  Inativo  "

		ILI9341_WriteString(180, 45, str, Font_11x18,
		ILI9341_BLACK, ILI9341_YELLOW);
	}
}

void lcd_config_ch_blink_status2(void) {
	if (ch_status_tmp[ch_config_index] != 0) {
		sprintf(str, sensors_config_act2[language_current]); // "[ Ativo ]"

		ILI9341_WriteString(180, 45, str, Font_11x18,
		ILI9341_ORANGE, ILI9341_BLACK);

//		ILI9341_WriteString(180, 45, "  ",
//				Font_11x18,
//				ILI9341_RED, ILI9341_BLACK);
	} else {
		sprintf(str, sensors_config_inac2[language_current]); // "[ Inativo ]"

		ILI9341_WriteString(180, 45, str, Font_11x18,
		ILI9341_YELLOW, ILI9341_BLACK);
	}
}

void lcd_config_ch_label(int label_index, uint16_t color) {
	char str2[3];
	int i = 0;

	for (i = 0; i < 28; i++) {
		if (channelLabel_tmp[i] == 0) {
			str2[0] = ' ';
		} else {
			str2[0] = channelLabel_tmp[i];
		}

		str2[1] = 0;

		ILI9341_WriteString(5 + (i * 11), 185, str2, Font_11x18, color,
		ILI9341_BLACK);
	}

	for (i = 0; i < 28; i++) {
		if (channelLabel_tmp[i + 28] == 0) {
			str2[0] = ' ';
		} else {
			str2[0] = channelLabel_tmp[i + 28];
		}
		str2[1] = 0;

		ILI9341_WriteString(5 + (i * 11), 205, str2, Font_11x18, color,
		ILI9341_BLACK);
	}
}

void lcd_config_ch_str_start(uint8_t channel) {
	uint8_t canal_start_day10 = 0;
	uint8_t canal_start_day1 = 0;
	uint8_t canal_start_month10 = 0;
	uint8_t canal_start_month1 = 0;
	uint8_t canal_start_year10 = 0;
	uint8_t canal_start_year1 = 0;
	uint8_t canal_start_hour10 = 0;
	uint8_t canal_start_hour1 = 0;
	uint8_t canal_start_minute10 = 0;
	uint8_t canal_start_minute1 = 0;

	if (channel_header_tmp[ch_config_index].start_day < 100) {
		canal_start_day10 = channel_header_tmp[ch_config_index].start_day / 10;
		canal_start_day1 = channel_header_tmp[ch_config_index].start_day
				- (canal_start_day10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_month < 100) {
		canal_start_month10 = channel_header_tmp[ch_config_index].start_month
				/ 10;
		canal_start_month1 = channel_header_tmp[ch_config_index].start_month
				- (canal_start_month10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_year < 100) {
		canal_start_year10 = channel_header_tmp[ch_config_index].start_year
				/ 10;
		canal_start_year1 = channel_header_tmp[ch_config_index].start_year
				- (canal_start_year10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_hour < 100) {
		canal_start_hour10 = channel_header_tmp[ch_config_index].start_hour
				/ 10;
		canal_start_hour1 = channel_header_tmp[ch_config_index].start_hour
				- (canal_start_hour10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_minute < 100) {
		canal_start_minute10 = channel_header_tmp[ch_config_index].start_minute
				/ 10;
		canal_start_minute1 = channel_header_tmp[ch_config_index].start_minute
				- (canal_start_minute10 * 10);
	}

	char str1[20];

	if (date_format == ddmmyy) {
		sprintf(str1, "%d%d/%d%d/%d%d", canal_start_day10, canal_start_day1,
				canal_start_month10, canal_start_month1, canal_start_year10,
				canal_start_year1);
	} else {
		sprintf(str1, "%d%d-%d%d-%d%d", canal_start_month10, canal_start_month1,
				canal_start_day10, canal_start_day1, canal_start_year10,
				canal_start_year1);
	}

	sprintf(str, "%s %d%dh%d%d", str1, canal_start_hour10, canal_start_hour1,
			canal_start_minute10, canal_start_minute1);
}

void lcd_config_ch_str_stop(uint8_t channel) {
	uint8_t canal_stop_day10 = 0;
	uint8_t canal_stop_day1 = 0;
	uint8_t canal_stop_month10 = 0;
	uint8_t canal_stop_month1 = 0;
	uint8_t canal_stop_year10 = 0;
	uint8_t canal_stop_year1 = 0;
	uint8_t canal_stop_hour10 = 0;
	uint8_t canal_stop_hour1 = 0;
	uint8_t canal_stop_minute10 = 0;
	uint8_t canal_stop_minute1 = 0;

	if (channel_header_tmp[ch_config_index].start_minute < 100) {
		canal_stop_day10 = channel_header_tmp[ch_config_index].stop_day / 10;
		canal_stop_day1 = channel_header_tmp[ch_config_index].stop_day
				- (canal_stop_day10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_minute < 100) {
		canal_stop_month10 = channel_header_tmp[ch_config_index].stop_month
				/ 10;
		canal_stop_month1 = channel_header_tmp[ch_config_index].stop_month
				- (canal_stop_month10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_minute < 100) {
		canal_stop_year10 = channel_header_tmp[ch_config_index].stop_year / 10;
		canal_stop_year1 = channel_header_tmp[ch_config_index].stop_year
				- (canal_stop_year10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_minute < 100) {
		canal_stop_hour10 = channel_header_tmp[ch_config_index].stop_hour / 10;
		canal_stop_hour1 = channel_header_tmp[ch_config_index].stop_hour
				- (canal_stop_hour10 * 10);
	}

	if (channel_header_tmp[ch_config_index].start_minute < 100) {
		canal_stop_minute10 = channel_header_tmp[ch_config_index].stop_minute
				/ 10;
		canal_stop_minute1 = channel_header_tmp[ch_config_index].stop_minute
				- (canal_stop_minute10 * 10);
	}

	char str1[20];

	if (date_format == ddmmyy) {
		sprintf(str1, "%d%d/%d%d/%d%d", canal_stop_day10, canal_stop_day1,
				canal_stop_month10, canal_stop_month1, canal_stop_year10,
				canal_stop_year1);
	} else {
		sprintf(str1, "%d%d-%d%d-%d%d", canal_stop_month10, canal_stop_month1,
				canal_stop_day10, canal_stop_day1, canal_stop_year10,
				canal_stop_year1);
	}

	sprintf(str, "%s %d%dh%d%d", str1, canal_stop_hour10, canal_stop_hour1,
			canal_stop_minute10, canal_stop_minute1);
}

void lcd_config_ch_confirm(void) {
	if (ch_status_tmp_confirm == 4) { // Aviso dados não exportados
		if (ch_exported[ch_config_index] == 0) {
			sprintf(str, "%s", config_ch_confirm_label14[language_current]); // "Aviso: Exportar dados"
			lcd_window(0, 20, 320, 200, str, ILI9341_ORANGE, ILI9341_BLACK);

			sprintf(str, "%s", config_ch_confirm_label15[language_current]); // "Dados não exportados"
			ILI9341_WriteString(5, 120, str, Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_RED);

			sprintf(str, "%s", config_ch_confirm_label16[language_current]); // "ENTER: confirmar"
			ILI9341_WriteString(5, 215, str, Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_BLACK);
		} else {
			lcd_screen = LcdScreenCfgCh;

			lcd_refresh = 3;
		}
	} else {
		if ((ch_status_tmp_confirm == 2) || (ch_status_tmp_confirm == 3)) {
			sprintf(str, "%s", config_ch_confirm_label1[language_current]); // "Alterar etiqueta"
			lcd_window(0, 20, 320, 200, str, ILI9341_ORANGE, ILI9341_BLACK);

			sprintf(str, "%s", config_ch_confirm_label2[language_current]); //"Deseja substituir etiqueta?"
			ILI9341_WriteString(5, 80, str, Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_BLACK);
		} else if (ch_status_tmp_confirm < 2) {
			sprintf(str, "%s", config_ch_confirm_label3[language_current]); // "Alterar registros"
			lcd_window(0, 20, 320, 200, str, ILI9341_ORANGE, ILI9341_BLACK);

			if (ch_status_tmp_confirm == 0) {
				sprintf(str, "%s", config_ch_confirm_label4[language_current]); // "Deseja parar registros?"
				ILI9341_WriteString(5, 90, str, Font_11x18,
				ILI9341_LIGHTGREY,
				ILI9341_BLACK);
			} else {
				sprintf(str, "%s", config_ch_confirm_label5[language_current]); // "Deseja iniciar registros?"
				ILI9341_WriteString(5, 90, str, Font_11x18,
				ILI9341_LIGHTGREY,
				ILI9341_BLACK);

				sprintf(str, "%s", config_ch_confirm_label6[language_current]); // "(Bloquear etiqueta?)"
				ILI9341_WriteString(5, 60, str, Font_11x18,
				ILI9341_LIGHTGREY,
				ILI9341_BLACK);
			}
		}

		if (ch_status_tmp_confirm2 == 1) {
			sprintf(str, "%s", config_ch_confirm_label7[language_current]); // "< SIM >"
			ILI9341_WriteString(5, 120, str, Font_11x18,
			ILI9341_RED,
			ILI9341_BLACK);

			sprintf(str, config_ch_confirm_label8[language_current]); // "  NAO  "
			ILI9341_WriteString(155, 120, str, Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_BLACK);
		} else {
			sprintf(str, config_ch_confirm_label11[language_current]); // "  SIM  "
			ILI9341_WriteString(5, 120, str, Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_BLACK);

			sprintf(str, config_ch_confirm_label12[language_current]); // "< NAO >"
			ILI9341_WriteString(155, 120, str, Font_11x18,
			ILI9341_RED,
			ILI9341_BLACK);
		}

		sprintf(str, "%s", config_ch_confirm_label9[language_current]); // "(Sim: substituir dados)"
		ILI9341_WriteString(5, 160, str, Font_11x18,
		ILI9341_LIGHTGREY,
		ILI9341_BLACK);

		sprintf(str, "%s", config_ch_confirm_label10[language_current]); // "(Nao: voltar sem alterar)"
		ILI9341_WriteString(5, 180, str, Font_11x18,
		ILI9341_LIGHTGREY,
		ILI9341_BLACK);

		sprintf(str, "%s", config_ch_confirm_label13[language_current]); // "ENTER: confirmar, ESC: sair"
		ILI9341_WriteString(5, 215, str, Font_11x18,
		ILI9341_LIGHTGREY,
		ILI9341_BLACK);
	}

//	if (save_channel_label == 1) {
//		W25Q64_update_name(ch_config_index, channelLabel[ch_config_index]);
//
//		save_channel_label = 0;
//	}

	screen_first_load = 1;
}

void lcd_config_language(void) {
	uint16_t color;

	sprintf(str, "%s", config_language_label1[language_current]);

	if (screen_first_load == 1) {
		screen_first_load = 0;

		lcd_window(0, 20, 320, 200, str, ILI9341_BLUE, ILI9341_LIGHTGREY);
	}

	if (language_current == lang_en) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] English");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] English");
	}

	ILI9341_WriteString(5, 45, str, Font_11x18, color,
	ILI9341_BLACK);

	if (language_current == lang_es) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] Espanol");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] Espanol");
	}

	ILI9341_WriteString(5, 70, str, Font_11x18, color,
	ILI9341_BLACK);

	if (language_current == lang_ptBr) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] Portug Br");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] Portug Br");
	}

	ILI9341_WriteString(5, 95, str, Font_11x18, color,
	ILI9341_BLACK);

	sprintf(str, "%s", config_language_label2[language_current]);

	ILI9341_WriteString(159, 45, str, Font_11x18, ILI9341_YELLOW,
	ILI9341_BLACK);

	if (date_format == mmddyy) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] %s", config_language_label6[language_current]);
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] %s", config_language_label6[language_current]);
	}

	ILI9341_WriteString(159, 70, str, Font_11x18, color,
	ILI9341_BLACK);

	if (date_format == ddmmyy) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] %s", config_language_label7[language_current]);
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] %s", config_language_label7[language_current]);
	}

	ILI9341_WriteString(159, 95, str, Font_11x18, color,
	ILI9341_BLACK);

	sprintf(str, "%s", config_language_label3[language_current]);

	ILI9341_WriteString(5, 125, str, Font_11x18,
	ILI9341_YELLOW,
	ILI9341_BLACK);

	if (decimal_separator == sepDot) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] 0.00");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] 0.00");
	}

	ILI9341_WriteString(14, 150, str, Font_11x18, color,
	ILI9341_BLACK);

	if (decimal_separator == sepComma) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] 0,00");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] 0,00");
	}

	ILI9341_WriteString(14, 175, str, Font_11x18, color,
	ILI9341_BLACK);

	sprintf(str, "%s", config_language_label4[language_current]);

	ILI9341_WriteString(159, 125, str, Font_11x18,
	ILI9341_YELLOW,
	ILI9341_BLACK);

	if (temperature_unit == TemperatureUnit_F) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] *F");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] *F");
	}

	ILI9341_WriteString(159, 150, str, Font_11x18, color,
	ILI9341_BLACK);

	if (temperature_unit == TemperatureUnit_C) {
		color = ILI9341_GREEN;
		sprintf(str, "[x] *C");
	} else {
		color = ILI9341_LIGHTGREY;
		sprintf(str, "[ ] *C");
	}

	ILI9341_WriteString(159, 175, str, Font_11x18, color,
	ILI9341_BLACK);

	ILI9341_DrawRect(4, 42, 152, 22, ILI9341_BLACK); // English
	ILI9341_DrawRect(4, 67, 152, 22, ILI9341_BLACK); // Espanol
	ILI9341_DrawRect(4, 92, 152, 22, ILI9341_BLACK); // Portug Br
	ILI9341_DrawRect(4, 147, 152, 22, ILI9341_BLACK); // Sep Dot
	ILI9341_DrawRect(4, 172, 152, 22, ILI9341_BLACK); // Sep Comma
	ILI9341_DrawRect(156, 67, 155, 22, ILI9341_BLACK); // Date format 1
	ILI9341_DrawRect(156, 92, 155, 22, ILI9341_BLACK); // Date format 2
	ILI9341_DrawRect(156, 147, 155, 22, ILI9341_BLACK); // Deg *F
	ILI9341_DrawRect(156, 172, 155, 22, ILI9341_BLACK); // Deg *C

	if (language_screen_index == 0) {
		ILI9341_DrawRect(4, 42, 152, 22, ILI9341_RED); // English
	} else if (language_screen_index == 1) {
		ILI9341_DrawRect(4, 67, 152, 22, ILI9341_RED); // Espanol
	} else if (language_screen_index == 2) {
		ILI9341_DrawRect(4, 92, 152, 22, ILI9341_RED); // Portug Br
	} else if (language_screen_index == 3) {
		ILI9341_DrawRect(4, 147, 152, 22, ILI9341_RED); // Sep Dot
	} else if (language_screen_index == 4) {
		ILI9341_DrawRect(4, 172, 152, 22, ILI9341_RED); // Sep Comma
	} else if (language_screen_index == 5) {
		ILI9341_DrawRect(156, 67, 155, 22, ILI9341_RED); // Date format 1
	} else if (language_screen_index == 6) {
		ILI9341_DrawRect(156, 92, 155, 22, ILI9341_RED); // Date format 2
	} else if (language_screen_index == 7) {
		ILI9341_DrawRect(156, 147, 155, 22, ILI9341_RED); // Deg *F
	} else if (language_screen_index == 8) {
		ILI9341_DrawRect(156, 172, 155, 22, ILI9341_RED); // Deg *C
	}

	sprintf(str, "%s", config_language_label5[language_current]);

	ILI9341_WriteString(3, 210, str, Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	ConfigStruct_TypeDef conf_dat;

	W25Q64_read_config(&conf_dat);

	conf_dat.DateFormat = date_format;

	conf_dat.DecimalSeparator = decimal_separator;

	conf_dat.TemperatureUnit = temperature_unit;

	conf_dat.Language = language_current;

	W25Q64_update_config(&conf_dat);
}

void lcd_config_ch_erase(void) {
	sprintf(str, "%s %d %s", config_ch_erase_label1[language_current],
			ch_config_index + 1, config_ch_erase_label2[language_current]); // "Canal %d: Limpar memoria"
	lcd_window(0, 20, 320, 200, str, ILI9341_BLUE, ILI9341_BLACK);
}

void make_menu_str(uint8_t channel) {
	uint8_t canal_start_day10 = 0;
	uint8_t canal_start_day1 = 0;
	uint8_t canal_start_month10 = 0;
	uint8_t canal_start_month1 = 0;
	uint8_t canal_start_year10 = 0;
	uint8_t canal_start_year1 = 0;

	uint8_t canal_lenght_day10 = 0;
	uint8_t canal_lenght_day1 = 0;
	uint8_t canal_lenght_hour10 = 0;
	uint8_t canal_lenght_hour1 = 0;
	uint8_t canal_lenght_minute10 = 0;
	uint8_t canal_lenght_minute1 = 0;

	if (channel_header_current[channel].start_day < 100) {
		canal_start_day10 = channel_header_current[channel].start_day / 10;
		canal_start_day1 = channel_header_current[channel].start_day
				- (canal_start_day10 * 10);
	}

	if (channel_header_current[channel].start_month < 100) {
		canal_start_month10 = channel_header_current[channel].start_month / 10;
		canal_start_month1 = channel_header_current[channel].start_month
				- (canal_start_month10 * 10);
	}

	if (channel_header_current[channel].start_year < 100) {
		canal_start_year10 = channel_header_current[channel].start_year / 10;
		canal_start_year1 = channel_header_current[channel].start_year
				- (canal_start_year10 * 10);
	}

	if (channel_header_current[channel].lenght_day < 100) {
		canal_lenght_day10 = channel_header_current[channel].lenght_day / 10;
		canal_lenght_day1 = channel_header_current[channel].lenght_day
				- (canal_lenght_day10 * 10);
	}

	if (channel_header_current[channel].lenght_hour < 100) {
		canal_lenght_hour10 = channel_header_current[channel].lenght_hour / 10;
		canal_lenght_hour1 = channel_header_current[channel].lenght_hour
				- (canal_lenght_hour10 * 10);
	}

	if (channel_header_current[channel].lenght_minute < 100) {
		canal_lenght_minute10 = channel_header_current[channel].lenght_minute
				/ 10;
		canal_lenght_minute1 = channel_header_current[channel].lenght_minute
				- (canal_lenght_minute10 * 10);
	}

	char str1[18];

	if (date_format == ddmmyy) {
		sprintf(str1, "%d%d/%d%d/%d%d", canal_start_day10, canal_start_day1,
				canal_start_month10, canal_start_month1, canal_start_year10,
				canal_start_year1);
	} else {
		sprintf(str1, "%d%d-%d%d-%d%d", canal_start_month10, canal_start_month1,
				canal_start_day10, canal_start_day1, canal_start_year10,
				canal_start_year1);
	}

	sprintf(str, "%d:  CH%d  %s %d%dd%d%dh%d%dm", channel + 1, channel + 1,
			str1, canal_lenght_day10, canal_lenght_day1, canal_lenght_hour10,
			canal_lenght_hour1, canal_lenght_minute10, canal_lenght_minute1);
}

uint8_t str_add_spaces(char *str_data, int length) {
	int size_item = 0;
	int result = 0;

	size_item = 0;

	for (int k = 0; k < length; k++) {
		if (str_data[k] != 0) {
			size_item++;
		} else {
			break;
		}
	}

	if (size_item < length) {
		for (int k = 0; k < (length - size_item); k++) {
			str_data[size_item + k] = 32;  // add space " "
		}
	} else {
		result = 1;
	}

	str_data[length] = 0;  // terminator

	return result;
}

//uint16_t size_of_char(const char *char_in) {
//	uint16_t ret = 0;
//
//	for (int i = 0; i < sizeof char_in; i++) {
//		if (char_in[i] != 0) {
//			ret++;
//		} else {
//			break;
//		}
//	}
//
//	return ret;
//}

uint8_t lcd_main(int phase) {
	uint8_t menu_index_end = menu_list_begin + 8;
	uint8_t y = 0;

	if (phase == 0) {
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off

		if (screen_first_load == 1) {
			screen_first_load = 0;

			get_data_from_mem_for_main();
			sprintf(str, "%s (%s)", Firm_Mod, Firm_Ver); // Datalogger 2039 v. 0.1 beta
			lcd_window(0, 20, 320, 200, str, ILI9341_DARKBLUE,
			ILI9341_LIGHTGREY);

			sprintf(str, "[%s]", main_u_disk[language_current]); // "[USBdisk]"

			main_udisk_chars_num = 0;

			for (int i = 0; i < sizeof main_u_disk[language_current]; i++) {
				if (main_u_disk[language_current][i] != 0) {
					main_udisk_chars_num++;
				} else {
					break;
				}
			}

//		main_udisk_chars_num = size_of_char(main_u_disk[language_current]);

			ILI9341_WriteString(1, 218, str, Font_11x18, ILI9341_BLUE,
			ILI9341_BLACK);

			main_keyboard_chars_num = 0;

			for (int i = 0; i < sizeof main_keyboard[language_current]; i++) {
				if (main_keyboard[language_current][i] != 0) {
					main_keyboard_chars_num++;
				} else {
					break;
				}
			}

//		main_keyboard_chars_num = size_of_char(main_keyboard[language_current]);

			sprintf(str, "[%s]", main_keyboard[language_current]);

			if (USB_FS_OC_Error_old == 1) {
				USB_FS_OC_Error_old = 0;

				if ((main_keyboard_chars_num + 2) < 10) {
					ILI9341_FillRect(319 - ((8 + 2) * 11), 218, ((8 + 2) * 11),
							18, ILI9341_BLACK);
				}
			}

			ILI9341_WriteString(319 - ((main_keyboard_chars_num + 2) * 11), 218,
					str, Font_11x18, ILI9341_BLUE,
					ILI9341_BLACK);
		} else {
			ILI9341_FillRect(309, 42, 7, 200,
			ILI9341_BLACK);
		}
	} else if (phase == 1) {
		ILI9341_DrawLineV(305, 40, 175,
		ILI9341_DARKBLUE);
	} else if (phase == 2) {
		ILI9341_DrawLineH(0, 215, 320,
		ILI9341_DARKBLUE);
	} else if (phase == 3) {
		int j = 0;

		for (int i = menu_list_begin; i < menu_index_end; i++) {
			if (j == menu_index_position) {
				if ((i >= 0) && (i <= 7)) {
					make_menu_str(i);
				} else {
					sprintf(str, "%s", main_menu_item[language_current][i - 8]);

					str_add_spaces(str, size_menu_item);
				}

				ILI9341_WriteString(5, 45 + (21 * j), str, Font_11x18,
				ILI9341_BLACK,
				ILI9341_GREEN);

				if ((i >= 0) && (i <= 7)) {
					if ((SENSORS[i].FLAG_ERROR) || (SENSORS[i].FLAG_ID_ERROR)) {
						ILI9341_WriteString(5 + (11 * 2), 45 + (21 * j), "*",
								Font_11x18,
								ILI9341_BLACK,
								ILI9341_GREEN);
					}

					if (ch_status[i] != 0) {
						ILI9341_WriteString(5 + (11 * 3), 45 + (21 * j), "<",
								Font_11x18,
								ILI9341_BLACK,
								ILI9341_GREEN);
						ILI9341_WriteString(5 + (11 * 7), 45 + (21 * j), ">",
								Font_11x18,
								ILI9341_BLACK,
								ILI9341_GREEN);
					}
				}
			} else {
				if ((i >= 0) && (i <= 7)) {
					make_menu_str(i);
				} else {
					sprintf(str, "%s", main_menu_item[language_current][i - 8]);

					str_add_spaces(str, size_menu_item);
				}
				ILI9341_WriteString(5, 45 + (21 * j), str, Font_11x18,
				ILI9341_GREY,
				ILI9341_BLACK);

				if ((i >= 0) && (i <= 7)) {
					if ((SENSORS[i].FLAG_ERROR) || (SENSORS[i].FLAG_ID_ERROR)) {
						ILI9341_WriteString(5 + (11 * 2), 45 + (21 * j), "*",
								Font_11x18,
								ILI9341_ORANGE,
								ILI9341_BLACK);
					}

					if (ch_status[i] == 1) {
						ILI9341_WriteString(5 + (11 * 3), 45 + (21 * j), "<",
								Font_11x18,
								ILI9341_ORANGE,
								ILI9341_BLACK);
						ILI9341_WriteString(5 + (11 * 7), 45 + (21 * j), ">",
								Font_11x18,
								ILI9341_ORANGE,
								ILI9341_BLACK);
					}
				}
			}

			j++;
		}
	} else if (phase == 4) {
		y = ((float) (menu_list_begin + menu_index_position) * 5.1);
		ILI9341_FillRect(309, 42 + y, 7, 100,
		ILI9341_DARKBLUE);
	} else if (phase == 5) {
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On
		return TRUE;
	}

	return FALSE;
}

void lcd_button(int left, int top, int w, int h, char *title, int title_size,
		uint16_t border_color, uint16_t font_color) {
	for (int i = 0; i < maxChannel; i++) {
		ILI9341_DrawRect(left + i, top + i, w - (i * 2), h - (i * 2),
				border_color);
	}

	ILI9341_WriteString(left + 8 + ((w - 16 - (title_size * 11)) / 2), top + 12,
			title, Font_11x18, font_color, ILI9341_BLACK);
}

void lcd_window(int left, int top, int w, int h, const char *title,
		uint16_t color, uint16_t font_color) {
	int i = (w / 11) - 1;
	int j;
	char str[i];

	for (j = 0; j < i; j++) {
		str[j] = title[j];
	}

	str[j] = 0;

	ILI9341_FillRect(left, top, w, 20, color);

	ILI9341_FillRect(left + 1, top + 20, w - 2, h, ILI9341_BLACK);

	ILI9341_WriteString(left + 3, top + 2, str, Font_11x18, font_color, color);

	// Horizontal border lines
	ILI9341_DrawLineH(left, top + h + 20 - 1, w, color); // 1dip

	// Vertical border lines
	ILI9341_DrawLineV(left, top + 20, h, color); // 1dip
	ILI9341_DrawLineV(left + w - 1, top + 20, h, color); // 1dip
}

uint8_t lcd_loop(void) {
	int update_window = 0;

	while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
		IWDG_delay_ms(0);
	}

	if (lcd_screen_old != lcd_screen) {
		lcd_screen_old = lcd_screen;

		lcd_phase = 0;

		update_window = 1;

		back_to_main_timeout = 0;
	}

	if (lcd_screen == LcdScreenLanguage) {
		lcd_config_language();
	} else if (lcd_screen == LcdScreenCfgChMemErase) {
		lcd_config_ch_erase();
	} else if (lcd_screen == LcdScreenMemTest) {
		lcd_memory_test();
	} else if (lcd_screen == LcdScreenPasswordConfig) {
		lcd_password_config();
	} else if (lcd_screen == LcdScreenMain) {
		//beep(1, 0);

		if ((lcd_phase < 99) && (lcd_main(lcd_phase) == FALSE)) {
			lcd_phase++;

			return FALSE;
		} else {
			lcd_phase = 99;
		}
	} else if (lcd_screen == LcdScreenUsbExport) {
		lcd_usb_export();
	} else if (lcd_screen == LcdScreenPassword) {
		lcd_password();
	} else if (lcd_screen == LcdScreenGraphic) {

		if ((lcd_phase < 99) && (lcd_graph(lcd_phase) == FALSE)) {
			lcd_phase++;

			return FALSE;
		} else {
			lcd_phase = 99;
		}
//	} else if (lcd_screen == LcdScreenMenu) {
//		lcd_menu();
	} else if (lcd_screen == LcdScreenCfgCh) {
		lcd_config_ch();
	} else if (lcd_screen == LcdScreenCfgChConfirm) {
		lcd_config_ch_confirm();
	} else if (lcd_screen == LcdScreenCfgClk) {
		lcd_config_clock();
	} else if (lcd_screen == LcdScreenMonCh) {
		int check_phase = lcd_sensors_monitor(update_window, lcd_phase);

		if ((lcd_phase < 99) && (check_phase != 2)) {
			if (check_phase != 1)
				lcd_phase++;

			return FALSE;
		} else {
			lcd_phase = 99;
		}
	} else if (lcd_screen == LcdScreenMonChOffset) {
		lcd_sensors_monitor_offset();
	} else if (lcd_screen == LcdScreenBattChr) {
		lcd_batt_charging();
//	} else if (lcd_screen == LcdScreenLangExport) {
//		lcd_lang_export();
	}

	return TRUE;
}

uint8_t lcd_usb_export(void) {
	//char start_str[maxChannel] = { '-', '-', ':', '-', '-', ':', '-', '-' };
	UsrLog("lcd_usb_export");

	HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin, GPIO_PIN_RESET); // USB on

	if (screen_first_load == 1) {
		screen_first_load = 0;

		lcd_window(0, 20, 320, 200, usb_export_title[language_current],
		ILI9341_DARKBLUE,
		ILI9341_LIGHTGREY);
	} else {
		ILI9341_FillRect(3, 120, 315, 18, ILI9341_BLACK);
	}

	sprintf(str, "%s:", usb_export_channel[language_current]);

	ILI9341_WriteString(3, 55, str, Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	sprintf(str, "(%s)", usb_export_note1[language_current]);

	ILI9341_WriteString(3, 80, str, Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	sprintf(str, "(%s)", usb_export_note2[language_current]);

	ILI9341_WriteString(3, 100, str, Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	str[1] = 0;

	for (int i = 0; i < maxChannel; i++) {
		str[0] = i + 49;

		if (export_ch[i] == 0) {
			ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
			ILI9341_GREY,
			ILI9341_BLACK);
		} else {
			if (Appli_state_HS == APPLICATION_READY) {
				ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
				ILI9341_BLACK, ILI9341_GREEN);
			} else {
				ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
				ILI9341_BLACK, ILI9341_GREY);
			}
		}
	}

	uint16_t FileTypeColor = ILI9341_CYAN;

	if (file_type == datFile16) {
		sprintf(str, "Dat:%03d", file_cnt_dat);
		FileTypeColor = ILI9341_LIGHTGREY;
	} else if (file_type == datFile64) {
		sprintf(str, "dat:%03d", file_cnt_dat);
#ifdef output_csv_file
		} else {
		sprintf(str, "CSV:%03d", file_cnt_csv);

		if (file_cnt_csv > 10) {
			if (file_cnt_csv > 5) {
				FileTypeColor = ILI9341_GREEN;
			} else {
				FileTypeColor = ILI9341_RED;
			}
		} else {
			FileTypeColor = ILI9341_YELLOW;
		}
#endif
	}

	ILI9341_WriteString(239, 100, str, Font_11x18,
	ILI9341_BLACK, FileTypeColor);

	ILI9341_DrawRect(1, 140, 317, 30, ILI9341_LIGHTGREY);

	sprintf(str, "%s ", usb_export_label3[language_current]);

	for (int i = 0; i < maxChannel; i++) {
		sprintf(str + strlen(str), "%c", start_str[i]);
	}

	ILI9341_WriteString(3, 180, str, Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	sprintf(str, "(%s)", usb_export_label4[language_current]);

	ILI9341_WriteString(3, 210, str, Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK);

	lcd_progress_bar();

//	for (int i = 0; i < 10; i++) {
//
//		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
//			IWDG_delay_ms(0);
//		}
//
//		ILI9341_FillRect(3 + (32 * i), 145, 25, 20,
//		ILI9341_BLUE);
//
//	}

#ifdef output_csv_file
	ConfigStruct_TypeDef conf_dat;

	W25Q64_read_config(&conf_dat);

	conf_dat.FileType = file_type;

	conf_dat.FileCntCsv = file_cnt_csv;

	W25Q64_update_config(&conf_dat);
#endif

	return TRUE;
}

uint8_t lcd_progress_barX10(void) {
	progress_bar_count++;

//	UsrLog("%u", progress_bar_count);

	if (progress_bar_count == 50) {
		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
			IWDG_delay_ms(0);
		}
		ILI9341_WriteString(168, 120, ".", Font_11x18,
		ILI9341_ORANGE,
		ILI9341_BLACK);
	} else if (progress_bar_count == 100) {
		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
			IWDG_delay_ms(0);
		}
		ILI9341_WriteString(179, 120, ".", Font_11x18,
		ILI9341_ORANGE,
		ILI9341_BLACK);
	} else if (progress_bar_count == 150) {
		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
			IWDG_delay_ms(0);
		}
		ILI9341_WriteString(190, 120, ".", Font_11x18,
		ILI9341_ORANGE,
		ILI9341_BLACK);
	} else if (progress_bar_count == 200) {
		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
			IWDG_delay_ms(0);
		}
		ILI9341_FillRect(168, 120, 33, 18,
		ILI9341_BLACK);
	} else if (progress_bar_count > 200) {
		progress_bar_count = 0;
	}

	return TRUE;
}

uint8_t lcd_progress_bar(void) {
	uint8_t progress = progress_usb_export;

	// IWDG_delay_ms(5000);

	if (progress > 10) {
		progress = 10;
	}

	if (abort_operation > 0) {
		sprintf(str, "Status: Operation aborted");
		ILI9341_WriteString(3, 120, str, Font_11x18,
		ILI9341_BLACK,
		ILI9341_RED);
	} else if (export_error != 0) {
		if (export_error != 99) {
			sprintf(str, "%s %d:", usb_export_error[language_current],
					export_error);
			if (export_error == 1) {
				sprintf(str + strlen(str), " CANNOT MOUNT");
			} else if (export_error == 2) {
				sprintf(str + strlen(str), " FAT FORMATTING?");
			} else if ((export_error == 3) || (export_error == 6)
					|| (export_error == 8) || (export_error == 11)
					|| (export_error == 14)) {
				sprintf(str + strlen(str), " FILE CLOSE");
			} else if ((export_error == 4) || (export_error == 5)
					|| (export_error == 7) || (export_error == 9)
					|| (export_error == 12)) {
				sprintf(str + strlen(str), " FILE WRITE");
			} else if (export_error == 10) {
				sprintf(str + strlen(str), " FILE OPEN");
			} else if (export_error == 13) {
				sprintf(str + strlen(str), " FILE READ");
			} else if (export_error == 15) {
				sprintf(str + strlen(str), " WRONG DATA");
			} else if (export_error == 16) {
				sprintf(str + strlen(str), " DISCONNECT");
			}
		} else {
			sprintf(str, "ERROR: USB %s", usb_export_error99[language_current]);
		}

		ILI9341_WriteString(3, 120, str, Font_11x18,
		ILI9341_BLACK,
		ILI9341_RED);

	} else {
		if (Appli_state_HS == APPLICATION_READY) {
			if ((userFunction_phase != 0)
					&& ((export_data_enabled != 0) || (progress_usb_export != 0))) {
				if (userFunction_phase == 7) {
					while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
						IWDG_delay_ms(0);
					}

					sprintf(str, "Status: %s",
							progress_bar_status2[language_current]);

					ILI9341_FillRect(91, 120, 220, 18, ILI9341_BLACK);

					ILI9341_WriteString(3, 120, str, Font_11x18,
					ILI9341_BLUE,
					ILI9341_BLACK); // "Status: finalizado!"
				} else {
					while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
						IWDG_delay_ms(0);
					}

					ILI9341_FillRect(3, 120, 311, 20, ILI9341_BLACK);

					while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
						IWDG_delay_ms(0);
					}

					if (userFunction_phase == 3) {
						if (scanning_sensor_ID != 1) {
							sprintf(str, "Status: %s (%lu Ln)",
									progress_bar_status[language_current],
									progress_remaining);
						} else {
							sprintf(str, "Status: %s (%lu Ln)",
									progress_bar_status3[language_current],
									progress_remaining);
						}
					} else if (userFunction_phase == 5) {
						sprintf(str, "Status: %s (%lu Ln)",
								progress_bar_status1[language_current],
								progress_remaining);
					} else {
						sprintf(str, "Status: %s",
								usb_export_label1[language_current]);
					}

					ILI9341_FillRect(91, 120, 220, 18, ILI9341_BLACK);

					ILI9341_WriteString(3, 120, str, Font_11x18,
					ILI9341_BLACK,
					ILI9341_YELLOW);
				}

				lcd_draw_progress_bars(progress);
			} else {
				ILI9341_WriteString(3, 120, usb_export_label1[language_current],
						Font_11x18,
						ILI9341_LIGHTGREY,
						ILI9341_BLUE); // "Status: em espera."

				lcd_draw_progress_bars(10);
			}

		} else {
			ILI9341_WriteString(3, 120, usb_export_label2[language_current],
					Font_11x18,
					ILI9341_BLACK,
					ILI9341_ORANGE); // "Status: desconectado."

			lcd_draw_progress_bars(10);
		}
	}

	sprintf(start_str, "%02d:%02d:%02d", tm1.hours, tm1.minutes, tm1.seconds);

	uint8_t update_tmp = 0;

	for (int i = 0; i < maxChannel; i++) {
		if (start_str_old[i] != start_str[i]) {
			start_str_old[i] = start_str[i];
			update_tmp = 1;
		}
	}

	if (update_tmp == 1) {
		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
			IWDG_delay_ms(0);
		}

		sprintf(str, "%s ", usb_export_label3[language_current]);

		for (int i = 0; i < maxChannel; i++) {
			sprintf(str + strlen(str), "%c", start_str[i]);
		}

		ILI9341_WriteString(3, 180, str, Font_11x18,
		ILI9341_LIGHTGREY,
		ILI9341_BLACK);
	}

	return TRUE;
}

void lcd_draw_progress_bars(uint8_t progress) {
	uint16_t color = 0;

	if (progress > 10) {
		progress = 10;
	}

	while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
		IWDG_delay_ms(0);
	}

	ILI9341_FillRect(3, 145, 313, 20, ILI9341_BLACK);

	if (Appli_state_HS == APPLICATION_READY) {
		if ((export_data_enabled == 0) || (userFunction_phase == 7)) {
			color = ILI9341_DARKBLUE;
		} else {
			color = ILI9341_BLUE;
		}
	} else {
		color = ILI9341_DARKGREY;
	}

	for (int i = 0; i < progress; i++) {

		while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
			IWDG_delay_ms(0);
		}

		ILI9341_FillRect(3 + (32 * i), 145, 25, 20, color);
	}

	if (progress < 10) {
		for (int i = progress; i < 10; i++) {

			while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
				IWDG_delay_ms(0);
			}

			ILI9341_FillRect(3 + (32 * i), 145, 25, 20,
			ILI9341_DARKGREY);
		}
	}
}

uint8_t lcd_memory_test_status(uint8_t ch, uint8_t step, uint8_t max) {
	int cnt_test_enabled = 0;

	for (int i = 0; i < maxChannel; i++) {
		if (test_ch[i] == 1) {
			cnt_test_enabled++;
		}
	}

	if (cnt_test_enabled > 0) {
		if (mem_test_enabled == 0) {
			ILI9341_WriteString(3, 195, memory_test_status1[language_current],
					Font_11x18,
					ILI9341_BLUE,
					ILI9341_BLACK); // "Status: Em espera           "
		}
	} else {
		sprintf(str, memory_test_status2[language_current]);
		ILI9341_WriteString(3, 195, str, Font_11x18,
		ILI9341_BLUE,
		ILI9341_BLACK); // "Status: Selecionar canal    "
	}

	return 0;
}

uint8_t lcd_memory_test(void) {
	int x = 0;
	int y = 0;
	uint16_t color = 0;

	if (screen_first_load == 1) {
		screen_first_load = 0;

		lcd_window(0, 20, 320, 200, memory_test_title[language_current],
		ILI9341_BLUE,
		ILI9341_BLACK); // "Testar memoria"
	}

	ILI9341_WriteString(3, 55, memory_test_label1[language_current], Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK); // "Canais:"

	ILI9341_WriteString(3, 80, memory_test_label2[language_current], Font_11x18,
	ILI9341_LIGHTGREY,
	ILI9341_BLACK); // "(T: todos,  N: nenhum)"

	str[1] = 0;

	for (int i = 0; i < maxChannel; i++) {
		str[0] = i + 49;

		if (test_ch[i] == 0) {
			if (ch_status[i] == 1) {
				ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
				ILI9341_BROWN,
				ILI9341_BLACK);
			} else {
				ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
				ILI9341_GREY,
				ILI9341_BLACK);
			}
		} else {
			if (ch_status[i] == 1) {
				ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
				ILI9341_BLACK,
				ILI9341_ORANGE);
			} else {
				ILI9341_WriteString(100 + (20 * i), 55, str, Font_11x18,
				ILI9341_BLACK,
				ILI9341_GREEN);
			}
		}

		IWDG_delay_ms(0);
	}

	for (int i = 0; i < maxChannel; i++) {
		x = i % 2;

		y = i / 2;

		if (memTestStatus[i] == 0) {
			sprintf(str, "CH%d[ ---- ]", i + 1);
			color = ILI9341_LIGHTGREY;
		} else if (memTestStatus[i] == memTestStatusTest) {
			sprintf(str, "CH%d[ %s ]", i + 1,
					memTestStatusStr[memTestStatusTest]);
			color = ILI9341_BLUE;
		} else if (memTestStatus[i] == memTestStatusFail) {
			sprintf(str, "CH%d[ %s ]", i + 1,
					memTestStatusStr[memTestStatusFail]);
			color = ILI9341_RED;
		} else if (memTestStatus[i] == memTestStatusPass) {
			sprintf(str, "CH%d[ %s ]", i + 1,
					memTestStatusStr[memTestStatusPass]);
			color = ILI9341_GREEN;
		} else if (memTestStatus[i] == memTestStatusLock) {
			sprintf(str, "CH%d[ %s ]", i + 1,
					memTestStatusStr[memTestStatusLock]);
			color = ILI9341_ORANGE;
		} else if (memTestStatus[i] == memTestStatusSkip) {
			sprintf(str, "CH%d[ %s ]", i + 1,
					memTestStatusStr[memTestStatusSkip]);
			color = ILI9341_BLUE;
		}

		ILI9341_WriteString(15 + (x * 170), 110 + (y * 20), str, Font_11x18,
				color,
				ILI9341_BLACK);

		IWDG_delay_ms(0);
	}

	lcd_memory_test_status(0, 0, 0);

	ILI9341_WriteString(3, 220, memory_test_label3[language_current],
			Font_11x18,
			ILI9341_LIGHTGREY, ILI9341_BLACK); // "ENTER: Iniciar   ESC: Voltar"

	return TRUE;
}

uint8_t lcd_password_config(void) {
	if ((screen_first_load == 1) || (passwd_config_cnt == 3)) {
		screen_first_load = 0;

		lcd_window(0, 20, 320, 200, password_config_title[language_current],
		ILI9341_ORANGE,
		ILI9341_BLACK); // "Configurar senha"
	} else {
		ILI9341_FillRect(2, 71, 315, 28, ILI9341_BLACK);

		ILI9341_FillRect(2, 131, 315, 28, ILI9341_BLACK);

		ILI9341_DrawRect(2, 191, 315, 28, ILI9341_BLACK);
	}

	if (passwd_config_cnt == 3) {
		ILI9341_WriteString(3, 110, password_config_label1[language_current],
				Font_11x18,
				ILI9341_BLACK,
				ILI9341_GREEN); // "Senha atualizada"

		IWDG_delay_ms(1500);

		passwd_esc_cnt0 = 0;

		lcd_screen = LcdScreenMain;

		screen_first_load = 1;

		lcd_refresh = 3;
	} else if (passwd_config_cnt == 4) {
		ILI9341_WriteString(3, 110, password_config_label2[language_current],
				Font_11x18,
				ILI9341_BLACK,
				ILI9341_RED); // "Senha incorreta"

		IWDG_delay_ms(1500);

		passwd_esc_cnt0 = 0;

		lcd_screen = LcdScreenMain;

		screen_first_load = 1;

		lcd_refresh = 3;
	} else {
		ILI9341_WriteString(3, 50, password_config_label3[language_current],
				Font_11x18,
				ILI9341_YELLOW,
				ILI9341_BLACK); // "Digite a senha atual:"

		ILI9341_DrawRect(1, 70, 317, 30, ILI9341_LIGHTGREY);

		ILI9341_WriteString(3, 110, password_config_label4[language_current],
				Font_11x18,
				ILI9341_YELLOW,
				ILI9341_BLACK); // "Digite a senha nova:"

		ILI9341_DrawRect(1, 130, 317, 30, ILI9341_LIGHTGREY);

		ILI9341_WriteString(3, 170, password_config_label4[language_current],
				Font_11x18,
				ILI9341_YELLOW,
				ILI9341_BLACK); // "Digite a senha nova:"

		ILI9341_DrawRect(1, 190, 317, 30, ILI9341_LIGHTGREY);
	}

	if (save_new_passwd == 1) {
		ConfigStruct_TypeDef conf_dat;

		W25Q64_read_config(&conf_dat);

		for (int i = 0; i < 6; i++) {
			conf_dat.PW_Code[i] = password_code[i];
		}

		W25Q64_update_config(&conf_dat);

		save_new_passwd = 0;
	}

	return TRUE;
}

uint8_t lcd_password(void) {
	lcd_window(0, 20, 320, 200, password_title[language_current],
	ILI9341_ORANGE,
	ILI9341_BLACK);

	ILI9341_DrawRect(1, 120, 317, 30, ILI9341_LIGHTGREY);

	ILI9341_WriteString(3, 190, "(Contrasena)", Font_11x18,
	ILI9341_YELLOW,
	ILI9341_BLACK);

	ILI9341_WriteString(3, 210, "(Password)", Font_11x18,
	ILI9341_YELLOW,
	ILI9341_BLACK);

	if (save_new_passwd == 1) {
		ConfigStruct_TypeDef conf_dat;

		W25Q64_read_config(&conf_dat);

		for (int i = 0; i < 6; i++) {
			conf_dat.PW_Code[i] = password_code[i];
		}

		W25Q64_update_config(&conf_dat);

		save_new_passwd = 0;
	}

	return TRUE;
}

void lcd_password_blink(uint16_t color) {
	ILI9341_WriteString(3, 100, password_label1[language_current], Font_11x18,
			color,
			ILI9341_BLACK); // "Digite a senha:"
}

void lcd_data_process(int channel) {
	maxVal[channel] = 0;
	minVal[channel] = 9999;

	for (int i = 0; i < dataLength; i++) {
		if (maxVal[channel] < dataChart[channel][i]) {
			maxVal[channel] = dataChart[channel][i];
		}

		if (minVal[channel] > dataChart[channel][i]) {
			if (dataChart[channel][i] > 0) {
				minVal[channel] = dataChart[channel][i];
			} else if (dataChart[channel][i] < 0) {
				//UsrLog("Negative value: %d", dataChart[channel][i]);
			}
		}
	}

	for (int i = 0; i < dataLength; i++) {
		int data_buff = dataChart[channel][i];

		if (data_buff < minVal[channel]) {
			data_buff = minVal[channel];
		}

		dataChart[channel][i] = map(data_buff, minVal[channel], maxVal[channel],
				0, 250);
	}
}

uint8_t lcd_graph(int phase) {
	float k = 0.0;
	float temper = 0;

	while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
		IWDG_delay_ms(0);
	}

	if (phase == 0) {
		sprintf(str, "%d:", channelSelected + 1);
		lcd_window(0, 20, 320, 200, str, ILI9341_DARKBLUE,
		ILI9341_LIGHTGREY);
	} else if (phase == 1) {
		for (int j = 0; j < maxChannel; j++) {
			for (int i = 0; i < dataLength; i++) {
				if (ch_mark_cnt[j] < 2) {
					dataChart[j][i] = 0;
				} else {
					k = ch_mark_cnt[j]; //datacsv0_size;

					//UsrLog("k: %.0f", k);

					if (chart_zoom_enable[j] != 0) {
						if (k
								> ((10080.0 * (chart_zoom_week[j] + 1))
										+ (1440.0 * (chart_zoom_day[j] + 1))
										+ (60.0 * (chart_zoom_hour[j] + 1)))) {
							k = ((10080.0 * (chart_zoom_week[j] + 1))
									+ (1440.0 * (chart_zoom_day[j] + 1))
									+ (60.0 * (chart_zoom_hour[j] + 1)));
						} else if (k
								> ((1440.0 * (chart_zoom_day[j] + 1))
										+ (60.0 * (chart_zoom_hour[j] + 1)))) {
							k = ((1440.0 * (chart_zoom_day[j] + 1))
									+ (60.0 * (chart_zoom_hour[j] + 1)));
						} else if (k > ((60.0 * (chart_zoom_hour[j] + 1)))) {
							k = (60.0 * (chart_zoom_hour[j] + 1));
						}
					}

					k /= (float) dataLength;

					HAL_StatusTypeDef result_test_data = HAL_BUSY;

					DadaStruct_TypeDef data_buffer;

					read_data_steps = 0;

					while (result_test_data == HAL_BUSY) {
						result_test_data = W25Q64_read_data(j, (float) (i) * k,
								&data_buffer);
					}

					temper = DS18B20_bin_to_float(data_buffer.Temperature);

					if (temper < 0) {
						//UsrLog("Negative value: %.4f", temper);
					}

					dataChart[j][i] = temper * 100; //datacsv0[i * k] * 100;
				}
			}
		}

	} else if (phase == 2) {
		lcd_data_process(0);
	} else if (phase == 3) {
		lcd_data_process(1);
	} else if (phase == 4) {
		lcd_data_process(2);
	} else if (phase == 5) {
		lcd_data_process(3);
	} else if (phase == 6) {
		lcd_data_process(4);
	} else if (phase == 7) {
		lcd_data_process(5);
	} else if (phase == 8) {
		lcd_data_process(6);
	} else if (phase == 9) {
		lcd_data_process(7);
	} else if ((phase >= 10) && (phase < 23)) {
		drawLegends(phase - 10);
	} else if ((phase >= 23) && (phase < 41)) // 10 + drawLegends(13) = 23
			{
		drawGrids(phase - 23);
	} else if (phase == 41) // 23 + drawGrids(17) = 41
			{
		for (int j = 0; j < maxChannel; j++) {
			for (int i = 0; i < 320; i++) {
				dataChartOld[j][i] = dataChart[j][i];
			}
		}
	} else if (phase == 42) {
		float maxValBuff = maxVal[channelSelected] / 100;
		float minValBuff = minVal[channelSelected] / 100;

		sprintf(str, "%d:[%.1f~%.1f]%s", channelSelected + 1, minValBuff,
				maxValBuff, channelLabel[channelSelected]);

//		UsrLog("%s", channelLabel[channelSelected]);

		for (int i = 0; i < 28; i++) {
			if (str[i] == ']') {
				break;
			}

			if (decimal_separator == 1) {
				if (str[i] == '.') {
					str[i] = ',';
				}
			}

		}

		str[28] = 0;

		ILI9341_WriteString(3, 22, str, Font_11x18, ILI9341_LIGHTGREY,
		ILI9341_DARKBLUE);

		if (chart_zoom_enable[channelSelected] != 0) {
			while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
				IWDG_delay_ms(0);
			}

			ILI9341_WriteString(211, 22, "Z:", Font_11x18,
			ILI9341_LIGHTGREY,
			ILI9341_BLACK);
			uint16_t color_w = ILI9341_BLACK;
			uint16_t color_d = ILI9341_BLACK;
			uint16_t color_h = ILI9341_BLACK;

			if (chart_zoom[channelSelected] == Zoom_Week) {
				color_w = ILI9341_RED;
			} else if (chart_zoom[channelSelected] == Zoom_Day) {
				color_d = ILI9341_RED;
			} else if (chart_zoom[channelSelected] == Zoom_Hour) {
				color_h = ILI9341_RED;
			}

			while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
				IWDG_delay_ms(0);
			}

			sprintf(str, "w%d", chart_zoom_week[channelSelected]);

			ILI9341_WriteString(235, 22, str, Font_11x18,
			ILI9341_LIGHTGREY, color_w);

			while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
				IWDG_delay_ms(0);
			}

			sprintf(str, "d%d", chart_zoom_day[channelSelected]);

			ILI9341_WriteString(259, 22, str, Font_11x18,
			ILI9341_LIGHTGREY, color_d);

			while ((lcd_inter_busy == TRUE) || (acc_busy == TRUE)) {
				IWDG_delay_ms(0);
			}

			sprintf(str, "h%d", chart_zoom_hour[channelSelected]);

			ILI9341_WriteString(283, 22, str, Font_11x18,
			ILI9341_LIGHTGREY, color_h);
		}
	} else if (phase == 43) {
		return TRUE;
	}

	return FALSE;
}

uint8_t drawLegends(int phase) {
	char txtData[3] = { 'F', 0, 0 };

	uint16_t dataColorTimeout[maxChannel];
	uint16_t dataColorTimeout2[maxChannel];

	for (int i = 0; i < maxChannel; i++) {
		if (SENSORS[i].FLAG_TIMEOUT == 1) {
			dataColorTimeout[i] = ILI9341_LIGHTGREY;
			dataColorTimeout2[i] = dataColor[i];
		} else {
			dataColorTimeout[i] = dataColor[i];
			dataColorTimeout2[i] = ILI9341_BLACK;
		}
	}

	if (phase == 0) {
		for (int i = 0; i < maxChannel; i++) {
			ILI9341_DrawRect(i * 40, 200, 40, 40, ILI9341_DARKBLUE);
		}
	} else if ((phase >= 1) && (phase <= 8)) {
		ILI9341_FillRect(((phase - 1) * 40) + 1, 201, 38, 38,
				dataColorTimeout[phase - 1]);
	} else if (phase == 9) {
		for (int i = 0; i < maxChannel; i++) {
			if (SENSORS[i].FLAG_ENABLED == 1) {
				ILI9341_WriteString((40 * i) + 2, 203, "LG", Font_7x10,
				ILI9341_BLACK, dataColorTimeout[i]); // sensor enabled
			} else {
				ILI9341_WriteString((40 * i) + 2, 203, "DL", Font_7x10,
				ILI9341_BLACK, dataColorTimeout[i]); // sensor disabled
			}
		}
	} else if (phase == 10) {
		txtData[1] = 0;
		for (int i = 0; i < maxChannel; i++) {
			txtData[0] = i + 49;
			ILI9341_WriteString((40 * i) + 11, 213, txtData, Font_16x26,
					dataColorTimeout2[i], dataColorTimeout[i]); // ch number
		}
	} else if (phase == 11) {
		for (int i = 0; i < maxChannel; i++) {
			if ((SENSORS[i].FLAG_PRESENT == 1)
					& (SENSORS[i].FLAG_TIMEOUT != 1)) {
				ILI9341_WriteString((40 * i) + 24, 203, "PR", Font_7x10,
				ILI9341_BLACK, dataColorTimeout[i]); // sensor present
			} else {
				ILI9341_WriteString((40 * i) + 24, 203, "NP", Font_7x10,
				ILI9341_BLACK, dataColorTimeout[i]); // sensor not present
			}

		}

		for (int i = 0; i < maxChannel; i++) {
			if (SENSORS[i].FLAG_TIMEOUT == 1) {
				ILI9341_DrawLine((40 * i) + 5, 217, (40 * i) + 35, 227,
				ILI9341_BLACK);
				ILI9341_DrawLine((40 * i) + 5, 227, (40 * i) + 35, 217,
				ILI9341_BLACK);
			}
		}
	} else if (phase == 12) {
		return TRUE;
	}

	return FALSE;
}

uint8_t drawGrids(int phase) {
	int left = 0;

	left = 0; // ((0.5 * width) * k);

	if (phase == 0) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[0], ILI9341_BLACK); // erase
	} else if (phase == 1) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[1], ILI9341_BLACK); // erase
	} else if (phase == 2) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[2], ILI9341_BLACK); // erase
	} else if (phase == 3) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[3], ILI9341_BLACK); // erase
	} else if (phase == 4) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[4], ILI9341_BLACK); // erase
	} else if (phase == 5) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[5], ILI9341_BLACK); // erase
	} else if (phase == 6) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[6], ILI9341_BLACK); // erase
	} else if (phase == 7) {
		drawLines(left + 1, 41, 320, 160, dataChartOld[7], ILI9341_BLACK); // erase
	} else if (phase == 8) {
		drawGrid(left, 40, 320, 160);
	} else if (phase == 9) {
		if (dataColor[0] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[0], dataColor[0]); //show
	} else if (phase == 10) {
		if (dataColor[1] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[1], dataColor[1]); //show
	} else if (phase == 11) {
		if (dataColor[2] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[2], dataColor[2]); //show
	} else if (phase == 12) {
		if (dataColor[3] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[3], dataColor[3]); //show
	} else if (phase == 13) {
		if (dataColor[4] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[4], dataColor[4]); //show
	} else if (phase == 14) {
		if (dataColor[5] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[5], dataColor[5]); //show
	} else if (phase == 15) {
		if (dataColor[6] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[6], dataColor[6]); //show
	} else if (phase == 16) {
		if (dataColor[7] != ILI9341_DARKGREY)
			drawLines(left + 1, 41, 320, 160, dataChart[7], dataColor[7]); //show
	} else if (phase == 17) {
		return TRUE;
	}

	return FALSE;
}

void drawGrid(int left, int top, int width, int height) {
	// Horizontal grid lines
	for (int j = 0; j < 4; j++) {
		ILI9341_DrawLineH(left + 1,
				top + (0.2 * (float) height)
						+ ((0.2 * (float) height) * (float) j),
				(left + width) - 2, ILI9341_DARKGREY); // darkGray ILI9341_DARKGREY
	}

	// Vertical grid lines
	for (int j = 0; j <= 14; j++) {
		ILI9341_DrawLineV(
				left + (0.0625 * (float) width)
						+ ((0.0625 * (float) width) * (float) j) + 1, top + 1,
				(height) - 2, ILI9341_DARKGREY);
	}
}

void drawLines(int left, int top, int width, int height, int *dataBuff,
		int lineColor) { //, int mode) {
	int x1 = 0;
	int x2 = 0;

	int y1 = 0;
	int y2 = 0;

	float stepFloat = width - 1;
	stepFloat = stepFloat / dataLength;

	for (int i = 0; i < (dataLength - 2); i++) {
		x1 = (i * stepFloat); // + borderDip;
		x2 = x1 + 1 + stepFloat; // ' +1: to connect to previews trace
		y1 = ((height - 3.0) * ((250.0 - dataBuff[i]) / 250.0)); // + 1.0;
		y2 = ((height - 3.0) * ((250.0 - dataBuff[i + 1]) / 250.0)); // + 1.0;
		ILI9341_DrawLine(x1 + left, y1 + top, x2 + left, y2 + top, lineColor);
	}
}
