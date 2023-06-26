/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file            : usb_host.c
 * @version         : v1.0_Cube
 * @brief           : This file implements the USB Host
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_hid.h"

/* USER CODE BEGIN Includes */
#include "DS3231.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "fatfs.h"
#include "ILI9341.h"
#include "SCREEN.h"
#include "usbh_hid_keybd.h"
#include "EEPROM_EXTERN.h"
#include "BMP_to_USB.h"
#include "num_base_64.h"
#include "key_map.h"
//#include "language.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define hide_index 1
#define compact_using_num_base_64 1
//#define compact_using_num_base_16 1
#define output_dat_file 1

#define lcd_bright_max 1023
#define lcd_bright_min 100

HID_KEYBD_Info_TypeDef *keybd_info1;
int keyboard_timeout_reset = 5; // 5
int keyboard_timeout = 0; // 60
int keyboard_timeout2 = 0;  // 60
int usb_pwr_off_timeout = 0;

int usb_power_restart_timeout = 0;
int usb_power_restart_count = 0;
int usb_power_recycle_count = 0;

int keyboard_status = 0;
int keyboard_status2 = 0;

//uint8_t caps_lock_state = 0;
uint8_t key_insert_accent = FALSE;

//int accent1 = 0;
uint8_t key_grave_accent = FALSE;
uint8_t key_acute_accent = FALSE;
uint8_t key_circumflex_accent = FALSE;
uint8_t key_tilde_accent = FALSE;
uint8_t key_diaeresis_accent = FALSE;

uint8_t keyboard_insert = FALSE;

uint32_t enumTimeout = 0;

uint32_t progress_remaining = 0;
uint8_t progress_usb_export = 0; // 0-10
uint8_t progress_usb_export_old = 0; // 0-10
uint32_t progress_tick = 0;

uint32_t usb_write_data_steps = 0;
//uint32_t usb_read_data_steps = 0;
uint8_t export_data_step = 0;
uint8_t export_in_progress = 0;
uint8_t export_error = 0;

//uint8_t save_BMP_in_progress = 0;
uint8_t save_BMP_error = 0;

uint32_t mouse_interval = 0;
const uint8_t mouse_interval_ms = 100;

FATFS USBDISKFatHs; /* File system object for USB disk logical drive */
FIL MyFile; /* File object */
char USBDISKPath[4] = { 48, 58, 47, 0 }; /* USB Host logical drive path */
USBH_HandleTypeDef hUSB_Host; /* USB Host handle */

uint8_t kbd_LED_status[1] = { 0B001 }; // kbd_num_lock_state = 1
uint8_t kbd_LED_status_old = 0;
uint32_t keyboard_led_status_tick = 0;
//uint8_t kbd_num_lock_state = 0;
//uint8_t kbd_caps_lock_state = 0;
//uint8_t kbd_scroll_lock_state = 0;

uint32_t userFunction_tick = 0;
uint32_t userFunction_phase = 0;
uint32_t userFunction_index1 = 0;
//uint32_t bytesread = 0;
//uint32_t byteswrote = 0;
uint32_t line_count = 0;
//int32_t j = 0;
uint32_t br = 0;
uint32_t highest_cnt = 0;
uint32_t abort_operation = 0;

uint8_t export_date = 0;
uint8_t export_month = 0;
uint8_t export_year = 0;
uint8_t export_hours = 0;
uint8_t export_minutes = 0;

uint32_t ch_mark_cnt_export[maxChannel];

uint8_t esc_cnt = 0;

uint8_t ctrl_atl_del_cnt = 0;
uint8_t ctrl_atl_del_restart = 0;
uint8_t ctrl_atl_del_timeout = 0;

/* ----------------------- */

HID_MOUSE_Info_TypeDef *mouse_info_local;

HID_CUSTOM_Info_TypeDef *custom_info_local;

// M.W.K.B.T.: Mini Wireless Keyboard with Built-in Touchpad
typedef enum {
	CODE_BTN_NEXT = 0xB5,
	CODE_BTN_PREV = 0xB6,
	CODE_BTN_PLAY_PAUSE = 0xCD,
	CODE_BTN_MUTE = 0xE2,
	CODE_BTN_VOL_UP = 0xE9,
	CODE_BTN_VOL_DOWN = 0xEA,
	CODE_BTN_MEDIA_PLAYER = 0x83,
	CODE_BTN_EMAIL = 0x8A,
	CODE_BTN_BROWSER = 0x96,
	CODE_BTN_SEARCH = 0x21,
	CODE_BTN_HOME = 0x23,
} MWKBT1_TypeTypeDef;

typedef enum {
	MWKBT_EMPTY = 0,
	MWKBT_VALID,
	MWKBT_NEXT,
	MWKBT_PREV,
	MWKBT_PLAY_PAUSE,
	MWKBT_MUTE,
	MWKBT_VOL_UP,
	MWKBT_VOL_DOWN,
	MWKBT_MEDIA_PLAYER,
	MWKBT_EMAIL,
	MWKBT_BROWSER,
	MWKBT_SEARCH,
	MWKBT_HOME,
} MWKBT2_TypeTypeDef;

uint8_t MWKBT_array_cod[11] = { CODE_BTN_NEXT, CODE_BTN_PREV,
		CODE_BTN_PLAY_PAUSE, CODE_BTN_MUTE, CODE_BTN_VOL_UP, CODE_BTN_VOL_DOWN,
		CODE_BTN_MEDIA_PLAYER, CODE_BTN_EMAIL, CODE_BTN_BROWSER,
		CODE_BTN_SEARCH, CODE_BTN_HOME };

uint8_t MWKBT_array_ref[11] = { MWKBT_NEXT, MWKBT_PREV, MWKBT_PLAY_PAUSE,
		MWKBT_MUTE, MWKBT_VOL_UP, MWKBT_VOL_DOWN, MWKBT_MEDIA_PLAYER,
		MWKBT_EMAIL, MWKBT_BROWSER, MWKBT_SEARCH, MWKBT_HOME };

char *MWKBT_array_name[11] = { "Next", "Previous", "Play/Pause", "Mute",
		"Volume Up", "Volume Down", "Media Player", "e-mail", "Browser",
		"Search", "Home" };

typedef struct _HID_Multimedia_Touchpad {
	uint8_t x;
	uint8_t y;
	uint8_t scroll;
	uint8_t valid;
	uint8_t buttons[3];
	uint8_t multimedia;
	uint8_t multimedia_size;
//	uint8_t valid;
} HID_MWKBT_TypeDef;

HID_MWKBT_TypeDef MWKBT_decoded;

HID_KEYBD_Info_TypeDef *keybd_info1;

// M.K.LITE-ON: Multimedia Keyboard Lite-On
typedef enum {
	MKLITEON_COD_NEXT = 0xB5,
	MKLITEON_COD_PREV = 0xB6,
	MKLITEON_COD_PLAY_PAUSE = 0xCD,
	MKLITEON_COD_MUTE = 0xE2,
	MKLITEON_COD_VOL_UP = 0xE9,
	MKLITEON_COD_VOL_DOWN = 0xEA,
	MKLITEON_COD_STOP = 0xB7
} MKLITEON1_TypeTypeDef;

typedef enum {
	MKLITEON_EMPTY = 0,
	MKLITEON_VALID,
	MKLITEON_NEXT,
	MKLITEON_PREV,
	MKLITEON_PLAY_PAUSE,
	MKLITEON_MUTE,
	MKLITEON_VOL_UP,
	MKLITEON_VOL_DOWN,
	MKLITEON_STOP
} MKLITEON2_TypeTypeDef;

uint8_t MKLITEON_array_cod[7] = { MKLITEON_COD_NEXT, MKLITEON_COD_PREV,
		MKLITEON_COD_PLAY_PAUSE, MKLITEON_COD_MUTE, MKLITEON_COD_VOL_UP,
		MKLITEON_COD_VOL_DOWN, MKLITEON_COD_STOP };

uint8_t MKLITEON_array_ref[7] = { MKLITEON_NEXT, MKLITEON_PREV,
		MKLITEON_PLAY_PAUSE, MKLITEON_MUTE, MKLITEON_VOL_UP, MKLITEON_VOL_DOWN,
		MKLITEON_STOP };

char *MKLITEON_array_name[7] = { "NEXT", "PREV", "PAUSE", "MUTE", "VOL_UP",
		"VOL_DOWN", "STOP" };

typedef struct _HID_MKLITEON {
	uint8_t multimedia;
	uint8_t multimedia_size;
	uint8_t valid;
} HID_MKLITEON_TypeDef;

HID_MKLITEON_TypeDef MKLITEON_decoded;

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
FRESULT set_timestamp(char *obj); /* Pointer to the file name */
FRESULT file_exists(char *filename);
void MX_USB_HOST_Process_HS(void);
void MX_USB_HOST_Process_FS(void);
void keyboardMain(HID_KEYBD_Info_TypeDef *keybd_info);
static void Datalogger2039_HID_handler(HID_KEYBD_Info_TypeDef *keybd_info1);

uint8_t userFunctionExport(void);
HAL_StatusTypeDef save_BMP_Export(void);
HAL_StatusTypeDef save_BMP_function(void);
void get_data_header(void);
void get_data_header2(void);
void replace_char(char *char_array, uint8_t find, uint8_t replace);
void get_data(uint32_t posit);
uint8_t userFunction(void);
void update_exported_data(void);

extern uint8_t USBH_isInitialized;

extern uint8_t file_type;
extern uint16_t file_cnt_csv;
extern uint16_t file_cnt_dat;

extern uint8_t language_current;

extern uint8_t decimal_separator;

extern uint32_t uC_ID[3];
extern uint8_t MEM_ID[8];

extern int cnt_timeout_reset_USB;
extern uint8_t temperature_unit;
extern uint8_t date_format;
extern uint32_t read_data_steps;
//extern char str[80];
//uint16_t max_index_tab_desc_event = 256;
//extern uint32_t ch_mark_cnt[maxChannel];
extern int ch_status[maxChannel];
extern uint8_t export_ch[maxChannel];
extern DadaStruct3_TypeDef canal_header[maxChannel];

extern uint32_t sizeof_array(const char *input);
extern void BASE64_encode(const char *input, char *output);
extern void beep(uint8_t repetions, uint8_t is_error);
extern void BMP_to_USB_header(uint8_t *header_buf);
extern float DS18B20_celsius_to_fahrenheit(float celsius);
extern USBH_StatusTypeDef USBH_HandleEnum(USBH_HandleTypeDef *phost);
//extern USBH_StatusTypeDef  DeInitStateMachine(USBH_HandleTypeDef *phost);
//extern void lcd_config_ch_label(void);
extern void lcd_menu_keyboard(HID_KEYBD_Info_TypeDef *keybd_info);
extern void user_pwm_setvalue(uint16_t value);
extern void lcd_config_clock_load(void);
extern uint8_t lcd_progress_bar(void);
extern uint8_t lcd_progress_barX10(void);

extern USBH_StatusTypeDef DeInitStateMachine(USBH_HandleTypeDef *phost);
extern USBH_StatusTypeDef USBH_ReEnumerate(USBH_HandleTypeDef *phost);

USBH_HandleTypeDef hUsbHostHS;
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state_HS = APPLICATION_IDLE;
ApplicationTypeDef Appli_state_FS = APPLICATION_IDLE;

/* ----------------------- */

static HID_MWKBT_TypeDef* USBH_HID_Decode_Mini_Keyboard_Touchpad(
		HID_MOUSE_Info_TypeDef *Minfo);
static void USBH_HID_MouseLogiDecode(HID_MOUSE_Info_TypeDef *Minfo);
static void USBH_HID_MultmediaKbDecode(HID_CUSTOM_Info_TypeDef *Cinfo);
/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */
extern volatile HID_KEYBD_Info_TypeDef keybd_info_lcd;

#define NumOfSensors 8
extern DS18B20_TypeDef SENSORS[NumOfSensors];
extern uint32_t ch_mark_cnt[maxChannel];
extern const uint16_t lcd_timeout0recall;
extern const uint16_t lcd_timeout0recall2;
extern uint8_t button1_stage;

extern uint8_t lcd_timeout0;
extern uint8_t lcd_restart;
//extern uint32_t datacsv0_size;
extern int keyboard_timeout_reset;
extern bool lcd_enabled;
//extern bool clock1_setup;
extern DS3231_Time_t clock1;
extern int clock_index_edit_mode;
//extern bool key_insert_accent;

//extern bool keyboard_insert;
extern int ch_config_index;
//extern int keyboard_status;
extern int keyboard_timeout;
extern int keyboard_timeout2;

uint8_t uart_tx_buffer[100];
extern HID_MOUSE_Info_TypeDef mouse_info;
//extern void keyboardMain(uint8_t key);
extern bool USB_FS_restart;
extern int lcd_screen;
extern int label_index_edit;
extern int label_index_edit2;

extern char channelLabel[maxChannel][57];
extern uint8_t lcd_refresh;
extern int menu_button;
extern uint8_t timeout0;
//extern bool standby_mode;

extern int lcd_bright_set;
//extern bool lcd_restart;
extern int lcd_bright_new;
extern const uint16_t dataColorRecall[maxChannel];
extern uint16_t dataColor[maxChannel];
extern uint8_t channelSelected;
extern int start_MSC_Application1;
extern int start_MSC_Application2;
extern uint32_t cnt0;
extern uint32_t cnt1;
//extern bool caps_lock_state;
//extern const int maxChar;

extern const uint8_t HID_KEYBRD_ShiftKey[];
extern const uint8_t HID_KEYBRD_Codes[];

extern int accent1;
//extern const float datacsv0[];
//extern float get_data0(uint32_t addr);

extern int clock_index_edit;
extern uint8_t standby_mode;

extern uint8_t save_BMP_enabled;
extern uint8_t save_BMP_step;
extern uint8_t save_BMP_phase;

extern uint8_t export_data_enabled;
//extern uint32_t cnt0;
//extern uint32_t cnt1;
extern DS3231_Time_t tm1;

extern uint8_t ch_exported[8];

/* ----------------------- */

HID_KEYBD_Info_TypeDef *keybd_info1;
extern HID_MOUSE_Info_TypeDef mouse_info;
/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess1(USBH_HandleTypeDef *phost, uint8_t id);
static void USBH_UserProcess2(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */
void update_kbd_info_lcd(HID_KEYBD_Info_TypeDef *keybd_info) {
	keybd_info_lcd.key_ascii = keybd_info->key_ascii;
	keybd_info_lcd.key_special = keybd_info->key_special;
	keybd_info_lcd.keys[0] = keybd_info->keys[0];

	keybd_info_lcd.ralt = keybd_info->ralt;
	keybd_info_lcd.rctrl = keybd_info->rgui;
	keybd_info_lcd.rgui = keybd_info->rshift;
	keybd_info_lcd.rshift = keybd_info->rshift;

	keybd_info_lcd.lalt = keybd_info->lalt;
	keybd_info_lcd.lctrl = keybd_info->lctrl;
	keybd_info_lcd.lgui = keybd_info->lgui;
	keybd_info_lcd.lshift = keybd_info->lshift;

//	keybd_info_lcd.led_num_lock = keybd_info->led_num_lock;
//	keybd_info_lcd.led_caps_lock = keybd_info->led_caps_lock;
//	keybd_info_lcd.led_scroll_lock = keybd_info->led_scroll_lock;

	keybd_info_lcd.key_special = keybd_info->key_special;
}

void keyboardMain(HID_KEYBD_Info_TypeDef *keybd_info) {
	uint8_t key = keybd_info->keys[0];

	cnt_timeout_reset_USB = 0;

	if (key == KEY_INSERT) {
		keyboard_insert = !keyboard_insert;
	} else if (key != 0) {
		usb_power_recycle_count = 0;

		if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port, AC_DC_STATE_Pin)
				== GPIO_PIN_SET) {
			lcd_timeout0 = lcd_timeout0recall2;
		} else {
			lcd_timeout0 = lcd_timeout0recall;
		}

		if (standby_mode == true) {
			standby_mode = false;

			lcd_restart = true;

			lcd_bright_set = lcd_bright_max;
			//			user_pwm_setvalue(lcd_bright_max);
		} else {
			button1_stage = ScreenAutoDimm;

			lcd_bright_set = lcd_bright_max;
		}

		Appli_state_FS = APPLICATION_READY;

		update_kbd_info_lcd(keybd_info);

		lcd_menu_keyboard(keybd_info);

		keyboard_timeout_reset = 5;

		keyboard_timeout = 60;

		keyboard_timeout2 = 60;
	} else if (key == 0) {

		if (start_MSC_Application1 == 1) {
			start_MSC_Application1 = 2;
		} else if (start_MSC_Application2 == 1) {
			start_MSC_Application2 = 2;
		}

		keyboard_timeout = 60;
		//keyboard_status = 3;
	}

	cnt0 = 0;
	cnt1 = 0;

	keyboard_timeout_reset = 5;
}

#define USBH_ADDRESS_DEFAULT                     0x00U
#define USBH_ADDRESS_ASSIGNED                    0x01U
#define USBH_MPS_DEFAULT                         0x40U

uint32_t keyboardMain_debounce = 0;

void USB_Error_Handler(uint16_t code) {
	/* USER CODE BEGIN USB_Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
//HAL_GPIO_WritePin(GPIOE,GPIO_PIN_0,GPIO_PIN_RESET); // LED D2
	USBH_UsrLog("USB_Error_Handler: %d", code);
	/* USER CODE END USB_Error_Handler */
}

#include "ff.h"
#include <stdio.h>

FATFS USBH_fatfs;
FIL MyFile;
FRESULT res;
uint32_t bytesWritten;
char rtext[2500];
char wtext[2500]; // 2080
char name[256];	//file name yymmddhhss.csv
uint32_t counter = 0;

extern char USBHPath[]; /* USBH logical drive path */

extern UART_HandleTypeDef huart3;
uint8_t uart_tx_buffer[100];

uint8_t userFunctionExport(void) {
	for (uint16_t i = 0; i < 512; i++) {
		if (userFunction() == TRUE) {
			export_data_enabled = 0;

			return 0;
		}

		IWDG_delay_ms(0);
	}

	return 1;
}

FRESULT file_exists(char *filename) {
	FIL MyFile1;

	if (f_open(&MyFile1, filename, FA_OPEN_EXISTING) == FR_OK) {
		f_close(&MyFile1);
		UsrLog("file exists");
		return FR_EXIST;
	} else {
		UsrLog("file doesn't exist");
		return FR_NO_FILE;
	}
}

HAL_StatusTypeDef save_BMP_Export(void) {
	if (save_BMP_function() == HAL_OK) {
		//save_BMP_enabled = 0;

		return HAL_OK;
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef save_BMP_function(void) {
//	int16_t x_read = 0;
	int16_t y_read = 0;
	uint16_t buff1[320] = { 0 }; // 3x 320 = 960
	uint8_t buff2[3] = { 0 }; // 3x 320 = 960
	uint32_t color_buff = 0;
	unsigned char header_buff[54];

	if (save_BMP_phase == 1) {
		UsrLog("save_BMP_phase == 1");

		if (Appli_state_HS == APPLICATION_READY) {
//			save_BMP_in_progress = 1;

			/* Register the file system object to the FatFs module */
			if (f_mount(&USBDISKFatHs, (TCHAR const*) USBDISKPath, 0)
					!= FR_OK) {
				/* FatFs Initialization Error */
				USB_Error_Handler(0);

				save_BMP_phase = 0;

				lcd_refresh = 3;
			}

			sprintf(name, "20%02d.%02d.%02d_%02d.%02d.%02d.bmp", tm1.year,
					tm1.month, tm1.date, tm1.hours, tm1.minutes, tm1.seconds);

			/*Create a file*/
			if (f_open(&MyFile, name, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
				/* Creation failed */
				UsrLog("Cannot open %s file \n", name);
				save_BMP_error = 2;
				lcd_refresh = 3;

				save_BMP_phase = 0;
			} else {
				UsrLog("file %s created \n", name);
				/*write message to the file. Use variable wtext, bytesWritten*/

				save_BMP_phase++;
			}
		} else {
			save_BMP_phase = 0;

			lcd_refresh = 3;
		}
	} else if (save_BMP_phase == 2) {
		UsrLog("save_BMP_phase == 2");

		BMP_to_USB_header(header_buff);

		res = f_write(&MyFile, header_buff, sizeof(header_buff),
				(void*) &bytesWritten);

		for (y_read = 239; y_read >= 0; y_read--) {
			//for (y_read = 0; y_read < 239; y_read++) {
			ILI9341_readPixels16(0, y_read, 319, y_read, buff1);

			for (uint16_t i = 0; i < 320; i++) {
				if (buff1[i] == ILI9341_BLUE)
					color_buff = ILI9341_888_BLUE;
				else if (buff1[i] == ILI9341_RED)
					color_buff = ILI9341_888_RED;
				else if (buff1[i] == ILI9341_GREEN)
					color_buff = ILI9341_888_GREEN;
				else if (buff1[i] == ILI9341_CYAN)
					color_buff = ILI9341_888_CYAN;
				else if (buff1[i] == ILI9341_MAGENTA)
					color_buff = ILI9341_888_MAGENTA;
				else if (buff1[i] == ILI9341_YELLOW)
					color_buff = ILI9341_888_YELLOW;
				else if (buff1[i] == ILI9341_WHITE)
					color_buff = ILI9341_888_WHITE;
				else if (buff1[i] == ILI9341_LIGHTGREY)
					color_buff = ILI9341_888_LIGHTGREY;
				else if (buff1[i] == ILI9341_ORANGE)
					color_buff = ILI9341_888_ORANGE;
				else if (buff1[i] == ILI9341_GREY)
					color_buff = ILI9341_888_GREY;
				else if (buff1[i] == ILI9341_DARKGREY)
					color_buff = ILI9341_888_DARKGREY;
				else if (buff1[i] == ILI9341_DARKBLUE)
					color_buff = ILI9341_888_DARKBLUE;
				else if (buff1[i] == ILI9341_DARKCYAN)
					color_buff = ILI9341_888_DARKCYAN;
				else if (buff1[i] == ILI9341_DARKYELLOW)
					color_buff = ILI9341_888_DARKYELLOW;
				else if (buff1[i] == ILI9341_BLUESKY)
					color_buff = ILI9341_888_BLUESKY;
				else if (buff1[i] == ILI9341_BROWN)
					color_buff = ILI9341_888_BROWN;
				else
					color_buff = ILI9341_888_BLACK;

				buff2[0] = color_buff;
				buff2[1] = color_buff >> 8;
				buff2[2] = color_buff >> 16;

				res = f_write(&MyFile, buff2, 3, (void*) &bytesWritten);

				IWDG_delay_ms(0);
			}

			//UsrLog("%lu", len);

			if ((y_read % 10) == 0) {
				UsrLog("line: %u", y_read);
				beep(1, 0);
			}

			IWDG_delay_ms(0);
		}

		save_BMP_phase++;

//		}
	} else if (save_BMP_phase == 3) {
		UsrLog("save_BMP_phase == 3");
		// End MSC_HOST_HANDS_ON: Add the call to userFunction()

//		save_BMP_in_progress = 0;

		/*
		 *
		 * Begin: Data Export (lock content)
		 * Date: 2020-May-06
		 *
		 */

		if (lcd_screen == LcdScreenMonCh) {
			uint8_t u_write_buff[8];

			u_write_buff[0] = tm1.year;
			u_write_buff[1] = tm1.month;
			u_write_buff[2] = tm1.date;

			res = f_write(&MyFile, u_write_buff, 3, (void*) &bytesWritten);

			u_write_buff[0] = tm1.hours;
			u_write_buff[1] = tm1.minutes;
			u_write_buff[2] = tm1.seconds;

			res = f_write(&MyFile, u_write_buff, 3, (void*) &bytesWritten);

			for (int j = 0; j < 3; j++) {
				for (int i = 0; i < 4; i++) {
					u_write_buff[i] = (uC_ID[j] >> (24 - (i * 8))) & 0xFF;
				}

				res = f_write(&MyFile, u_write_buff, 4, (void*) &bytesWritten);

				IWDG_delay_ms(0);
			}

			for (int i = 0; i < 8; i++) {
				u_write_buff[i] = w25qxx.UniqID[i];
			}

			res = f_write(&MyFile, u_write_buff, 8, (void*) &bytesWritten);

			for (int i = 0; i < maxChannel; i++) {
				for (int j = 0; j < maxChannel; j++) {
					u_write_buff[j] = SENSORS[i].SERIAL_ID[j];
				}

				res = f_write(&MyFile, u_write_buff, 8, (void*) &bytesWritten);

				IWDG_delay_ms(0);
			}

			u_write_buff[0] = language_current + 48;

			res = f_write(&MyFile, u_write_buff, 1, (void*) &bytesWritten);
		}

		/*
		 *
		 * End: Data Export (lock content)
		 *
		 */

		/*close the file*/
		f_close(&MyFile);

		set_timestamp(name);

		save_BMP_phase = 0;

		HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin, GPIO_PIN_SET); // USB off

		lcd_refresh = 3;

		return HAL_OK;
	}

	return HAL_BUSY;
}

// source https://forum.arduino.cc/t/writing-binary-file-to-sd-creating-bmp-files/110331/6

/*
 * END of code: https://forum.arduino.cc/t/writing-binary-file-to-sd-creating-bmp-files/110331/6
 */

void get_data_header(void) {
	sprintf(wtext, "%s%c%s%c", Firm_Mod, sepChr, Firm_Ver, sepChr);

	sprintf(wtext + strlen(wtext), "%08lX%08lX%08lX%c", uC_ID[0], uC_ID[1],
			uC_ID[2], sepChr);

	sprintf(wtext + strlen(wtext), "%02X%02X%02X%02X%02X%02X%02X%02X%c",
			MEM_ID[0], MEM_ID[1], MEM_ID[2], MEM_ID[3], MEM_ID[4], MEM_ID[5],
			MEM_ID[6], MEM_ID[7], sepChr);

	sprintf(wtext + strlen(wtext), "%lu%c", (highest_cnt - 1), sepChr);

	uint8_t ch_enabled = 0;

	for (uint8_t i = 0; i < maxChannel; i++) {
		if (export_ch[i] == 1) {
			ch_enabled = (ch_enabled << 1) | 0B1;
		} else {
			ch_enabled = (ch_enabled << 1) & 0B11111110;
		}
	}

	sprintf(wtext + strlen(wtext), "%d%c%d%c%d%c%d%c%d%c%d%c%d", maxChannel,
	sepChr, ch_enabled,
	sepChr, temperature_unit, sepChr, decimal_separator, sepChr, date_format,
	sepChr, language_current, sepChr, file_type);
}

void get_data_header2(void) {
//	if (file_type == datFile64) { // Hide index
//		wtext[0] = 0;
//		wtext[1] = 0;
//	} else { // datFile16 or csvFile
	sprintf(wtext, "Index%c", sepChr);
//	}

	for (int8_t i = 0; i < maxChannel; i++) {
		if (export_ch[i] == 1) {
			char TempUnit = 'C';

			if (temperature_unit == TemperatureUnit_F) {
				TempUnit = 'F';
			}

			sprintf(wtext + strlen(wtext),
					"CH%d: Date%cCH%d: Time (GMT-3)%cCH%d: Temp. %c%c (", i + 1,
					sepChr, i + 1, sepChr, i + 1,
					degChr, TempUnit);

			// UsrLog("channelLabel: %s", channelLabel[i]);

			if (channelLabel[i][0] != 255) {
				char label_tmp[maxChar];

				for (int j = 0; j < maxChar; j++) { // copy
					label_tmp[j] = channelLabel[i][j];
				}

				if (sepChr == 59) {
					replace_char(label_tmp, 59, 124); // ASCII(59): ';', ASCII(124): '|'
				}

				sprintf(wtext + strlen(wtext), "%s", label_tmp);
			} else {
				sprintf(wtext + strlen(wtext), "-");
			}

			sprintf(wtext + strlen(wtext),
					")%cCH%d: Sensor ID%cCH%d: ACC_x(%c)%cCH%d: ACC_y(%c)%cCH%d: Bat.(V)%cCH%d: Carreg.(V)%c",
					sepChr, i + 1, sepChr, i + 1,
					degChr, sepChr, i + 1, degChr,
					sepChr, i + 1,
					sepChr, i + 1, sepChr);
		}
	}

	sprintf(wtext + strlen(wtext), "%cEvent Date%cEvent Time%cEvent Desc.%c",
	sepChr,
	sepChr, sepChr, sepChr);

	if (file_type != csvFile) {
		BASE64_encode(wtext, wtext); // hide data
	}
}

#define shared_array_max 64

uint8_t shared_Sensor_Id64[maxChannel][shared_array_max][8];
uint32_t shared_Sensor_Id64_index[maxChannel][shared_array_max];

uint32_t shared_data_posit = 0;

void shared_data_clear(void) {
	for (uint8_t i = 0; i < maxChannel; i++) {
		for (uint8_t j = 0; j < shared_array_max; j++) {
			for (uint8_t k = 0; k < 8; k++) {
				shared_Sensor_Id64[i][j][k] = 0;
			}

			shared_Sensor_Id64_index[i][j] = 0;
		}
	}

	shared_data_posit = 0;
}

int32_t get_shared_Sensor_Id_index64(uint8_t channel, uint8_t *Sensor_Id,
		uint32_t posit) {
	uint8_t sens_ID_tmp[8];
	uint8_t shared_sens_ID_tmp[8];
	uint8_t compare_empty = 0;
	uint8_t compare_equal = 0;

	for (uint8_t j = 0; j < shared_array_max; j++) {
		for (uint8_t i = 0; i < 8; i++) {
			sens_ID_tmp[i] = Sensor_Id[i];
			shared_sens_ID_tmp[i] = shared_Sensor_Id64[channel][j][i];

			if (shared_sens_ID_tmp[i] == 0) {
				compare_empty++;
			} else {
				if (shared_sens_ID_tmp[i] == sens_ID_tmp[i]) {
					compare_equal++;
				}
			}
		}

		if (compare_equal == 8) {
			return shared_Sensor_Id64_index[channel][j];
		} else {
			if (compare_empty == 8) {
				for (uint8_t i = 0; i < 8; i++) {
					shared_Sensor_Id64[channel][j][i] = sens_ID_tmp[i];
				}

				shared_Sensor_Id64_index[channel][j] = posit;

				return -1; // new entry
			}
		}
	}

	return -2;
}

void replace_char(char *char_array, uint8_t find, uint8_t replace) {
	uint16_t _length = 0;

	_length = sizeof_array(char_array);

	for (uint16_t j = 0; j < _length; j++) {
		if (char_array[j] == find) { // ascii(46) = "."
			char_array[j] = replace; // ascii(44) = ","
		}
	}
}

void get_data(uint32_t posit) {
	HAL_StatusTypeDef result_test_data;

	DadaStruct_TypeDef buff_test_data;

	DadaStruct2b_TypeDef buff_data2[maxChannel];

	uint8_t buff_data_skip[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	for (uint8_t i = 0; i < maxChannel; i++) {
		if ((posit < ch_mark_cnt_export[i]) && (export_ch[i] == 1)) {
			result_test_data = HAL_BUSY;

			read_data_steps = 0;

			while (result_test_data == HAL_BUSY) {
				result_test_data = W25Q64_read_data(i, posit, &buff_test_data);
			}

			if (file_type == csvFile) {
				if (date_format == ddmmyy) {
					sprintf(buff_data2[i].Date, "%02d/%02d/%02d",
							buff_test_data.Date[0], buff_test_data.Date[1],
							buff_test_data.Date[2]);
				} else { // mmddyy:
					sprintf(buff_data2[i].Date, "%02d/%02d/%02d",
							buff_test_data.Date[1], buff_test_data.Date[0],
							buff_test_data.Date[2]);
				}

				sprintf(buff_data2[i].Time, "%02d:%02d", buff_test_data.Time[0],
						buff_test_data.Time[1]);
			} else {
				if (file_type == datFile16) {
					sprintf(buff_data2[i].Date, "%02X%02X%02X",
							buff_test_data.Date[2], buff_test_data.Date[1],
							buff_test_data.Date[0]);

					sprintf(buff_data2[i].Time, "%02X%02X",
							buff_test_data.Time[0], buff_test_data.Time[1]);
				} else { // datFile64
					char y_tmp[3];
					char m_tmp[2];
					char d_tmp[2];

					conv_b10_b64(buff_test_data.Date[0], d_tmp);
					conv_b10_b64(buff_test_data.Date[1], m_tmp);
					conv_b10_b64(buff_test_data.Date[2], y_tmp);

					if (y_tmp[1] == 0) {
						y_tmp[2] = 0;
						y_tmp[1] = y_tmp[0];
						y_tmp[0] = 'A';
					}

					sprintf(buff_data2[i].Date, "%s%s%s", y_tmp, m_tmp, d_tmp);

//					for (uint8_t j = 0; j < 3; j++) {
//						conv_b10_b64(buff_test_data.Date[2 - j], tmp);
//
//						buff_data2[i].Date[j + 1] = tmp[0];
//					}
//
//					buff_data2[i].Date[3] = 0;

					char tmp[2];

					for (uint8_t j = 0; j < 2; j++) {
						conv_b10_b64(buff_test_data.Time[1 - j], tmp);

						buff_data2[i].Time[j] = tmp[0];
					}

					buff_data2[i].Time[2] = 0;
				}
			}

			buff_data2[i].Temperature_f = DS18B20_bin_to_float(
					buff_test_data.Temperature);

			if (file_type == csvFile) {
				DS18B20_hex_id(buff_test_data.Sensor_Id,
						buff_data2[i].Sensor_Id);
			} else {
				if (file_type == datFile16) {
					DS18B20_hex_id(buff_test_data.Sensor_Id,
							buff_data2[i].Sensor_Id);
				} else { // datFile64
					DS18B20_b64_id(buff_test_data.Sensor_Id,
							buff_data2[i].Sensor_Id);

//					DS18B20_b64_id(buff_test_data.Sensor_Id,
//							buff_data2[i].Sensor_Id);
//
//					int32_t ID_index = -5;
//
//					ID_index = get_shared_Sensor_Id_index64(i,
//							buff_test_data.Sensor_Id, posit);
//
//					if (ID_index >= 0) {
//						sprintf(buff_data2[i].Sensor_Id, "%lu", ID_index);
//					}
				}
			}
			//           031297795C2728
			// DEpd5XCco: 31297795C2728
//			}

			buff_data2[i].AccX = buff_test_data.AccX;
			buff_data2[i].AccY = buff_test_data.AccY;
			buff_data2[i].BattV = buff_test_data.BattV;
			buff_data2[i].ChargerV = buff_test_data.ChargerV;

			buff_data_skip[i] = 0;
		} else {
			buff_data_skip[i] = 1;
		}
	}

//	if (file_type == datFile64) { // Hide index
//		wtext[0] = 0;
//		wtext[1] = 0;
//		sprintf(wtext, "%lu%c", posit, sepChr);
//	} else { // datFile16 or csvFile
	sprintf(wtext, "%lX%c", posit, sepChr);
//	}

	for (uint8_t i = 0; i < maxChannel; i++) {
		if (export_ch[i] == 1) {
			if ((posit < ch_mark_cnt_export[i]) && (buff_data_skip[i] == 0)) {
				int8_t temperature_i = buff_data2[i].Temperature_f;
				float temperature_dec_f = buff_data2[i].Temperature_f
						- temperature_i;
				temperature_dec_f *= 10000;
				temperature_dec_f /= 625;

				int8_t temperature_dec_i = temperature_dec_f;

				int8_t _AccX = buff_data2[i].AccX - 127;
				int8_t _AccY = buff_data2[i].AccY - 127;

				float _BattV = buff_data2[i].BattV;
				_BattV *= 0.004;
				_BattV += 3.5;

				float _ChargerV = buff_data2[i].ChargerV;
				_ChargerV *= 0.1;
				_ChargerV += 7.0;

				if (file_type == csvFile) {
					float _Temperature_f = buff_data2[i].Temperature_f;

					if (temperature_unit != TemperatureUnit_C) { // TemperatureUnit_F
						_Temperature_f *= 9.0; // (°C × 9/5) + 32
						_Temperature_f /= 5.0; // (°C × 9/5) + 32
						_Temperature_f += 32.0; // (°C × 9/5) + 32
					}

					sprintf(wtext + strlen(wtext),
							"%s%c%s%c%.4f%c%s%c%d%c%d%c%f%c%f%c",
							buff_data2[i].Date,
							sepChr, buff_data2[i].Time,
							sepChr, _Temperature_f, sepChr,
							buff_data2[i].Sensor_Id, sepChr, _AccX,
							sepChr, _AccY, sepChr, _BattV,
							sepChr, _ChargerV, sepChr);

					if (decimal_separator == sepComma) {
						replace_char(wtext, 46, 44); // ascii(46): "."; ascii(44): ","
					}
				} else {
					if (file_type == datFile16) {
						sprintf(wtext + strlen(wtext),
								"%s%c%s%c%X.%X%c%s%c%X%c%X%c%X%c%X%c",
								buff_data2[i].Date,
								sepChr, buff_data2[i].Time,
								sepChr, temperature_i, temperature_dec_i,
								sepChr, buff_data2[i].Sensor_Id, sepChr,
								buff_data2[i].AccX,
								sepChr, buff_data2[i].AccY, sepChr,
								buff_data2[i].BattV,
								sepChr, buff_data2[i].ChargerV, sepChr);
					} else {
						char temperature_c[4];
						char temperature_dec_c[2];
						char AccX_c[5];
						char AccY_c[5];
						char BattV_c[4];
						char ChargerV_c[4];

						conv_b10_b64(temperature_i, temperature_c);
						conv_b10_b64(temperature_dec_i, temperature_dec_c);
						conv_b10_b64(buff_data2[i].AccX, AccX_c);
						conv_b10_b64(buff_data2[i].AccY, AccY_c);
						conv_b10_b64(buff_data2[i].BattV, BattV_c);
						conv_b10_b64(buff_data2[i].ChargerV, ChargerV_c);

						sprintf(wtext + strlen(wtext),
								"%s%c%s%c%s.%s%c%s%c%s%c%s%c%s%c%s%c",
								buff_data2[i].Date,
								sepChr, buff_data2[i].Time,
								sepChr, temperature_c, temperature_dec_c,
								sepChr, buff_data2[i].Sensor_Id, sepChr, AccX_c,
								sepChr, AccY_c, sepChr, BattV_c,
								sepChr, ChargerV_c, sepChr);
					}
				}
			} else {
				sprintf(wtext + strlen(wtext), "%c%c%c%c%c%c%c%c",
				sepChr, sepChr, sepChr, sepChr, sepChr, sepChr, sepChr, sepChr);
			}
		}
	}

	if (file_type == csvFile) {
		if (date_format == ddmmyy) {
			sprintf(wtext + strlen(wtext), "%c%02d/%02d/%02d%c%02d:%02d%c%s%c",
			sepChr, export_date, export_month, export_year, sepChr,
					export_hours, export_minutes, sepChr, "0",
					sepChr);
		} else { // mmddyy:
			sprintf(wtext + strlen(wtext), "%c%02d/%02d/%02d%c%02d:%02d%c%s%c",
			sepChr, export_month, export_date, export_year, sepChr,
					export_hours, export_minutes, sepChr, "0",
					sepChr);
		}
	} else {
		if (file_type == datFile16) {
			sprintf(wtext + strlen(wtext), "%c%02x%02x%02x%c%02X%02X%c%s%c",
			sepChr, export_year, export_month, export_date, sepChr,
					export_hours, export_minutes, sepChr, "0",
					sepChr);
		} else { // datFile64
			char _export_date[2];
			char _export_month[2];
			char _export_year[3];
			char _export_hours[2];
			char _export_minutes[2];

			conv_b10_b64(export_date, _export_date);
			conv_b10_b64(export_month, _export_month);
			conv_b10_b64(export_year, _export_year);
			conv_b10_b64(export_hours, _export_hours);
			conv_b10_b64(export_minutes, _export_minutes);

			if (_export_year[1] == 0) {
				_export_year[2] = 0;
				_export_year[1] = _export_year[0];
				_export_year[0] = 'A';
			}

			sprintf(wtext + strlen(wtext), "%c%s%s%s%c%s%s%c%s%c", sepChr,
					_export_year, _export_month, _export_date, sepChr,
					_export_hours, _export_minutes, sepChr, "0",
					sepChr);

//			char y_tmp[3];
//			char m_tmp[2];
//			char d_tmp[2];
//
//			conv_b10_b64(buff_test_data.Date[0], d_tmp);
//			conv_b10_b64(buff_test_data.Date[1], m_tmp);
//			conv_b10_b64(buff_test_data.Date[2], y_tmp);
//
//			if (y_tmp[1] == 0) {
//				y_tmp[2] = 0;
//				y_tmp[1] = y_tmp[0];
//				y_tmp[0] = 'A';
//			}
//
//			sprintf(buff_data2[i].Date, "%s%s%s",
//					y_tmp, m_tmp, d_tmp);

		}
	}

	if (file_type == datFile16) {
		BASE64_encode(wtext, wtext);  // hide data
	}
}

////Use custom get_fattime() function
//#define FATFS_CUSTOM_FATTIME    1

////Use custom get_fattime function
////Implement RTC get time here if you need it
//DWORD get_fattime (void) {
//    //Get your time from RTC or something like that
//
//return  ((DWORD)(2014 - 1980) << 25)  // Year 2014
//        | ((DWORD)7 << 21)            // Month 7
//        | ((DWORD)10 << 16)           // Mday 10
//        | ((DWORD)16 << 11)           // Hour 16
//        | ((DWORD)0 << 5)             // Min 0
//        | ((DWORD)0 >> 1);            // Sec 0
//}

FRESULT set_timestamp(char *obj) { /* Pointer to the file name */
	FILINFO fno;

	fno.fdate =
			(WORD) (((tm1.year + 1940) * 512U) | tm1.month * 32U | tm1.date);
	fno.ftime =
			(WORD) (tm1.hours * 2048U | tm1.minutes * 32U | tm1.seconds / 2U);

	return f_utime(obj, &fno);
}

uint8_t scanning_sensor_ID = 0;

uint8_t userFunction(void) {
	uint8_t progress_tmp;
	if (userFunction_phase == 1) {
		if (abort_operation == 1) {
			UsrLog("Operation aborted by user \n");
			export_error = 0;

			export_in_progress = 0;
			userFunction_phase = 0;
			export_data_enabled = 0;
			progress_usb_export = 0;
			start_MSC_Application1 = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;
		}

		UsrLog("userFunction_phase == 1");

		highest_cnt = 0;

		export_date = tm1.date;
		export_month = tm1.month;
		export_year = tm1.year;
		export_hours = tm1.hours;
		export_minutes = tm1.minutes;

		for (int8_t i = 0; i < maxChannel; i++) {
			ch_mark_cnt_export[i] = ch_mark_cnt[i];
		}

		for (int8_t i = 0; i < maxChannel; i++) {
			UsrLog("ch_mark_cnt_export[%d]: %lu", i, ch_mark_cnt_export[i]);

			if ((highest_cnt < ch_mark_cnt_export[i]) && (export_ch[i] == 1)) {
				highest_cnt = ch_mark_cnt_export[i] - 1;
			}
		}

		UsrLog("highest_cnt: %lu", highest_cnt);

		if ((highest_cnt > 0) && (Appli_state_HS == APPLICATION_READY)) {
			export_in_progress = 1;

			/* Register the file system object to the FatFs module */
			if (f_mount(&USBDISKFatHs, (TCHAR const*) USBDISKPath, 0)
					!= FR_OK) {

//				/* FatFs Initialization Error */

				UsrLog("Cannot mount USBDISK \n");
				export_error = 1;
				lcd_refresh = 3;

				userFunction_phase = 0;

				export_data_enabled = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			}

			char file_ext[4] = "";

			if (file_type == csvFile) {
				sprintf(file_ext, "csv");
			} else if (file_type == datFile64) {
				sprintf(file_ext, "dat");
			} else if (file_type == datFile16) {
				sprintf(file_ext, "Dat");
			}

			sprintf(name, "Datalogger2039_20%02d-%02d-%02d_%02d-%02d-%02d.%s",
					tm1.year, tm1.month, tm1.date, tm1.hours, tm1.minutes,
					tm1.seconds, file_ext);

			/*Create a file*/
			if (f_open(&MyFile, name,
			FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
				/* Creation failed */
				UsrLog("Cannot open %s file \n", name);
				export_error = 2;
				lcd_refresh = 3;

				userFunction_phase = 0;

				export_data_enabled = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			} else {
				UsrLog("file %s created \n", name);
				/*write message to the file. Use variable wtext, bytesWritten*/

				userFunction_phase++;
			}
		} else {
			userFunction_phase = 0;

			lcd_refresh = 3;
		}
	} else if (userFunction_phase == 2) {
		UsrLog("userFunction_phase == 2");

		if (abort_operation == 1) {
			UsrLog("Operation aborted by user \n");

			/*close the file*/
			if (f_close(&MyFile) != FR_OK) {
				UsrLog("Cannot close %s file \n", name);
				export_error = 3;
				lcd_refresh = 3;

				export_in_progress = 0;
				userFunction_phase = 0;
				export_data_enabled = 0;
				progress_usb_export = 0;
				start_MSC_Application1 = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			}

			set_timestamp(name);

			export_error = 0;

			export_in_progress = 0;
			userFunction_phase = 0;
			export_data_enabled = 0;
			progress_usb_export = 0;
			start_MSC_Application1 = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;
		}

		get_data_header(); // --> wtext

		sprintf(wtext + strlen(wtext), "\n"); // wtext + \n

		if (f_write(&MyFile, wtext, sizeof_array(wtext), (void*) &bytesWritten)
				!= FR_OK) {
			UsrLog("Cannot write %s file \n", name);
			export_error = 4;
			lcd_refresh = 3;

			userFunction_phase = 0;

			export_data_enabled = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;
		}

		get_data_header2(); // -- wtext

		sprintf(wtext + strlen(wtext), "\n"); // wtext + \n

		if (f_write(&MyFile, wtext, sizeof_array(wtext), (void*) &bytesWritten)
				!= FR_OK) {
			UsrLog("Cannot write %s file \n", name);
			export_error = 5;
			lcd_refresh = 3;

			userFunction_phase = 0;

			export_data_enabled = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;
		}

		userFunction_phase++;

		userFunction_index1 = 0;

		shared_data_clear();

		if (file_type == csvFile) {
			if (file_cnt_csv > 1) {
				file_cnt_csv--;
			} else {
				file_cnt_csv = 0;

				file_type = datFile64;
			}
		} else {
			if (file_cnt_dat < 999) {
				file_cnt_dat++;
			} else {
				file_cnt_dat = 0;
			}
		}

		ConfigStruct_TypeDef conf_dat;

		W25Q64_read_config(&conf_dat);

		conf_dat.FileType = file_type;

		conf_dat.FileCntDat = file_cnt_dat;

		conf_dat.FileCntCsv = file_cnt_csv;

		W25Q64_update_config(&conf_dat);

	} else if (userFunction_phase == 3) {
		UsrLog("userFunction_phase == 3");
		while ((HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port,
		RTC_INT_STATE_Pin) == GPIO_PIN_RESET) && (userFunction_phase == 3)) {

			if (abort_operation == 1) {
				UsrLog("Operation aborted by user \n");

				/*close the file*/
				if (f_close(&MyFile) != FR_OK) {
					UsrLog("Cannot close %s file \n", name);
					export_error = 6;
					lcd_refresh = 3;

					export_in_progress = 0;
					userFunction_phase = 0;
					export_data_enabled = 0;
					progress_usb_export = 0;
					start_MSC_Application1 = 0;

					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_SET); // USB off

					return TRUE;
				}

				set_timestamp(name);

				export_error = 0;

				export_in_progress = 0;
				userFunction_phase = 0;
				export_data_enabled = 0;
				progress_usb_export = 0;
				start_MSC_Application1 = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			}

			if ((userFunction_index1 <= (highest_cnt - 1))
					&& (highest_cnt > 0)) {
				if (scanning_sensor_ID != 1) {
					progress_tmp = ((userFunction_index1 * 8) / highest_cnt)
							+ 1;
				} else {
					progress_tmp = ((shared_data_posit * 8) / highest_cnt) + 1;
				}

				if (progress_tmp > 7) {
					progress_usb_export = 7;
				} else {
					progress_usb_export = progress_tmp;
				}

				if ((progress_usb_export_old != progress_usb_export)
						|| ((HAL_GetTick() - progress_tick) >= 3000)) {
					progress_tick = HAL_GetTick();
					if (scanning_sensor_ID != 1) {
						if (highest_cnt >= userFunction_index1) {
							progress_remaining = highest_cnt
									- userFunction_index1;
						} else {
							progress_remaining = 0;
						}
					} else {
						if (highest_cnt >= shared_data_posit) {
							progress_remaining = highest_cnt
									- shared_data_posit;
						} else {
							progress_remaining = 0;
						}
					}
					progress_usb_export_old = progress_usb_export;
					lcd_progress_bar();
				}

				//lcd_progress_barX10();

//				if (userFunction_index1 == 0) {
//					if (shared_data(highest_cnt) != HAL_OK) {
//						scanning_sensor_ID = 1;
//						return FALSE;
//					} else {
//						scanning_sensor_ID = 0;
//					}
//				}

				// UsrLog("get_data");

				get_data(userFunction_index1);

				userFunction_index1++;

				sprintf(wtext + strlen(wtext), "\n");

				// UsrLog("wtext: %s", wtext);

				if (f_write(&MyFile, wtext, sizeof_array(wtext),
						(void*) &bytesWritten) != FR_OK) {
					UsrLog("Cannot write %s file \n", name);
					export_error = 7;
					lcd_refresh = 3;

					userFunction_phase = 0;

					export_data_enabled = 0;

					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_SET); // USB off

					return TRUE;
				}

				if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
				AC_DC_STATE_Pin)) {
					lcd_timeout0 = lcd_timeout0recall2;
				} else {
					lcd_timeout0 = lcd_timeout0recall;
				}

				if ((HAL_GetTick() - userFunction_tick) >= 5) {
					userFunction_tick = HAL_GetTick();

					IWDG_delay_ms(0);

//					MX_USB_HOST_Process_FS();

					drawInfo();
				}
			} else {
				/*close the file*/
				if (f_close(&MyFile) != FR_OK) {
					UsrLog("Cannot close %s file \n", name);
					export_error = 8;
					lcd_refresh = 3;

					userFunction_phase = 0;

					export_data_enabled = 0;

					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_SET); // USB off

					return TRUE;
				}

				set_timestamp(name);

				userFunction_phase++;

				shared_data_clear();
			}
		}
	} else if (userFunction_phase == 4) {
		UsrLog("userFunction_phase == 4");

		if (abort_operation == 1) {
			UsrLog("Operation aborted by user \n");

			export_error = 0;

			export_in_progress = 0;
			userFunction_phase = 0;
			export_data_enabled = 0;
			progress_usb_export = 0;
			start_MSC_Application1 = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;
		}

		/*check number of written bytes*/
		if ((bytesWritten == 0) || (res != FR_OK)) {
			/*error during writing*/
			UsrLog("write error \n");

			export_error = 9;
			lcd_refresh = 3;

			userFunction_phase = 0;

			export_data_enabled = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;

		} else {
			/*open file to verification*/
			if (f_open(&MyFile, name, FA_READ) != FR_OK) {
				/*file open failure*/
				UsrLog("Cannot open %s file for verify \n", name);
				export_error = 10;
				lcd_refresh = 3;
				userFunction_phase = 0;

				export_data_enabled = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			} else {
				userFunction_phase++;

				line_count = 0;

				userFunction_index1 = 0;
			}

			/*end program execution after verification*/
		}
	} else if (userFunction_phase == 5) {
		UsrLog("userFunction_phase == 5");

		while ((HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port,
		RTC_INT_STATE_Pin) == GPIO_PIN_RESET) && (userFunction_phase == 5)) {
			IWDG_delay_ms(0);

//			MX_USB_HOST_Process_FS();

			if (abort_operation == 1) {
				UsrLog("Operation aborted by user \n");

				/*close the file*/
				if (f_close(&MyFile) != FR_OK) {
					UsrLog("Cannot close %s file \n", name);
					export_error = 11;
					lcd_refresh = 3;

					export_in_progress = 0;
					userFunction_phase = 0;
					export_data_enabled = 0;
					progress_usb_export = 0;
					start_MSC_Application1 = 0;

					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_SET); // USB off

					return TRUE;
				}

				//set_timestamp(name);

				export_error = 0;

				export_in_progress = 0;
				userFunction_phase = 0;
				export_data_enabled = 0;
				progress_usb_export = 0;
				start_MSC_Application1 = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			}

			//UsrLog("index: %lu", userFunction_index1);

			if ((userFunction_index1 <= (highest_cnt - 1))
					&& (highest_cnt > 0)) {

				//UsrLog("line_count: %lu", line_count);

				// UsrLog("check point 1");

				/*Read file content. Use variable : rtext, bytesread*/

				// f_gets: http://elm-chan.org/fsw/ff/doc/gets.html
				if (!f_gets(rtext, sizeof(rtext), &MyFile)) {
					UsrLog("null pointer");
				}

				if ((f_eof(&MyFile) != 0) || (f_error(&MyFile) != 0)) {
					userFunction_phase++;

					// line_count = 0;

					FRESULT ret = f_error(&MyFile);

					if (ret != 0) {
						UsrLog("Error f_gets: %d", ret);
					}
				} else {
					if (line_count == 0) {
						get_data_header(); // line 0
					} else if (line_count == 1) {
						get_data_header2(); // line 1
					} else {
						get_data(userFunction_index1); // line (n - 1)

						userFunction_index1++;
					}

					//UsrLog("sizeof(wtext): %d", sizeof(wtext));

					for (uint16_t i = 0; i < sizeof(wtext); i++) {
						if (wtext[i] != 0) {
							if (wtext[i] != rtext[i]) {
								userFunction_phase++;

								UsrLog(
										"Error: Index: %lu; write(%02X) != read(%02X); pos: %d",
										userFunction_index1, wtext[i], rtext[i],
										i);
								UsrLog("wtext:%s", wtext);
								UsrLog("rtext:%s", rtext);

								IWDG_delay_ms(0);

								export_error = 9;
								lcd_refresh = 3;

								//break; // Error found: exit from for loop
							}
						} else {
							//UsrLog("break");
							break;// All OK: exit from for loop
						}
					}
				}

				line_count++;

				progress_tmp = ((line_count * 2) / highest_cnt) + 7;

				if (progress_tmp < 7) {
					progress_usb_export = 7;
				} else {
					progress_usb_export = progress_tmp;
				}

				if ((progress_usb_export_old != progress_usb_export)
						|| ((HAL_GetTick() - progress_tick) >= 3000)) {
					if (highest_cnt >= line_count) {
						progress_remaining = highest_cnt - line_count;
					} else {
						progress_remaining = 0;
					}

					progress_tick = HAL_GetTick();
					progress_usb_export_old = progress_usb_export;
					lcd_progress_bar();
				}

				//lcd_progress_barX10();

				//		IWDG_delay_ms(0);
				//
				//		MX_USB_HOST_Process_FS();

				if ((HAL_GetTick() - userFunction_tick) >= 5) {
					userFunction_tick = HAL_GetTick();

					IWDG_delay_ms(0);

//					MX_USB_HOST_Process_FS();

					drawInfo();
				}
			}
		}
	} else if (userFunction_phase == 6) {
		UsrLog("userFunction_phase == 6");

		if (abort_operation == 1) {
			UsrLog("Operation aborted by user \n");

			/*close the file*/
			if (f_close(&MyFile) != FR_OK) {
				UsrLog("Cannot close %s file \n", name);
				export_error = 12;
				lcd_refresh = 3;

				export_in_progress = 0;
				userFunction_phase = 0;
				export_data_enabled = 0;
				progress_usb_export = 0;
				start_MSC_Application1 = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			}

			set_timestamp(name);

			export_error = 0;

			export_in_progress = 0;
			userFunction_phase = 0;
			export_data_enabled = 0;
			progress_usb_export = 0;
			start_MSC_Application1 = 0;

			return TRUE;
		}

		if (line_count == 0) {
			/*read fail*/
			UsrLog("Cannot read file for verification \n");

			export_error = 13;
			lcd_refresh = 3;
			userFunction_phase = 0;

			export_data_enabled = 0;

			HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
					GPIO_PIN_SET); // USB off

			return TRUE;
		} else {
			/*read success*/

			/*close the file*/
			if (f_close(&MyFile) != FR_OK) {
				/*check number of written bytes*/
				UsrLog("fclose fail \n");

				export_error = 14;
				lcd_refresh = 3;
				userFunction_phase = 0;

				export_data_enabled = 0;

				HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
						GPIO_PIN_SET); // USB off

				return TRUE;
			} else {
				/* Compare read data with the expected data */
				if (line_count == (highest_cnt + 2)) {
					/*verification success full - number of written bytes is equal to number of read bytes*/
					UsrLog(
							"verification OK - read content matches written content");

//					int d = 1000;

//					while (d--) {
					IWDG_delay_ms(0);
//						MX_USB_HOST_Process_FS();
//					}

					progress_usb_export = 10;

					if (progress_usb_export_old != progress_usb_export) {
						progress_usb_export_old = progress_usb_export;
						lcd_progress_bar();
					}

					userFunction_phase++;

				} else {
					/*verification failed - number of written bytes is not equal to number of read bytes*/
					UsrLog("verify fail; lines read: %lu, lines wrote: %lu",
							line_count, (highest_cnt + 2));

					export_error = 15;
					lcd_refresh = 3;
					userFunction_phase = 0;

					export_data_enabled = 0;

					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_SET); // USB off

					return TRUE;
				}
			}
		}
	} else if (userFunction_phase == 7) {
		UsrLog("userFunction_phase == 7");
// End MSC_HOST_HANDS_ON: Add the call to userFunction()

		HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin, GPIO_PIN_SET); // USB off

		export_in_progress = 0;

		progress_usb_export = 99;

		lcd_progress_bar();

		for (int i = 0; i < 8; i++) { // maxChannel
			if (export_ch[i] == 1) {
				ch_exported[i] = 1; // set as exported
			}
		}

		update_exported_data();

		return TRUE;
	}

	return FALSE;
}

void update_exported_data(void) {
	ConfigStruct_TypeDef conf_dat;

	W25Q64_read_config(&conf_dat);

	for (int i = 0; i < 8; i++) { // maxChannel
		conf_dat.ChExported[i] = ch_exported[i];
	}

	W25Q64_update_config(&conf_dat);
}

void USB_Force_Recovery(void) {
	USBH_RecoveryPhost(&hUsbHostFS);
}

void MX_USB_HOST_Process_HS(void) {
	/* USB Host Background task */

	if (USBH_isInitialized == 1) {
		USBH_Process(&hUsbHostHS);

//		kbd_num_lock_state = keybd_info1->led_num_lock;
//		kbd_caps_lock_state = keybd_info1->led_caps_lock;
//		kbd_scroll_lock_state = keybd_info1->led_scroll_lock;
//	} else {
//		USBH_UsrLog("USBH HS not Initialized");
	}
}

uint32_t Process_FS_tick_old = 0;

uint32_t Process_FS_timer = 0;
uint32_t led_update_timer = 0;

void MX_USB_HOST_Process_FS(void) {
	if (USBH_isInitialized == 1) {
		/* USB Host Background task */
		if ((HAL_GetTick() - Process_FS_timer) >= 7) {
			Process_FS_timer = HAL_GetTick();

			USBH_Process(&hUsbHostFS);

			uint32_t Process_FS_tick_new = HAL_GetTick() - Process_FS_tick_old;
			Process_FS_tick_old = HAL_GetTick();

			if (Process_FS_tick_new > 50) {
				USBH_UsrLog("%lu - Process_FS", Process_FS_tick_new);
			}
		}

//	} else {
//		USBH_UsrLog("USBH FS not Initialized");

		if ((HAL_GetTick() - led_update_timer) >= 100) {
			led_update_timer = HAL_GetTick();

			keybd_info_lcd.led_num_lock = keybd_info1->led_num_lock;
			keybd_info_lcd.led_caps_lock = keybd_info1->led_caps_lock;
			keybd_info_lcd.led_scroll_lock = keybd_info1->led_scroll_lock;
		}
	}
}

/* ----------------------- */

// Mini Wireless Keyboard with built-in Touchpad decode:
static HID_MWKBT_TypeDef* USBH_HID_Decode_Mini_Keyboard_Touchpad(
		HID_MOUSE_Info_TypeDef *Minfo) {
	MWKBT_decoded.multimedia = MWKBT_EMPTY;
	MWKBT_decoded.valid = MWKBT_EMPTY;

	MWKBT_decoded.buttons[0] = 0;
	MWKBT_decoded.buttons[1] = 0;
	MWKBT_decoded.buttons[2] = 0;

//	USBH_UsrLog("raw_data32[0]: 0x%08lx", Minfo->raw_data32[0]);
//	USBH_UsrLog("raw_data32[1]: 0x%08lx", Minfo->raw_data32[1]);
//	USBH_UsrLog("raw_length: 0x%02x", Minfo->raw_length);

	uint32_t raw_data32_1 = Minfo->raw_data32[1] & 0xFFFFFF00;

//	USBH_UsrLog("raw_data32_1: 0x%08lX", raw_data32_1);

	if ((Minfo->raw_length == 8) && (raw_data32_1 == 0x00080000)) {
		if ((((Minfo->raw_data32[0] >> 1) & 1) == 0)
				&& ((Minfo->raw_data32[0] & 1) == 1)) {

			uint8_t btn0 = (Minfo->raw_data32[0] >> 8) & 1;
			uint8_t btn1 = (Minfo->raw_data32[0] >> 9) & 1;

			MWKBT_decoded.buttons[0] = btn0;
			MWKBT_decoded.buttons[1] = btn1;

			if ((MWKBT_decoded.buttons[0] == 1)
					&& (MWKBT_decoded.buttons[1] == 1)) {
				MWKBT_decoded.buttons[0] = 0;
				MWKBT_decoded.buttons[1] = 0;
				MWKBT_decoded.buttons[2] = 1;
			}

			uint8_t x_val = (Minfo->raw_data32[0] >> 16) & 0xFF;
			uint8_t y_val = (Minfo->raw_data32[0] >> 24) & 0xFF;

			if ((x_val > 0) || (y_val > 0)) {
				MWKBT_decoded.x = x_val;
				MWKBT_decoded.y = y_val;
			}

			if ((btn0 != 0) || (btn1 != 0) || (x_val > 0) || (y_val > 0)) {
				MWKBT_decoded.valid = MWKBT_VALID;
			}
		} else if ((((Minfo->raw_data32[0] >> 1) & 1) == 1)
				&& ((Minfo->raw_data32[0] & 1) == 0)) {

			//			printf("Multimedia functions\n");

			if (sizeof(MWKBT_array_cod) != sizeof(MWKBT_array_ref)) {
				USBH_UsrLog("Error: the sizes of the arrays do not match");

				return &MWKBT_decoded;
			}

			MWKBT_decoded.multimedia_size = sizeof(MWKBT_array_cod);

			uint8_t new_cod = (Minfo->raw_data32[0] >> 8) & 0xFF;

			//USBH_UsrLog("new_cod: 0x%02X", new_cod);

			USBH_UsrLog("multimedia_size: %d", MWKBT_decoded.multimedia_size);

			if (new_cod != 0) {
				for (uint8_t i = 0; i < MWKBT_decoded.multimedia_size; i++) {
					if (new_cod == MWKBT_array_cod[i]) {
						MWKBT_decoded.multimedia = MWKBT_array_ref[i];

						MWKBT_decoded.valid = MKLITEON_VALID;

						USBH_UsrLog("found");

						break;
					}
				}
			} else {
				MWKBT_decoded.multimedia = MKLITEON_EMPTY;
				MWKBT_decoded.valid = MKLITEON_EMPTY;
			}
		}

		MWKBT_decoded.scroll = Minfo->raw_data32[1] & 0xFF; // 8 bits

		if ((MWKBT_decoded.scroll & 1) == 1) {
			MWKBT_decoded.valid = MWKBT_VALID;
		}
	}

	return &MWKBT_decoded;
}

static void USBH_HID_MouseLogiDecode(HID_MOUSE_Info_TypeDef *Minfo) {
	if (Minfo->raw_length == 0x08) { // length_array: 8
		mouse_info_local->valid = MOUSE_EMPTY;

		/*Decode report */
		uint8_t x_val = (mouse_info_local->raw_data32[0] >> 24) & 0xFF;
		x_val |= (mouse_info_local->raw_data32[1] & 0xF) << 8;

		uint8_t y_val = (mouse_info_local->raw_data32[1] >> 4) & 0xFFF;

		if ((x_val > 0) || (y_val > 0)) {
			mouse_info_local->x = x_val;
			mouse_info_local->y = y_val;
		}

		uint8_t scroll_val = 0;

		if (((mouse_info_local->raw_data32[1] >> 16) & 1) == 1) {
			scroll_val = (mouse_info_local->raw_data32[1] >> 16) & 0xFF; // 8 bits
		} else {
			scroll_val = 0;
		}

		mouse_info_local->scroll = scroll_val;

		uint8_t btn0 = (mouse_info_local->raw_data32[0] >> 8) & 0b1;
		uint8_t btn1 = (mouse_info_local->raw_data32[0] >> 9) & 0b1;
		uint8_t btn2 = (mouse_info_local->raw_data32[0] >> 10) & 0b1;

		mouse_info_local->buttons[0] = btn0;
		mouse_info_local->buttons[1] = btn1;
		mouse_info_local->buttons[2] = btn2;

		if ((btn0 != 0) || (btn1 != 0) || (btn2 != 0) || (x_val > 0)
				|| (y_val > 0) || ((scroll_val & 1) == 1)) {
			mouse_info_local->valid = MOUSE_VALID;
		}
	}
}

static void USBH_HID_MultmediaKbDecode(HID_CUSTOM_Info_TypeDef *Cinfo) {
	uint8_t raw_data32 = custom_info_local->raw_data32[0] & 0xFFFF00FF;

	if ((raw_data32 == 1) && (Cinfo->raw_length == 0x03)) { // lengtkeybd_info->3
		if (sizeof(MKLITEON_array_cod) != sizeof(MKLITEON_array_cod)) {
			USBH_UsrLog("Error: the sizes of the arrays do not match");

			return;
		}

		MKLITEON_decoded.multimedia_size = sizeof(MKLITEON_array_cod);

		/*Decode report */
		uint8_t new_cod = (custom_info_local->raw_data32[0] >> 8) & 0xFF;

		// USBH_UsrLog("new_cod: 0x%02X", new_cod);

		if (new_cod != 0) {
			for (uint8_t i = 0; i < MKLITEON_decoded.multimedia_size; i++) {
				if (new_cod == MKLITEON_array_cod[i]) {
					MKLITEON_decoded.multimedia = MKLITEON_array_ref[i];

					MKLITEON_decoded.valid = MKLITEON_VALID;

					break;
				}
			}
		} else {
			MKLITEON_decoded.multimedia = MKLITEON_EMPTY;
			MKLITEON_decoded.valid = MKLITEON_EMPTY;
		}
	}
}

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost) {
	uint8_t idx = phost->device.current_interface;

	HID_HandleTypeDef *HID_Handle =
			(HID_HandleTypeDef*) phost->pActiveClass->pData_array[idx];
	if (HID_Handle->Init == USBH_HID_KeybdInit) {
		keybd_info1 = USBH_HID_GetKeybdInfo(phost);

		// ABNT2 KeyMap correction
		if (language_current == lang_ptBr) {
			keybd_info1->key_ascii = get_ABNT2(keybd_info1);

			if (keybd_info1->keys[0] != 0) {
				USBH_UsrLog("key_ascii: [%c], (0x%02X)", keybd_info1->key_ascii,
						keybd_info1->key_ascii);
			}
		}

		//USBH_UsrLog("USBH_HID_KeybdInit");

#if (KBD_LOG == 1U)
		if (lcd_screen != LcdScreenPassword) {
			UsrLog("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
					keybd_info1->keys[0], keybd_info1->keys[1],
					keybd_info1->keys[2], keybd_info1->keys[3],
					keybd_info1->keys[4], keybd_info1->keys[5]); // USBH_DbgLog

			UsrLog(
					"lalt: %d, ralt: %d, lctrl: %d, rctrl: %d, lgui: %d, rgui: %d, lshift: %d, rshift: %d, state: %d",
					keybd_info1->lalt, keybd_info1->ralt, keybd_info1->lctrl,
					keybd_info1->rctrl, keybd_info1->lgui, keybd_info1->rgui,
					keybd_info1->lshift, keybd_info1->rshift,
					keybd_info1->state);
		}
#endif
		if ((keybd_info1->keys[0] != KEY_KEYPAD_NUM_LOCK_AND_CLEAR)
				&& (keybd_info1->keys[0] != KEY_CAPS_LOCK)
				&& (keybd_info1->keys[0] != KEY_SCROLL_LOCK)) {
			Datalogger2039_HID_handler(keybd_info1);
		}

	} else if (HID_Handle->Init == USBH_HID_MouseInit) {
		if ((HAL_GetTick() - mouse_interval) < mouse_interval_ms) {
			return; // to avoid excessive mouse handling
		} else {
			mouse_interval = HAL_GetTick();
		}

		mouse_info_local = USBH_HID_GetMouseInfo(phost);

		uint16_t VID_info = phost->device.DevDesc.idVendor;
		uint16_t PID_info = phost->device.DevDesc.idProduct;

		USBH_UsrLog("VID: 0x%04X, PID: 0x%04X", VID_info, PID_info);

		// Mini Wireless Keyboard
		const uint16_t VID_MWKBT = 0x0513;
		const uint16_t PID_MWKBT = 0x0318;

		// Wireless Logitech M220
		const uint16_t VID_WLogiM220 = 0x046D;
		const uint16_t PID_WLogiM220 = 0xC534;

		if ((VID_info == VID_MWKBT) && (PID_info == PID_MWKBT)) {

			HID_MWKBT_TypeDef *mini_kb_touchpad;

			mini_kb_touchpad = USBH_HID_Decode_Mini_Keyboard_Touchpad(
					mouse_info_local);

			if (mini_kb_touchpad->multimedia != MWKBT_EMPTY) {
				if (MWKBT_decoded.valid == MWKBT_VALID) {
					USBH_UsrLog("Mini Wireless Keyboard Multimidea");

					for (uint8_t i = 0; i < MWKBT_decoded.multimedia_size;
							i++) {
						if (MWKBT_decoded.multimedia == MWKBT_array_ref[i]) {
							USBH_UsrLog("%s", MWKBT_array_name[i]);
						}
					}
				}
			} else {
				if (MWKBT_decoded.valid == MWKBT_VALID) {
					USBH_UsrLog(
							"Mini KB Touchpad action: x=  0x%02X, y=  0x%02X, scroll=  0x%02X, button1= 0x%x, button2= 0x%x, button3= 0x%x",
							mini_kb_touchpad->x, mini_kb_touchpad->y,
							mini_kb_touchpad->scroll,
							mini_kb_touchpad->buttons[0],
							mini_kb_touchpad->buttons[1],
							mini_kb_touchpad->buttons[2]);

//					keybd_info1->keys[0] =
//
//					keyboardMain(keybd_info1);
				}
			}
		} else if ((VID_info == VID_WLogiM220) && (PID_info == PID_WLogiM220)) {
			USBH_UsrLog("(Wireless) Logitech Mouse");

			USBH_HID_MouseLogiDecode(mouse_info_local);

			if (mouse_info_local->valid == MOUSE_VALID) {
				USBH_UsrLog(
						"Logitech Mouse action: x=  0x%03X, y=  0x%03X, scroll=  0x%02X, button1= 0x%x, button2= 0x%x, button3= 0x%x",
						mouse_info_local->x, mouse_info_local->y,
						mouse_info_local->scroll, mouse_info_local->buttons[0],
						mouse_info_local->buttons[1],
						mouse_info_local->buttons[2]);
			}
		} else { // regular mouse (2 buttons + scroll wheel)
			//printf("Mouse action (raw data): ");

			//print_raw_info(mouse_info);
			if (mouse_info_local->valid == MOUSE_VALID) {
				USBH_UsrLog(
						"Mouse action: x=  0x%02X, y=  0x%02X, scroll=  0x%02X, button1= 0x%x, button2= 0x%x, button3= 0x%x",
						mouse_info_local->x, mouse_info_local->y,
						mouse_info_local->scroll, mouse_info_local->buttons[0],
						mouse_info_local->buttons[1],
						mouse_info_local->buttons[2]);
			}
		}
	} else if (HID_Handle->Init == USBH_HID_CustomInit) {
		custom_info_local = USBH_HID_GetCustomInfo(phost);

		uint16_t VID_info = phost->device.DevDesc.idVendor;
		uint16_t PID_info = phost->device.DevDesc.idProduct;

		// USBH_UsrLog("VID: 0x%04X, PID: 0x%04X", VID_info, PID_info);

		// Multimedia Keyboard Lite-On
		const uint16_t VID_MKLiteOn = 0x04CA;
		const uint16_t PID_MKLiteOn = 0x005A;

		if ((VID_info == VID_MKLiteOn) && (PID_info == PID_MKLiteOn)) {
			USBH_HID_MultmediaKbDecode(custom_info_local);

			if (MKLITEON_decoded.valid == MKLITEON_VALID) {
				USBH_UsrLog("Multimedia Keyboard Lite-On");

				for (uint8_t i = 0; i < MKLITEON_decoded.multimedia_size; i++) {
					if (MKLITEON_decoded.multimedia == MKLITEON_array_ref[i]) {
						USBH_UsrLog("%s", MKLITEON_array_name[i]);
					}
				}
			}
		}
	}
}

static void Datalogger2039_HID_handler(HID_KEYBD_Info_TypeDef *keybd_info1) {
	// ESC x20: RESTART
	if (keybd_info1->keys[0] == KEY_ESCAPE) {
		if (esc_cnt < 20) {
			esc_cnt++;
		} else {
			if (ctrl_atl_del_restart == 0) {
				ctrl_atl_del_restart = 1;

				if ((tm1.seconds > 5) && (tm1.seconds < 55)) {
					ctrl_atl_del_timeout = 0;
				} else {
					ctrl_atl_del_timeout = 10;
				}
			}
		}
	}

	// CTRL + ALT + DEL: RESTART
	if ((keybd_info1->keys[0] == KEY_DELETE)
			&& ((keybd_info1->lalt == 1) || (keybd_info1->ralt == 1))
			&& ((keybd_info1->lctrl == 1) || (keybd_info1->rctrl == 1))) {

		if (ctrl_atl_del_cnt < 2) {
			ctrl_atl_del_cnt++;

			beep(ctrl_atl_del_cnt, 0);
		} else {
			if (ctrl_atl_del_restart == 0) {
				ctrl_atl_del_restart = 1;

				if ((tm1.seconds > 5) && (tm1.seconds < 55)) {
					ctrl_atl_del_timeout = 0;
				} else {
					ctrl_atl_del_timeout = 10;
				}
			}
		}
	}

	if ((HAL_GetTick() - keyboardMain_debounce) >= 75) {
		keyboardMain_debounce = HAL_GetTick();

		//uint8_t key = keybd_info1->keys[0];

		keyboardMain(keybd_info1);
	}
}

void USBH_HID_EventCallback1(USBH_HandleTypeDef *phost) {
	HID_HandleTypeDef *HID_Handle =
			(HID_HandleTypeDef*) phost->pActiveClass->pData;
	if (HID_Handle->Init == USBH_HID_KeybdInit) {
		keybd_info1 = USBH_HID_GetKeybdInfo(phost);

#if (KBD_LOG == 1U)
		if (lcd_screen != LcdScreenPassword) {
			UsrLog("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
					keybd_info1->keys[0], keybd_info1->keys[1],
					keybd_info1->keys[2], keybd_info1->keys[3],
					keybd_info1->keys[4], keybd_info1->keys[5]); // USBH_DbgLog

			UsrLog(
					"lalt: %d, ralt: %d, lctrl: %d, rctrl: %d, lgui: %d, rgui: %d, lshift: %d, rshift: %d, state: %d",
					keybd_info1->lalt, keybd_info1->ralt, keybd_info1->lctrl,
					keybd_info1->rctrl, keybd_info1->lgui, keybd_info1->rgui,
					keybd_info1->lshift, keybd_info1->rshift,
					keybd_info1->state);
		}
#endif
		// ESC x20: RESTART
		if (keybd_info1->keys[0] == KEY_ESCAPE) {
			if (esc_cnt < 20) {
				esc_cnt++;
			} else {
				if (ctrl_atl_del_restart == 0) {
					ctrl_atl_del_restart = 1;

					if ((tm1.seconds > 5) && (tm1.seconds < 55)) {
						ctrl_atl_del_timeout = 0;
					} else {
						ctrl_atl_del_timeout = 10;
					}
				}
			}
		}

		// CTRL + ALT + DEL: RESTART
		if ((keybd_info1->keys[0] == KEY_DELETE)
				&& ((keybd_info1->lalt == 1) || (keybd_info1->ralt == 1))
				&& ((keybd_info1->lctrl == 1) || (keybd_info1->rctrl == 1))) {

			if (ctrl_atl_del_cnt < 2) {
				ctrl_atl_del_cnt++;

				beep(ctrl_atl_del_cnt, 0);
			} else {
				if (ctrl_atl_del_restart == 0) {
					ctrl_atl_del_restart = 1;

					if ((tm1.seconds > 5) && (tm1.seconds < 55)) {
						ctrl_atl_del_timeout = 0;
					} else {
						ctrl_atl_del_timeout = 10;
					}
				}
			}
		}
		//	cnt_esc_restart_timeout

		//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

		if ((HAL_GetTick() - keyboardMain_debounce) >= 100) {
			keyboardMain_debounce = HAL_GetTick();

			uint8_t key = keybd_info1->keys[0];

			//	keycode = USBH_HID_GetASCIICode_Custom(keybd_info);

			//kbd_LED_status = key;

			//	UsrLog("key: 0x%02X", key);
//			UsrLog("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
//					keybd_info->keys[0], keybd_info->keys[1],
//					keybd_info->keys[2], keybd_info->keys[3],
//					keybd_info->keys[4], keybd_info->keys[5]);

//			cnt_timeout_reset_USB = 0;

			if (key == KEY_INSERT) {
				keyboard_insert = !keyboard_insert;
			} else if (key == KEY_KEYPAD_NUM_LOCK_AND_CLEAR) {
				if ((kbd_LED_status[0] & 1) == 0) {
					kbd_LED_status[0] |= 0B001;
				} else {
					kbd_LED_status[0] &= 0B110;
				}

				UsrLog("kbd_LED_status %d", kbd_LED_status[0]);

				//		USB_Set_Keyboard_LED_Status();
			} else if (key == KEY_CAPS_LOCK) {
				if (((kbd_LED_status[0] >> 1) & 1) == 0) {
					kbd_LED_status[0] |= 0B010;
				} else {
					kbd_LED_status[0] &= 0B101;
				}

				UsrLog("kbd_LED_status %d", kbd_LED_status[0]);
			} else if (key == KEY_SCROLL_LOCK) {
				if (((kbd_LED_status[0] >> 2) & 1) == 0) {
					kbd_LED_status[0] |= 0B100;
				} else {
					kbd_LED_status[0] &= 0B011;
				}

				UsrLog("kbd_LED_status %d", kbd_LED_status[0]);
			} else {
				keyboardMain(keybd_info1);
			}
		}

//	} else if (HID_Handle->Init == USBH_HID_MouseInit) {
//		USBH_HID_GetMouseInfo(phost);
//
//		USBH_DbgLog(
//				"Mouse action: x=  0x%x, y=  0x%x, button1 = 0x%x, button2 = 0x%x, button3 = 0x%x \n",
//				mouse_info.x, mouse_info.y, mouse_info.buttons[0],
//				mouse_info.buttons[1], mouse_info.buttons[2]);
	}
}
/* USER CODE END 1 */

/**
 * Init USB host library, add supported class and start the library
 * @retval None
 */
void MX_USB_HOST_Init(void) {
	/* USER CODE BEGIN USB_HOST_Init_PreTreatment */
	if (lcd_enabled == false) {
		USBH_UsrLog("Skip: MX_USB_HOST_Init");
		return;
	} else {
		USBH_UsrLog("MX_USB_HOST_Init");
	}
	/* USER CODE END USB_HOST_Init_PreTreatment */

	/* Init host Library, add supported class and start the library. */
	if (USBH_Init(&hUsbHostHS, USBH_UserProcess1, HOST_HS) != USBH_OK) {
		Error_Handler();
	}
	if (USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS) != USBH_OK) {
		Error_Handler();
	}
	if (USBH_Start(&hUsbHostHS) != USBH_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_HOST_Init_PreTreatment */
	if (lcd_enabled == false) {
		USBH_UsrLog("Skip: MX_USB_HOST_Init");
		return;
	} else {
		USBH_UsrLog("MX_USB_HOST_Init");
	}
	/* USER CODE END USB_HOST_Init_PreTreatment */

	/* Init host Library, add supported class and start the library. */
	if (USBH_Init(&hUsbHostFS, USBH_UserProcess2, HOST_FS) != USBH_OK) {
		Error_Handler();
	}
	if (USBH_RegisterClass(&hUsbHostFS, USBH_HID_CLASS) != USBH_OK) {
		Error_Handler();
	}
	if (USBH_Start(&hUsbHostFS) != USBH_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_HOST_Init_PostTreatment */
	USBH_isInitialized = 1;
	/* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void) {
	/* USB Host Background task */
	USBH_Process(&hUsbHostHS);
	USBH_Process(&hUsbHostFS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess1(USBH_HandleTypeDef *phost, uint8_t id) {
	/* USER CODE BEGIN CALL_BACK_2 */
	switch (id) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_DISCONNECTION:
		Appli_state = APPLICATION_DISCONNECT;
		Appli_state_HS = Appli_state;
		if (lcd_screen == LcdScreenUsbExport) {
			lcd_refresh = 3;
		}
		if (export_in_progress == 1) {
			export_in_progress = 0;
			export_error = 16;
			lcd_refresh = 3;
			userFunction_phase = 0;

			export_data_enabled = 0;
			progress_usb_export = 0;
			start_MSC_Application1 = 0;
		}

		if (f_mount(NULL, "", 0) != FR_OK) {
			Error_Handler();
		}
		if (FATFS_UnLinkDriver(USBDISKPath) != 0) {
			Error_Handler();
		}
		break;

	case HOST_USER_CLASS_ACTIVE:
		Appli_state = APPLICATION_READY;
		Appli_state_HS = Appli_state;
		if (lcd_screen == LcdScreenUsbExport) {
			export_in_progress = 0;

			userFunction_phase = 0;

			export_data_enabled = 0;
			progress_usb_export = 0;

			export_error = 0;
			start_MSC_Application1 = 0;
			lcd_refresh = 3;
		}
		break;

	case HOST_USER_CONNECTION:
		Appli_state = APPLICATION_START;
		Appli_state_HS = Appli_state;

		if (FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0) {
			if (f_mount(&USBDISKFatHs, (const TCHAR*) USBDISKPath, 0)
					!= FR_OK) {
				Error_Handler();
			}
		}
		break;

	default:
		break;
	}
	/* USER CODE END CALL_BACK_2 */
}

static void USBH_UserProcess2(USBH_HandleTypeDef *phost, uint8_t id) {
	/* USER CODE BEGIN CALL_BACK_21 */
	switch (id) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_DISCONNECTION:
		Appli_state = APPLICATION_DISCONNECT;

		if (Appli_state_FS != APPLICATION_DISCONNECT) {
			usb_power_recycle_count++;
		}

		Appli_state_FS = Appli_state;

		kbd_LED_status_old = 99;

		keyboard_timeout = 0;

		keyboard_timeout2 = 0;

		break;

	case HOST_USER_CLASS_ACTIVE:
		Appli_state = APPLICATION_READY;
		Appli_state_FS = Appli_state;
		break;

	case HOST_USER_CONNECTION:
		Appli_state = APPLICATION_START;
		Appli_state_FS = Appli_state;
		break;

	default:
		break;
	}
	/* USER CODE END CALL_BACK_21 */
}

/**
 * @}
 */

/**
 * @}
 */

