/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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

/**
 ******************************************************************************
 * Rtek100 2019 - 2023 (Datalogger with STM32F407VG @ 168MHz/40MHz - 8MHz Clock source)
 ******************************************************************************
 *
 * Progress:
 * [ ] Events log
 * [x] Batt monitor - Interrupt
 * [x] Password
 * [x] LCD ILI9341 	([x] Init [x] Draw)
 * [x] ADC          ([x] Read [x] Read batt [x] Read charger)
 * [x] RTC DS3231 	([x] Init [x] Write, [x] Read, [x] Alarm)
 * [x] Flash W25Q64 ([x] Init [x] Read, [x] Write, [x] Erase [x] Data save [x] Data load)
 * [x] Pwr switch/Dimm
 * [x] USB HS MSC	([x] Read, [x] Write)
 * 					([x] Data save CSV)
 * 					([x] Data save DAT (base num 16(Hex)/base 64 encoder))
 * 					([x] Data save dat (base num 64/base 64 encoder))
 * 					([x] Selection decimal separator)
 * 					([x] Selection data type)
 * [x] USB FS HID	([x] Keyboard [-] Setup using keyboard PT-BR ABNT / US-EN)
 *                  ([x] Keyboard NUM LOCK LED)
 * 					([x] USB power switch)
 * [*] NTC temperature conversion (* analog sensor aborted)
 * [x] DS18B20 temperature digital sensor ([x] DS18B20 short circuit protection)
 * [ ] Add On/Off for all CH's (similar to Memory Test)
 * [x] Add Offset for sensors
 * [ ] Add TMP117 support (software I2C)
 * [ ] Split main file into other files
 *
 ******************************************************************************
 *
 *     ###############  Warning  ###############
 *
 *  PLEASE: Do not use "Device Configuration Tool Code Generation" (*.ioc file)
 *     ---> Codes were added in areas NOT protected from modification <---
 *
 ******************************************************************************
 *
 * [-] NEW FLASH ADDR 0x08008000 for bootloader
 * (Bootloader BUG: DS3231 cannot start w/o AC pwr)
 * (Manually calc CRC code)
 * [-] STM32F407VGTX_FLASH.ld
 * [-] FLASH (rx) : ORIGIN = 0x8008000, LENGTH = 1024K - 2*16K
 *
 * [-] #include "stm32f407xx.h"
 * [-] #define FLASH_BASE 0x08008000UL
 *
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "clock.h"

// #include "stm32f407xx.h" /* Ref.: https://community.st.com/s/question/0D53W00001ZYLpTSAX/stm32-usb-training-113-usb-msc-dfu-host-labs-does-the-current-code-not-fit */
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "IWDG_Delay.h"
#include "SCREEN.h"
#include "EEPROM_EXTERN.h"
#include "ILI9341.h"
#include "dwt_stm32_delay.h"
#include "MPU6050.h"
#include "STM32F4_UID.h"
#include "fonts.h"
#include "DS18B20.h"
#include "DS3231.h"
#include "usbh_hid_keybd.h"
#include "w25qxx.h"
#include "SN74HC595.h"
#include "base64.h"
//#include "logo1.h" // see: SCREEN.c
#include "logo2.h"
#include "BMP_to_USB.h"

// Enable ECB, CTR and CBC mode. Note this can be done before including aes.h or at compile-time.
#define CBC 0
#define CTR 1
#define ECB 0
#include "tiny-AES/aes.h"
//#include "logo.h"
//#include "channels_data.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DISABLE_BEEP_FOR_DEBUG 1	// 0: Perform beep for info or errors
#define DISABLE_FLASH_CRC_AT_BOOT 0 // 0: Perform CRC check
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

ETH_TxPacketConfig TxConfig;
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

CRC_HandleTypeDef hcrc;

ETH_HandleTypeDef heth;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t datacsv0_index = 0;
uint32_t datacsv1_index = 0;
uint32_t datacsv2_index = 0;
uint32_t datacsv3_index = 0;
uint32_t datacsv4_index = 0;
uint32_t datacsv5_index = 0;
uint32_t datacsv6_index = 0;
uint32_t datacsv7_index = 0;

char channelLabel[maxChannel][maxChar] = { 0 };

char channelLabel_tmp[maxChar_x2] = { 0 };

char channelLabel_ctrl_c[maxChar_x2] = { 0 };

char channelLabel_selected[maxChar_x2] = { 0 };

int ch_status[maxChannel] = { 0 };

int ch_status_tmp[maxChannel] = { 0 };

int ch_status_tmp_confirm = 0;
int ch_status_tmp_confirm2 = 0;

uint8_t lcd_screen_previews = LcdScreenMain;
uint8_t lcd_screen_next = LcdScreenMain;

int lcd_screen_old = 0;

int lcd_screen = LcdScreenMain; //LcdScreenUsbExport; //LcdScreenMonCh; //LcdScreenCfgCh; // LcdScreenMain; // 1st screen after boot
//typedef enum {
//	LcdScreenGraphic = 0,
//	LcdScreenMenu,
//	LcdScreenCfgCh,
//  LcdScreenCfgChConfirm,
//	LcdScreenCfgChCheck,
//	LcdScreenCfgClk,
//	LcdScreenMonCh,
//  LcdScreenMonChOffset,
//  LcdScreenLangExport,
//  LcdScreenPassword,
//  LcdScreenUsbExport,
//  LcdScreenMain,
//  LcdScreenPasswordConfig,
//  LcdScreenMemTest,
//  LcdScreenLanguage
//}LcdScreenTypeDef;

#ifdef check_lcd_status
uint32_t lcd_status_reference = 0; // 0x29820000 see: ILI9341 RDDST
#endif

int16_t sensor_offset[maxChannel];

uint8_t ch_exported[8]; // maxChannel

uint8_t language_current = lang_en;
uint8_t language_current_tmp = lang_en;
uint8_t language_screen_index = 0;
uint8_t decimal_separator = sepDot;
uint8_t date_format = mmddyy;
uint8_t file_type = datFile64;
uint16_t file_cnt_csv = 0;
uint16_t file_cnt_dat = 0;

uint32_t save_BMP_timeout = 0;
uint8_t save_BMP_enabled = 0;
uint8_t save_BMP_step = 0;
uint8_t save_BMP_phase = 0;

uint32_t export_data_timeout = 0;
uint8_t export_data_enabled = 0;
uint8_t mem_test_enabled = 0;
uint8_t mem_test_step = 0;
uint8_t mem_test_channel = 0;
uint8_t save_data_step = 0;

uint32_t mem_test_channel_progress = 0;
uint8_t mem_test_channel_progress_old = 0;

DATAHEADER_TypeDef data_header;

uint32_t HALGetTick1 = 0;

char memTestStatus[maxChannel] = { 0, 0, 0, 0, 0, 0, 0, 0 };

HAL_StatusTypeDef HAL_StatusMemTest;

uint8_t lcd_show_logo = 0;

uint8_t init_flag = 0;
uint8_t it_flag = 0;

uint8_t menu_list_begin = 0;
uint8_t menu_index_position = 0;
uint8_t menu_index_max = 10;

DadaStruct3_TypeDef channel_header_current[maxChannel];
DadaStruct3_TypeDef channel_header_tmp[maxChannel];

extern DadaStruct3_TypeDef channel_header_current[maxChannel];
extern DadaStruct3_TypeDef channel_header_tmp[maxChannel];

int64_t channels_ticks[maxChannel];

char password_code[6] = { 0 };

char password_code_tmp[6] = { 0, 0, 0, 0, 0, 0 };
char password_code_tmp_str[6][4] = { "[ ]", "[ ]", "[ ]", "[ ]", "[ ]", "[ ]" };

char password_code_tmp0[3][6] = { { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, {
		0, 0, 0, 0, 0, 0 } };

char password_config_code_tmp_str0[6][4] = { "[ ]", "[ ]", "[ ]", "[ ]", "[ ]",
		"[ ]" };
char password_config_code_tmp_str1[6][4] = { "[ ]", "[ ]", "[ ]", "[ ]", "[ ]",
		"[ ]" };
char password_config_code_tmp_str2[6][4] = { "[ ]", "[ ]", "[ ]", "[ ]", "[ ]",
		"[ ]" };

uint8_t password_error_timeout = 0;
uint8_t password_error_timeout0 = 0;

uint8_t password_request = password_request_yes;

uint8_t menu_config_index = 0;

uint8_t button1_restart_enabled = 0;

uint8_t temp_log_status = 0;

uint8_t usart_login = 0;
uint8_t usart_login_pw_index = 0;
uint8_t usart_login_pw_index_cmp = 0;
uint16_t usart_menu = 0;

uint32_t uC_ID[3]; // STM32 UuC_ID
uint8_t MEM_ID[8]; // W25Q64 UuC_ID

uint8_t cycle = 0;

uint8_t DS18B20_resolution_set = DS18B20_12_bits;

int lcd_bright_for_logo = 1023;
int lcd_bright_max = 900;
int lcd_bright_med = 500;
int lcd_bright_min = 75;
int lcd_bright_off = 0;

uint32_t tick100msMain = 0;
uint8_t tm1_seconds_old = 0;
uint8_t tm1_minutes_old = 0;
uint32_t tick100ms = 0;
uint32_t tick250ms = 0;
uint32_t tick500ms = 0;
uint32_t tick1000ms = 0;
uint8_t button_old = 0;
uint8_t blink_cnt1 = 0;
uint8_t blink_cnt2 = 0;
uint8_t blink_passwd = 0;
uint8_t blink_text = 0;
char str2[5];

//#define sensorFailTest 1	// uncomment to get fail

#ifndef sensorFailTest
OneWire_ChannelTypeDef OneWireCh0 = { SENSOR1_GPIO_Port, SENSOR1_Pin };
OneWire_ChannelTypeDef OneWireCh1 = { SENSOR2_GPIO_Port, SENSOR2_Pin };
OneWire_ChannelTypeDef OneWireCh2 = { SENSOR3_GPIO_Port, SENSOR3_Pin };
OneWire_ChannelTypeDef OneWireCh3 = { SENSOR4_GPIO_Port, SENSOR4_Pin };
OneWire_ChannelTypeDef OneWireCh4 = { SENSOR5_GPIO_Port, SENSOR5_Pin };
OneWire_ChannelTypeDef OneWireCh5 = { SENSOR6_GPIO_Port, SENSOR6_Pin };
OneWire_ChannelTypeDef OneWireCh6 = { SENSOR7_GPIO_Port, SENSOR7_Pin };
OneWire_ChannelTypeDef OneWireCh7 = { SENSOR8_GPIO_Port, SENSOR8_Pin };
#endif

#ifdef sensorFailTest
OneWire_ChannelTypeDef OneWireCh0 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh1 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh2 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh3 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh4 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh5 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh6 = {GPIOC, GPIO_PIN_15};
OneWire_ChannelTypeDef OneWireCh7 = {GPIOC, GPIO_PIN_15};
#endif

#define NumOfSensors 8

const uint8_t Num_Of_Sensors = NumOfSensors;
DS18B20_TypeDef SENSORS[NumOfSensors];

uint32_t timeout_temperature_read = 0;
uint8_t get_temperatures = 0;
uint8_t temperature_unit = TemperatureUnit_C; // TemperatureUnit_C, TemperatureUnit_F

const uint8_t ALARME1_ENABLE = ALARME_DISABLED;  // enabled = 1, disabled = 0
const uint8_t ALARME2_ENABLE = ALARME_ENABLED;  // enabled = 1, disabled = 0

uint8_t lcd_inter_busy = FALSE;
uint8_t rtc_busy = FALSE;
uint8_t acc_busy = FALSE;
uint8_t lcd_loop_busy = FALSE;
uint8_t update_info = FALSE;

DS3231_Time_t tm1;

uint32_t adc_update_tick = 0;
uint32_t adc_update_tick2 = 0;

uint32_t TP4056_charger_freq = 0;
uint32_t AD1_RES_ADC_DC_DC_5V = 0;
uint32_t AD2_RES_ADC_CHARGER = 0;
uint32_t AD3_RES_ADC_VBAT = 0;
uint32_t AD1_RES_ADC_VREF = 0;

/* ADC const calibration */
float const_adc_x2 = 0;
float const_adc_x16 = 0;

uint32_t battery_failure_timeout = 0;
volatile uint32_t battery_failure_cnt = 0;
volatile HAL_StatusTypeDef battery_failure_flag = HAL_OK;

uint8_t battery_status = 0;
float batt_percent_up = 0;
float batt_percent_down = 100;

int pwr_off_timeout = 0;

int back_to_main_timeout = 0;

DS3231_Time_t clock1;

int start_MSC_Application1 = 0;
int start_MSC_Application2 = 0;

uint32_t cnt0 = 0;
uint32_t cnt1 = 0;

uint8_t USB_FS_OC_Error = 0;
uint8_t USB_HS_OC_Error = 0;
uint8_t DC_DC_Error = 0;

uint8_t USBH_isInitialized = 0;

uint8_t USB_FS_restart = FALSE;
uint8_t lcd_refresh = 0;
uint8_t lcd_refresh_new = 0;
uint8_t lcd_updateInfo;

uint8_t RTC_Error = FALSE;

int read_interval[maxChannel] = { 1, 2, 3, 4, 5, 6, 7, 8 };

uint8_t top = 0;

char *FlashID[] = { "", "W25Q10", "W25Q20", "W25Q40", "W25Q80", "W25Q16",
		"W25Q32", "W25Q64", "W25Q128", "W25Q256", "W25Q512" };

uint8_t ADC_reading = FALSE;

uint8_t lcd_enabled = FALSE;

uint8_t receive_data[7], send_data[7];

const uint16_t lcd_timeout0recall = 20; // 20
const uint16_t lcd_timeout0recall2 = 180; // 180
const uint16_t back_to_main_timeout_recall = lcd_timeout0recall2 + 30; // 180
uint8_t lcd_timeout0 = 0;

uint8_t button1_stage = ScreenActive;
//uint8_t button1_info = 0;
uint8_t button1_flag = 0;
uint8_t button1_flag_old = 0;
uint8_t blink = FALSE;

uint8_t standby_mode = FALSE;
uint8_t lcd_restart = FALSE;

uint8_t milisec = 0;

uint8_t timer7_tick = FALSE;

uint32_t cycle_count0 = 0;
uint32_t cycle_count1 = 0;
uint8_t cycle_count2 = 0;

uint32_t ADC_raw[10];
uint8_t ADC_index = 0;

uint8_t checksumStatusError = 0;

uint8_t pwrOffLcdBack = 0;

uint8_t passwd_config_cnt = 0;

uint8_t config_ch_erase_enable = 0;
uint8_t config_ch_erase_step = 0;
HAL_StatusTypeDef HAL_StatusMemErase;
uint32_t config_ch_erase_progress = 0;
uint8_t config_ch_erase_progress_old = 0;

uint8_t channel_mark_index = 0;

uint8_t channel_save_data = 0; // start update

uint32_t ch_mark_cnt[maxChannel];

DadaStruct_TypeDef mem_data_buffer;

uint32_t EXTI_tick250ms = 0;

uint8_t read_sensors_screen = 0;

uint8_t read_sensors_error_cnt = 0;

extern uint8_t sysTick15000;

extern uint8_t kbd_LED_status;

extern ApplicationTypeDef Appli_state_FS;

extern HID_KEYBD_Info_TypeDef *keybd_info1;
extern uint8_t export_data_step;
extern uint32_t userFunction_phase;

extern uint8_t export_in_progress;
extern ApplicationTypeDef Appli_state_HS;
extern uint32_t write_mark_steps;
extern uint32_t read_mark_steps;
extern uint32_t write_data_steps;
extern uint32_t read_data_steps;

extern uint16_t isEmpty_steps1;
extern uint16_t isEmpty_steps2;
extern uint16_t erase_steps1;
extern uint16_t erase_steps2;
extern uint16_t zerofill_steps1;
extern uint16_t zerofill_steps2;
extern uint16_t channel_check_steps;

extern uint8_t test_ch[maxChannel];
extern uint8_t FLASH_Error;

extern uint8_t sensor_value_refresh;

extern uint16_t progress_usb_export;

extern int MPU6050_AccAngleX;
extern int MPU6050_AccAngleY;

extern int keyboard_timeout_reset; // 5
extern int keyboard_timeout; // 0
extern int keyboard_timeout2;  // 0
extern int usb_pwr_off_timeout; // 0

extern int label_index_edit;
extern int label_index_edit2;
extern uint8_t clock1_setup;
extern int lcd_phase;
extern int ch_config_index;

extern int lcd_bright_set;
extern int lcd_bright_old;
extern int lcd_screen;

extern uint8_t refresh_info;
extern char str[2048];
extern USBH_HandleTypeDef hUsbHostFS;
extern uint32_t enumTimeout;
extern int keyboard_timeout_reset;

#define BUFFER_SIZE_UART 1

uint8_t debug_rx_buffer[BUFFER_SIZE_UART];

uint32_t ADC_Conv_Timeout = 0;

uint32_t led_blink = 0;

//uint32_t check_point = 0;

#define V_Bat_val_index_max 100
uint8_t V_Bat_val_old[V_Bat_val_index_max];
uint8_t V_Bat_val_index = 0;
float batt_percent = 0.0;
float V_Bat_val = 0.0;
float V_Charger_val = 0.0;
float V_DC_DC_val = 0.0;

uint8_t batt_val10 = 0;
uint8_t batt_val1 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM8_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_IWDG_Init(void);
static void MX_ETH_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
uint32_t Crc32(uint32_t Crc, uint32_t Data);

void load_settings(void);
void main_init(void);
void DS3231_init(void);

HAL_StatusTypeDef readTemperatureSensors(void);
void beep(uint8_t repetions, uint8_t is_error);
void IWDG_reset(void);

void print_temperatures(void);

void string2byte(const char *input, uint32_t inputLength, uint8_t *output);
void byte2string(const uint8_t *input, uint32_t inputLength, char *output);
uint32_t sizeof_array(const char *input);

HAL_StatusTypeDef DS3231_Setup(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
float map(float x, float in_min, float in_max, float out_min, float out_max);
void UART_Printf(const char *fmt, ...);
void ADC_get(void);
void get_ADC_Vref(void);
void user_pwm_setvalue(uint16_t value);
void password_user_clear(void);
void password_config_clear(void);
void get_data_from_mem_for_main(void);

//static int test_encrypt_ctr(void);
//static int test_decrypt_ctr(void);
void MX_SPI3_Init2(void);
void TIM7_IRQHandler(void);

extern void BASE64_encode(const char *input, char *output);
extern uint32_t BASE64_encodeLength(uint32_t inputLength);
extern void BASE64_decode(const char *input, char *output);
extern uint32_t BASE64_decodeLength(const char *input);

extern void IWDG_delay_ms(uint32_t delay);
extern void IWDG_reset(void);

extern void HC595_init(void);
extern void HC595_set(uint8_t value);
extern HAL_StatusTypeDef DS18B20_init(void);
extern void DS18B20_IO_as_INPUT(OneWire_ChannelTypeDef channel);
extern void DS18B20_IO_as_OUTPUT(OneWire_ChannelTypeDef channel);

extern void lcd_graph_test(void);
extern void MX_USB_HOST_Process_HS(void);
extern void MX_USB_HOST_Process_FS(void);
extern void USBH_UserProcess2(USBH_HandleTypeDef *phost, uint8_t id);
extern USBH_StatusTypeDef USBH_FS_Restart(void);
extern uint8_t MSC_Application(void);
extern void draw_logo(const unsigned short *logo);
//extern uint8_t lcd_memory_test_progress_bar(uint16_t progress, uint16_t max);
extern uint8_t lcd_memory_test_status(uint8_t ch, uint8_t step, uint8_t max);
extern uint8_t lcd_memory_test(void);
extern uint8_t userFunctionExport(void);
extern HAL_StatusTypeDef save_BMP_Export(void);
extern void update_exported_data(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define BUFFER_SIZE	(0x100000 - 4)/4 // (1MB - 4)

__IO uint32_t uwCRCValue = 0;
__IO uint32_t myCRCValue1 = 0xFFFFFFFF;
__IO uint32_t ucChecksum = 0xFFFFFFFF;

const uint32_t *myDataBuffer;

const char compile_datetime[] = __DATE__ " " __TIME__;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	// https://www.openstm32.org/Using%2BCCM%2BMemory
	// https://stackoverflow.com/questions/64203031/gcc-ignores-attribute-sectionccmram-how-to-fix
	//	for (int i = 0; i < 1024; i++) {
	//		in_ccram_buffer[i] = 0;
	//	}
	uint16_t lcd_config_ch_label_color;
	HAL_StatusTypeDef result_write_data = HAL_BUSY;

	int mem_test_channel_progress_percent = 0;
	int mem_test_channel_progress_percent_old = 0;

	int config_ch_erase_progress_percent = 0;
	int config_ch_erase_progress_percent_old = 0;

	myDataBuffer = (uint32_t*) 0x08000000;

	if (DWT_Delay_Init()) {
		Error_Handler(); /* Call Error Handler */
	}

	for (uint8_t i = 0; i < maxChannel; i++) {
		for (uint8_t j = 0; j < maxChar; j++) {
			channelLabel[i][j] = 0;
		}
	}

	SENSORS[0].CHANNEL = OneWireCh0;
	SENSORS[1].CHANNEL = OneWireCh1;
	SENSORS[2].CHANNEL = OneWireCh2;
	SENSORS[3].CHANNEL = OneWireCh3;
	SENSORS[4].CHANNEL = OneWireCh4;
	SENSORS[5].CHANNEL = OneWireCh5;
	SENSORS[6].CHANNEL = OneWireCh6;
	SENSORS[7].CHANNEL = OneWireCh7;

	for (int i = 0; i < maxChannel; i++) {
		SENSORS[i].FLAG_ENABLED = 1;
		SENSORS[i].FLAG_TIMEOUT = 0;
		SENSORS[i].FLAG_ERROR = 0;
		SENSORS[i].FLAG_ID_ERROR = 0;
		SENSORS[i].TEMPERATURE = 0.0;
	}
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_ADC2_Init();
	MX_ADC3_Init();
	MX_CRC_Init();
	MX_I2C1_Init();
	MX_IWDG_Init();
	MX_SPI1_Init();
	MX_SPI3_Init();
	MX_TIM7_Init();
	MX_TIM8_Init();
	MX_USART1_UART_Init();
	MX_FATFS_Init();
	// MX_USB_HOST_Init();
	MX_I2C3_Init();
	MX_ETH_Init();
	/* USER CODE BEGIN 2 */
	UsrLog("\033[2J\033[H");
	UsrLog(" ");
	UsrLog("App start");
	UsrLog("MCU-ID %08lX", DBGMCU->IDCODE);

	UsrLog(" ");
	UsrLog("Power On");
	UsrLog("%s", Firm_Mod);
	UsrLog("Fir. Ver: %s", Firm_Ver);

	UsrLog("by %s (Year: %s)", byRef, byYear);

	UsrLog("Compile datetime: %s", compile_datetime);

//	UsrLog("\nTesting AES256\n\n");

//	test_encrypt_ctr();
//	test_decrypt_ctr();

	HAL_UART_Receive_IT(&huart1, debug_rx_buffer, BUFFER_SIZE_UART);

#if DISABLE_FLASH_CRC_AT_BOOT == 0
	uwCRCValue = HAL_CRC_Accumulate(&hcrc, (uint32_t*) 0x08000000, BUFFER_SIZE);

	if (uwCRCValue == *(uint32_t*) 0x080FFFFC) // 0x080FFFFC STM32F407VGT6: 1MB = 0xFFFFF
			{
		UsrLog("CRC Check: OK");
	} else {
		while (1) {
			UsrLog("CRC Check: Fail");
			checksumStatusError = 1;

			UsrLog("Checksum generated: 0x%08X", (unsigned int ) uwCRCValue);

			UsrLog("Checksum stored:    0x%08X", *(unsigned int* )0x080FFFFC);

#if DISABLE_BEEP_FOR_DEBUG == 0
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // BUZZER On
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off
#endif // #if DISABLE_BEEP_FOR_DEBUG == 0

			IWDG_delay_ms(2000);

#if DISABLE_BEEP_FOR_DEBUG == 0
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // BUZZER Off
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On
#endif // #if DISABLE_BEEP_FOR_DEBUG == 0

			IWDG_delay_ms(2000);
		}
	}
#endif // #if DISABLE_FLASH_CRC_AT_BOOT == 0

	main_init();

	MX_USB_HOST_Init();

	button1_restart_enabled = 1; // enable read for SysTick_Handler();

	if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port, AC_DC_STATE_Pin)
			== GPIO_PIN_SET) {
		lcd_timeout0 = lcd_timeout0recall2;
	} else {
		lcd_timeout0 = lcd_timeout0recall;
	}

	if (readTemperatureSensors() != HAL_OK) {
		//UsrLog("readTemperatureSensors ERROR");
	}

	if (readTemperatureSensors() != HAL_OK) {
		//UsrLog("readTemperatureSensors ERROR");
	}

	print_temperatures();

	keyboard_timeout_reset = 5;

	keyboard_timeout = 2;

	keyboard_timeout2 = 2;

	get_data_from_mem_for_main();

//	UsrLog("V_Bat_val: %.3f", V_Bat_val);

	float V_Bat_val_tmp = V_Bat_val;

	V_Bat_val_tmp -= 3.5;

	V_Bat_val_tmp /= 0.004;

	if (V_Bat_val_tmp > 255) {
		V_Bat_val_tmp = 255;
	}

	for (int i = 0; i < (V_Bat_val_index_max - 1); i++) {
		V_Bat_val_old[i] = V_Bat_val_tmp;
	}

	ADC_get();

	UsrLog("V_Charger_val (9 to 24 VDC): %.3f V", V_Charger_val);
	UsrLog("V_Bat_val (3.5 to 4.2 VDC): %.3f V", V_Bat_val);
	UsrLog("V_DC_DC_val (0 or 5V): %.3f V", V_DC_DC_val);

	init_flag = 1;

#ifdef check_lcd_status
	lcd_status_reference = 0;

	uint32_t lcd_status_ref[5] = { 0, 0, 0, 0, 0 };
	uint8_t lcd_status_index = 0;
	uint8_t lcd_status_test = 0;

	while ((lcd_status_reference == 0) && (lcd_status_test < 5)) {
		if (lcd_status_index < 5) {
			lcd_status_index++;
		} else {
			lcd_status_index = 0;
		}

		lcd_status_ref[lcd_status_index] = ILI9341_Status();

		if ((lcd_status_index == 4) && (lcd_status_ref[0] != 0)
				&& (lcd_status_ref[0] == lcd_status_ref[1])
				&& (lcd_status_ref[0] == lcd_status_ref[2])
				&& (lcd_status_ref[0] == lcd_status_ref[3])
				&& (lcd_status_ref[0] == lcd_status_ref[4])) {
			lcd_status_reference = lcd_status_ref[0];
		} else {
			if (lcd_status_index == 4) {
				UsrLog("Error lcd status: 0x%02lX", lcd_status_ref[0]);

				beep(1, 0);

				lcd_status_test++;
			}
		}

		IWDG_delay_ms(50);
	}
#endif
	// Enable 5V for USB ports
	// Need reverse the VBUS command: /USB_HOST/Target/platform.c
	HAL_GPIO_WritePin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin, GPIO_PIN_SET);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
		// MX_USB_HOST_Process(); // not used, separated into FS and HS
		/* USER CODE BEGIN 3 */

		//		led_blink = 0;
		//		check_point = 1;
		//		if(led_blink < 10000){
		//			led_blink++;
		//		} else {
		//			led_blink = 0;
		//
		//			HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // LED2 Toggle
		//		}
		if (lcd_bright_old != lcd_bright_set) {
			if (it_flag == 0) {
				it_flag = 1;
				HAL_TIM_Base_Start_IT(&htim7);
			}
		} else {
			if (it_flag == 1) {
				it_flag = 0;
				HAL_TIM_Base_Stop_IT(&htim7);
			}
		}

		//		check_point = 2;

		MX_USB_HOST_Process_FS();

		IWDG_delay_ms(0);

		//		check_point = 3;

		drawInfo();

		//		check_point = 4;

		TIM7_IRQHandler();

		//		check_point = 5;

		lcd_menu_keyboard_async();

		//		check_point = 6;

		if (tm1_seconds_old != tm1.seconds) {
			tm1_seconds_old = tm1.seconds;

//			uint32_t time_check = HAL_GetTick();

			//			UsrLog("seconds: %d", tm1.seconds);

			//			kbd_LED_status = tm1.seconds;

			if ((tm1.seconds == 10) || (tm1.seconds == 30)
					|| (tm1.seconds == 50)) {
				HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_SET); // I2C Power On
				//				UsrLog("I2C1, DS3231: Line power On");

				IWDG_delay_ms(10);

				if (DS3231_GetDateTime(&tm1) == HAL_OK) {
					tm1_minutes_old = tm1.minutes;
				} else {
					HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin,
							GPIO_PIN_SET);

					IWDG_delay_ms(250);

					HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin,
							GPIO_PIN_RESET);

					IWDG_delay_ms(250);

					UsrLog("DS3231_GetDateTime ERROR");

					if (DS3231_GetDateTime(&tm1) == HAL_OK) {
						tm1_minutes_old = tm1.minutes;
					} else {
						HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin,
								GPIO_PIN_SET);

						IWDG_delay_ms(250);

						HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin,
								GPIO_PIN_RESET);

						IWDG_delay_ms(250);

						UsrLog("DS3231_GetDateTime ERROR");
					}
				}

				//				UsrLog("I2C1, DS3231: Line power Off");
				//HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin,
				//		GPIO_PIN_RESET); // I2C Power Off
			}

//			time_check = HAL_GetTick() - time_check;
//
//			if (time_check > 10) {
//				UsrLog("time_check: %lu", time_check);
//			}

			uint8_t alarmFlagBit = 0;

			if (HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port, RTC_INT_STATE_Pin)
					== GPIO_PIN_SET) {
				//				HAL_UART_Receive_IT(&huart1, debug_rx_buffer, BUFFER_SIZE_UART);
				HAL_UART_AbortReceive_IT(&huart1);
				UsrLog("DS3231 Alarm flag");

				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off

				//				if (temp_log_status == 0) {
				//					UsrLog("temp_log_status == 0");
				if ((lcd_screen != LcdScreenMonCh)
						|| ((lcd_screen == LcdScreenMonCh)
								&& (read_sensors_screen >= 9))) {
					if (read_sensors_screen < 9) {
						read_sensors_screen++;
					} else if (lcd_screen == LcdScreenMonCh) {
						read_sensors_screen = 0;
					}

					//						UsrLog("lcd_screen != LcdScreenMonCh");
					if (HAL_GPIO_ReadPin(SENSOR_PWR_GPIO_Port,
					SENSOR_PWR_Pin) == GPIO_PIN_RESET) {
						HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port,
						SENSOR_PWR_Pin, GPIO_PIN_SET); // sensors on

						IWDG_delay_ms(250);
					}

					if (readTemperatureSensors() != HAL_OK) {
						beep(1, 1);

						UsrLog("read Temperature Sensors ERROR");
					}

					temp_log_status = 1;

					print_temperatures();
				} else {
					print_temperatures();

					temp_log_status = 1;
				}

				if (lcd_screen == LcdScreenCfgCh) {
					lcd_refresh = 3;
				}
				//				} else {
				HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_SET); // I2C1 Power On

				if (DS3231_Alarm(ALARM_1, &alarmFlagBit) != HAL_OK) // [ ] verification added
						{
					UsrLog("DS3231_Alarm 1 ERROR");
				}

				if (DS3231_Alarm(ALARM_2, &alarmFlagBit) != HAL_OK) {
					UsrLog("DS3231_Alarm 2 ERROR");
				}

				IWDG_delay_ms(0);

				HAL_NVIC_DisableIRQ(EXTI0_IRQn);
				HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
				HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

				HAL_TIM_Base_Stop_IT(&htim7);

				if (MPU6050_acc() != HAL_OK) {
					HAL_GPIO_WritePin(I2C3_PWR_GPIO_Port, I2C3_PWR_Pin,
							GPIO_PIN_RESET); // I2C3 Power Off

					IWDG_delay_ms(10);

					HAL_GPIO_WritePin(I2C3_PWR_GPIO_Port, I2C3_PWR_Pin,
							GPIO_PIN_SET); // I2C3 Power On

					IWDG_delay_ms(10);

					UsrLog("MPU6050 retart");

					MPU6050_init(NO_CALC_ERROR);

					MPU6050_acc();

					UsrLog("Pass 2- Acc X:%d; Y:%d", MPU6050_AccAngleX,
							MPU6050_AccAngleY);
				} else {
					UsrLog("Pass 1- Acc X:%d; Y:%d", MPU6050_AccAngleX,
							MPU6050_AccAngleY);
				}

				HAL_NVIC_EnableIRQ(EXTI0_IRQn);
				HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
				HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

				HAL_TIM_Base_Start_IT(&htim7);

				HAL_GPIO_WritePin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin,
						GPIO_PIN_RESET); // USB On

				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On
			}

			if ((temp_log_status > 0)
					& (HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port,
					RTC_INT_STATE_Pin) == GPIO_PIN_RESET)) {
				temp_log_status = 0;

				channel_save_data = 1;

				save_data_step = 0;
			}
		}

		//		check_point = 7;

		if (channel_save_data == 1) {
			//			UsrLog("channel_save_data = 1");
			if (HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port, RTC_INT_STATE_Pin)
					== GPIO_PIN_RESET) {
				if (save_data_step == 0) {
					//				UsrLog("save_data_step == 0");

					save_data_step++;

					channel_mark_index = 0;

					write_mark_steps = 0;

					write_mark_steps = W25Q64_read_mark_preset(
							channel_mark_index);

					int ch_status_test = 0;

					for (int i = 0; i < maxChannel; i++) {
						if (ch_status[i] == 1) {
							ch_status_test++;
						}
					}

					if (ch_status_test == 0) {
						channel_save_data = 0;

						HAL_UART_Receive_IT(&huart1, debug_rx_buffer,
						BUFFER_SIZE_UART);
					}

				} else if (save_data_step == 1) {
					//UsrLog("save_data_step == 1");

					if (channel_mark_index < 8) {
						if ((lcd_screen == LcdScreenCfgChMemErase)
								&& (ch_config_index == channel_mark_index)) {
							channel_mark_index++;
						} else {
							if ((ch_status[channel_mark_index] == 1)
									&& ((ch_config_index != channel_mark_index)
											|| (lcd_screen
													!= LcdScreenCfgChMemErase))) {
								//								UsrLog("write mark %d: %d", channel_mark_index,
								//										ch_status[channel_mark_index]);

								if (W25Q64_write_mark(channel_mark_index)
										== HAL_OK) {
									channel_mark_index++;

									write_mark_steps = W25Q64_read_mark_preset(
											channel_mark_index);
								}
							} else {
								//							UsrLog("ch_status %d: %d", channel_mark_index,
								//									ch_status[channel_mark_index]);

								channel_mark_index++;
								write_mark_steps = W25Q64_read_mark_preset(
										channel_mark_index);
							}
						}
					} else {
						save_data_step++;

						channel_mark_index = 0;

						read_mark_steps = W25Q64_read_mark_preset(
								channel_mark_index);
					}
				} else if (save_data_step == 2) { // W25Q64_read_mark(i, ch_mark_cnt);
					if (channel_mark_index < 8) {
						if ((lcd_screen == LcdScreenCfgChMemErase)
								&& (ch_config_index == channel_mark_index)) {
							channel_mark_index++;
						} else {
							if (ch_status[channel_mark_index] == 1) {
								//							UsrLog("write mark %d: %d", channel_mark_index,
								//									ch_status[channel_mark_index]);
								if (W25Q64_read_mark(channel_mark_index,
										ch_mark_cnt) == HAL_OK) {
									channel_mark_index++;
									read_mark_steps = W25Q64_read_mark_preset(
											channel_mark_index);
								}
							} else {
								//							UsrLog("ch_status %d: %d", channel_mark_index,
								//									ch_status[channel_mark_index]);

								channel_mark_index++;
								read_mark_steps = W25Q64_read_mark_preset(
										channel_mark_index);
							}
						}
					} else {
						save_data_step++;

						channel_mark_index = 0;

						write_data_steps = W25Q64_read_mark_preset(
								channel_mark_index);
					}
				} else if (save_data_step == 3) { // W25Q64_read_mark(i, ch_mark_cnt);
					//					UsrLog("channel_mark_index = %d", channel_mark_index);
					if (channel_mark_index < 8) {
						if ((lcd_screen == LcdScreenCfgChMemErase)
								&& (ch_config_index == channel_mark_index)) {
							channel_mark_index++;
						} else {
							if (ch_status[channel_mark_index] == 1) {
								//								UsrLog("channel_mark_index = %d", channel_mark_index);

								//UsrLog("0x%04X", SENSORS[channel_mark_index].TEMPERATURE_RAW);

								mem_data_buffer.Temperature[0] =
										(uint8_t) (SENSORS[channel_mark_index].TEMPERATURE_RAW
												& 0xFF);

								mem_data_buffer.Temperature[1] =
										(uint8_t) ((SENSORS[channel_mark_index].TEMPERATURE_RAW
												>> 8) & 0xFF);

								for (int i = 0; i < maxChannel; i++) {
									mem_data_buffer.Sensor_Id[i] =
											SENSORS[channel_mark_index].SERIAL_ID[i];
								}

								mem_data_buffer.AccX = MPU6050_AccAngleX + 127;

								mem_data_buffer.AccY = MPU6050_AccAngleY + 127;

								if (lcd_screen != LcdScreenMonCh) {
									if ((HAL_GetTick() - adc_update_tick)
											>= 500) {
										adc_update_tick = HAL_GetTick();

										ADC_get();
									}
								}

								float V_Bat_val_tmp = V_Bat_val;

								V_Bat_val_tmp -= 3.5;

								V_Bat_val_tmp /= 0.004;

								if (V_Bat_val_tmp > 255) {
									V_Bat_val_tmp = 255;
								}

								mem_data_buffer.BattV = V_Bat_val_tmp;

								float V_Charger_val_ptm = V_Charger_val;

								V_Charger_val_ptm -= 7.0;

								V_Charger_val_ptm /= 0.1;

								if (V_Charger_val_ptm > 255)
									V_Charger_val_ptm = 255;

								mem_data_buffer.ChargerV = V_Charger_val_ptm;

								mem_data_buffer.Date[0] = tm1.date;
								mem_data_buffer.Date[1] = tm1.month;
								mem_data_buffer.Date[2] = tm1.year;

								mem_data_buffer.Time[0] = tm1.hours;
								mem_data_buffer.Time[1] = tm1.minutes;

								result_write_data = W25Q64_write_data(
										channel_mark_index,
										ch_mark_cnt[channel_mark_index] - 1,
										&mem_data_buffer);

								if (ch_exported[channel_mark_index] == 1) {
									ch_exported[channel_mark_index] = 0; // set as not exported

									update_exported_data();
								}

								if (result_write_data == HAL_OK) {

									channel_mark_index++;

									write_data_steps = W25Q64_read_mark_preset(
											channel_mark_index);
								} else if (result_write_data == HAL_ERROR) {
									//									UsrLog("W25Q64 write data: ERROR");
									UsrLog(
											"lcd on) ERROR write data ch:%d addr:0x%05lX temp:%.4f",
											channel_mark_index,
											ch_mark_cnt[channel_mark_index],
											SENSORS[channel_mark_index].TEMPERATURE);

									channel_mark_index++;

									write_data_steps = W25Q64_read_mark_preset(
											channel_mark_index);
								}
							} else {
								channel_mark_index++;

								write_data_steps = W25Q64_read_mark_preset(
										channel_mark_index);
							}
						}
					} else {
						save_data_step++;

						channel_mark_index = 0;

						read_mark_steps = W25Q64_read_mark_preset(
								channel_mark_index);
					}
				} else if (save_data_step == 4) {
					//					UsrLog("save_data_step = 4");
					if (channel_mark_index < 8) {
						if (ch_status[channel_mark_index] == 1) {
							if (W25Q64_read_mark(channel_mark_index,
									ch_mark_cnt) == HAL_OK) {
								//								UsrLog("ch%d channel_mark_index: %lu", channel_mark_index, ch_mark_cnt[channel_mark_index]);

								if (ch_mark_cnt[channel_mark_index]
										>= Max_Record_Data) { // 54541
									ch_config_stop(channel_mark_index);

									ch_status[channel_mark_index] = 0;

									ch_status_tmp[channel_mark_index] = 0;

									get_data_from_mem_for_main();

									lcd_refresh = 3;
								}

								channel_mark_index++;

								read_mark_steps = W25Q64_read_mark_preset(
										channel_mark_index);
							}
						} else {
							channel_mark_index++;

							read_mark_steps = W25Q64_read_mark_preset(
									channel_mark_index);
						}
					} else {
						channel_save_data = 0;

						HAL_UART_Receive_IT(&huart1, debug_rx_buffer,
						BUFFER_SIZE_UART);
					}
				}

			}
		} else if (export_data_enabled == 1) {
			if (Appli_state_HS == APPLICATION_READY) {
				userFunctionExport();
			} else {
				if ((userFunction_phase == 7) && (export_data_enabled == 1)) {
					if ((HAL_GetTick() - export_data_timeout) >= 4000) {
						HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
								GPIO_PIN_SET); // USB off

						userFunction_phase = 0;

						export_data_enabled = 0;
					}
				} else {
					if ((HAL_GetTick() - save_BMP_timeout) >= 10000) {
						save_BMP_enabled = 0;

						UsrLog("Error: save_BMP_enabled = 0");

						HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
								GPIO_PIN_SET); // USB off
					}
				}
			}
		} else if (save_BMP_enabled == 1) {
			if (Appli_state_HS == APPLICATION_READY) {
				save_BMP_Export();

				save_BMP_timeout = HAL_GetTick();
			} else {
				if ((save_BMP_phase == 3) && (save_BMP_enabled == 1)) {
					if ((HAL_GetTick() - save_BMP_timeout) >= 4000) {
						HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
								GPIO_PIN_SET); // USB off

						save_BMP_phase = 0;

						save_BMP_enabled = 0;
					}
				} else {
					if ((HAL_GetTick() - save_BMP_timeout) > 10000) {
						save_BMP_enabled = 0;

						save_BMP_phase = 0;

						UsrLog("Error: save_BMP_timeout");

						HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
								GPIO_PIN_SET); // USB off
					}
				}
			}
		} else {
			if (lcd_screen == LcdScreenMemTest) {
				if (channel_save_data == 0) {
					if (mem_test_enabled == 1) {
						if (mem_test_step == 0) {
							mem_test_step = 1;

							if ((test_ch[mem_test_channel] == 0)
									|| (ch_status[mem_test_channel] == 1)) {
								if (ch_status[mem_test_channel] == 1) {
									memTestStatus[mem_test_channel] =
											memTestStatusLock;

									sprintf(str,
											"Status: CH%d bloqueado       ",
											mem_test_channel + 1);
									ILI9341_WriteString(3, 195, str, Font_11x18,
									ILI9341_BLUE,
									ILI9341_BLACK);
								} else {
									sprintf(str,
											"Status: CH%d pular           ",
											mem_test_channel + 1);
									ILI9341_WriteString(3, 195, str, Font_11x18,
									ILI9341_BLUE,
									ILI9341_BLACK);
								}

								if (mem_test_channel < 7) {
									mem_test_channel++;

									mem_test_step = 0;
								} else {
									mem_test_enabled = 0;

									mem_test_step = 0;

									lcd_refresh = 3;
								}
							} else {
								memTestStatus[mem_test_channel] =
										memTestStatusTest;

								zerofill_steps1 = 0;

								zerofill_steps2 = 0;

								sprintf(str, "Status: CH%d zerofill 0%%     ",
										mem_test_channel + 1);
								ILI9341_WriteString(3, 195, str, Font_11x18,
								ILI9341_BLUE,
								ILI9341_BLACK);
							}

							lcd_memory_test();
						} else if (mem_test_step == 1) {
							mem_test_channel_progress_percent =
									(mem_test_channel_progress * 100) / 29;

							//					UsrLog("1) mem_test_channel_progress: %lu", mem_test_channel_progress);

							if (mem_test_channel_progress_percent > 100) {
								mem_test_channel_progress_percent = 100;
							}

							if (mem_test_channel_progress_percent_old
									!= mem_test_channel_progress_percent) {
								mem_test_channel_progress_percent_old =
										mem_test_channel_progress_percent;

								sprintf(str, "Status: CH%d zerofill %d%%   ",
										mem_test_channel + 1,
										mem_test_channel_progress_percent);
								ILI9341_WriteString(3, 195, str, Font_11x18,
								ILI9341_BLUE,
								ILI9341_BLACK);
							}

							HAL_StatusMemTest = W25Q64_channel_zerofill(
									mem_test_channel);
							if (HAL_StatusMemTest == HAL_OK) {
								mem_test_step++;

								mem_test_channel_progress = 0;

								channel_check_steps = 0;
							} else if (HAL_StatusMemTest == HAL_BUSY) {
								mem_test_channel_progress++;
							}
						} else if (mem_test_step == 2) {
							mem_test_channel_progress_percent =
									(mem_test_channel_progress * 100) / 4000;

							//					UsrLog("2) mem_test_channel_progress: %lu", mem_test_channel_progress);

							if (mem_test_channel_progress_percent > 100) {
								mem_test_channel_progress_percent = 100;
							}

							if (mem_test_channel_progress_percent_old
									!= mem_test_channel_progress_percent) {
								mem_test_channel_progress_percent_old =
										mem_test_channel_progress_percent;

								sprintf(str, "Status: CH%d verif 0x00 %d%%  ",
										mem_test_channel + 1,
										mem_test_channel_progress_percent);
								ILI9341_WriteString(3, 195, str, Font_11x18,
								ILI9341_BLUE,
								ILI9341_BLACK);
							}

							HAL_StatusMemTest = W25Q64_channel_check(
									mem_test_channel, 0x00);
							if (HAL_StatusMemTest == HAL_OK) {
								mem_test_step++;

								mem_test_channel_progress = 0;

								erase_steps1 = 0;

								erase_steps2 = 0;
							} else if (HAL_StatusMemTest == HAL_BUSY) {
								mem_test_channel_progress++;
							} else if (HAL_StatusMemTest == HAL_ERROR) {
								memTestStatus[mem_test_channel] =
										memTestStatusFail;

								if (mem_test_channel < 7) {
									mem_test_channel++;

									mem_test_step = 0;

									lcd_refresh = 3;
								}
							}
						} else if (mem_test_step == 3) {
							mem_test_channel_progress_percent =
									(mem_test_channel_progress * 100) / 29;

							//UsrLog("3) mem_test_channel_progress: %lu",
							//		mem_test_channel_progress);

							if (mem_test_channel_progress_percent > 100) {
								mem_test_channel_progress_percent = 100;
							}

							if (mem_test_channel_progress_percent_old
									!= mem_test_channel_progress_percent) {
								mem_test_channel_progress_percent_old =
										mem_test_channel_progress_percent;

								sprintf(str, "Status: CH%d apagando %d%%    ",
										mem_test_channel + 1,
										mem_test_channel_progress_percent);
								ILI9341_WriteString(3, 195, str, Font_11x18,
								ILI9341_BLUE,
								ILI9341_BLACK);
							}

							HAL_StatusMemTest = W25Q64_channel_erase(
									mem_test_channel);
							if (HAL_StatusMemTest == HAL_OK) {
								mem_test_step++;

								mem_test_channel_progress = 0;

								channel_check_steps = 0;
							} else if (HAL_StatusMemTest == HAL_BUSY) {
								mem_test_channel_progress++;
							}
						} else if (mem_test_step == 4) {
							mem_test_channel_progress_percent =
									(mem_test_channel_progress * 100) / 4000;

							//					UsrLog("4) mem_test_channel_progress: %lu", mem_test_channel_progress);

							if (mem_test_channel_progress_percent > 100) {
								mem_test_channel_progress_percent = 100;
							}

							if (mem_test_channel_progress_percent_old
									!= mem_test_channel_progress_percent) {
								mem_test_channel_progress_percent_old =
										mem_test_channel_progress_percent;

								sprintf(str, "Status: CH%d verif 0xFF %d%% ",
										mem_test_channel + 1,
										mem_test_channel_progress_percent);
								ILI9341_WriteString(3, 195, str, Font_11x18,
								ILI9341_BLUE,
								ILI9341_BLACK);
							}

							HAL_StatusMemTest = W25Q64_channel_check(
									mem_test_channel, 0xFF);
							if (HAL_StatusMemTest == HAL_OK) {
								mem_test_step++;

								memTestStatus[mem_test_channel] =
										memTestStatusPass;
								mem_test_channel_progress = 0;
							} else if (HAL_StatusMemTest == HAL_BUSY) {
								mem_test_channel_progress++;
							} else if (HAL_StatusMemTest == HAL_ERROR) {
								mem_test_step++;

								memTestStatus[mem_test_channel] =
										memTestStatusFail;
							}
						} else if (mem_test_step == 5) {

							W25Q64_update_name(mem_test_channel,
									channelLabel[ch_config_index]);

							if (mem_test_channel < 7) {
								mem_test_channel++;
							} else {
								mem_test_enabled = 0;

								lcd_refresh = 3;
							}

							mem_test_step = 0;
						}
					} else {
						mem_test_step = 0;

						mem_test_channel_progress = 0;
					}
				}
			}

			//			UsrLog("lcd_screen = %d", lcd_screen);

			if (lcd_screen == LcdScreenCfgChMemErase) {
				//				UsrLog("lcd_screen = LcdScreenCfgChMemErase");
				if ((config_ch_erase_enable == 1)
						&& (lcd_screen_old == LcdScreenCfgChMemErase)) {

					//					UsrLog("config_ch_erase_enable = 1 + lcd_screen_old == LcdScreenCfgChMemErase");
					if (config_ch_erase_step == 0) {
						config_ch_erase_progress = 0;

						isEmpty_steps1 = 0;

						isEmpty_steps2 = 0;

						config_ch_erase_step++;

						sprintf(str, "Status: Verificar canal     ");
						ILI9341_WriteString(3, 95, str, Font_11x18,
						ILI9341_GREEN,
						ILI9341_BLACK);
					} else if (config_ch_erase_step == 1) {
						HAL_StatusMemErase = W25Q64_channel_is_empty(
								ch_config_index);

						if (HAL_StatusMemErase == HAL_OK) {
							sprintf(str, "Status: Canal limpo         ");
							ILI9341_WriteString(3, 95, str, Font_11x18,
							ILI9341_GREEN,
							ILI9341_BLACK);

							sprintf(str, "ENTER: Sair                 ");
							ILI9341_WriteString(3, 195, str, Font_11x18,
							ILI9341_LIGHTGREY,
							ILI9341_BLACK);

							config_ch_erase_enable = 0;

							ch_config_start(ch_config_index);

							get_data_from_mem_for_main();
						} else if (HAL_StatusMemErase == HAL_ERROR) {
							sprintf(str, "Status: Limpar canal        ");
							ILI9341_WriteString(3, 95, str, Font_11x18,
							ILI9341_GREEN,
							ILI9341_BLACK);

							erase_steps1 = 0;

							erase_steps2 = 0;

							config_ch_erase_step++;
						}
					} else if (config_ch_erase_step == 2) {
						config_ch_erase_progress_percent =
								(config_ch_erase_progress * 100) / 29;

						//					UsrLog("1) config_ch_erase_progress: %lu", config_ch_erase_progress);

						if (config_ch_erase_progress_percent > 100) {
							config_ch_erase_progress_percent = 100;
						}

						if (config_ch_erase_progress_percent_old
								!= config_ch_erase_progress_percent) {
							config_ch_erase_progress_percent_old =
									config_ch_erase_progress_percent;

							sprintf(str, "Status: Limpando %d%%       ",
									config_ch_erase_progress_percent);
							ILI9341_WriteString(3, 95, str, Font_11x18,
							ILI9341_BLUE,
							ILI9341_BLACK);
						}

						HAL_StatusMemErase = W25Q64_channel_erase(
								ch_config_index);
						if (HAL_StatusMemErase == HAL_OK) {
							config_ch_erase_step++;

							config_ch_erase_progress = 0;

							channel_check_steps = 0;

						} else if (HAL_StatusMemErase == HAL_BUSY) {
							config_ch_erase_progress++;
						}
					} else if (config_ch_erase_step == 3) {
						config_ch_erase_progress_percent =
								(config_ch_erase_progress * 100) / 29;

						//					UsrLog("2) config_ch_erase_progress: %lu", config_ch_erase_progress);

						if (config_ch_erase_progress_percent > 100) {
							config_ch_erase_progress_percent = 100;
						}

						if (config_ch_erase_progress_percent_old
								!= config_ch_erase_progress_percent) {
							config_ch_erase_progress_percent_old =
									config_ch_erase_progress_percent;

							sprintf(str, "Status: Verif 0xFF %d%%    ",
									config_ch_erase_progress_percent);
							ILI9341_WriteString(3, 95, str, Font_11x18,
							ILI9341_BLUE,
							ILI9341_BLACK);
						}

						HAL_StatusMemErase = W25Q64_channel_is_empty(
								ch_config_index);

						if (HAL_StatusMemErase == HAL_OK) {

							config_ch_erase_enable = 0;

							ch_config_start(ch_config_index);

							W25Q64_update_name(ch_config_index,
									channelLabel[ch_config_index]);

							get_data_from_mem_for_main();

							lcd_screen = LcdScreenCfgCh;

							lcd_refresh = 3;
						} else if (HAL_StatusMemErase == HAL_BUSY) {
							config_ch_erase_progress++;
						} else if (HAL_StatusMemErase == HAL_ERROR) {
							//						UsrLog("Verif error");

							config_ch_erase_enable = 0;

							sprintf(str, "Status: Verif 0xFF ERRO       ");
							ILI9341_WriteString(3, 95, str, Font_11x18,
							ILI9341_RED,
							ILI9341_BLACK);

							sprintf(str, "ERRO: Nao usar este canal     ");
							ILI9341_WriteString(3, 145, str, Font_11x18,
							ILI9341_ORANGE,
							ILI9341_BLACK);

							sprintf(str, "ENTER: Sair");
							ILI9341_WriteString(3, 195, str, Font_11x18,
							ILI9341_LIGHTGREY,
							ILI9341_BLACK);
						}
					}
				}
			}
		}

		//		check_point = 8;

		if (lcd_screen == LcdScreenMonCh) {
			//			HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_SET); // sensors on

			if (get_temperatures > 0) {
				get_temperatures = 0;

				if (readTemperatureSensors() != HAL_OK) {
					UsrLog("readTemperatureSensors ERROR");

					//					HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_RESET); // sensors off
				}

//				MX_USB_HOST_Process_FS();
//
//				MX_USB_HOST_Process_HS();
				IWDG_delay_ms(0);

				if ((HAL_GetTick() - adc_update_tick2) >= 500) {
					adc_update_tick2 = HAL_GetTick();

					ADC_get();
				}
				if (MPU6050_acc() != HAL_OK) {
					UsrLog("MPU6050_init");

					HAL_GPIO_WritePin(I2C3_PWR_GPIO_Port, I2C3_PWR_Pin,
							GPIO_PIN_RESET); // I2C3 Power Off

					uint8_t delay_restart = 100;

					while (delay_restart--) {
						IWDG_delay_ms(1);

//						MX_USB_HOST_Process_FS();
//
//						MX_USB_HOST_Process_HS();
						IWDG_delay_ms(0);
					}

					HAL_GPIO_WritePin(I2C3_PWR_GPIO_Port, I2C3_PWR_Pin,
							GPIO_PIN_SET); // I2C3 Power On

					HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

					HAL_TIM_Base_Stop_IT(&htim7);

					MPU6050_init(NO_CALC_ERROR);

					MPU6050_acc();

					HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

					HAL_TIM_Base_Start_IT(&htim7);

					HAL_GPIO_WritePin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin,
							GPIO_PIN_RESET); // USB On
				}
			}
		} else {
			if (HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port, RTC_INT_STATE_Pin)
					== GPIO_PIN_RESET) {
				//HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_RESET); // sensors off
			}
		}

		//		check_point = 9;

		if (USB_FS_restart == TRUE) {
			USB_FS_restart = FALSE;
		}

		if (timer7_tick == TRUE) { // 100ms interval
			timer7_tick = FALSE;

			if ((lcd_refresh > 0) && (lcd_refresh < 0xFF)) {
				if ((lcd_refresh == 2) && (lcd_phase == 99))
					lcd_phase = 0;
				if (lcd_refresh > 1)
					lcd_refresh--;
			}

			int i = 0;

			if (clock1_setup == TRUE) {
				clock1_setup = FALSE;

				i = 0;

				HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_SET); // I2C1 Power On

				while (DS3231_SetDateTime(&clock1) != HAL_OK) // [x] verification added
				{
//					MX_USB_HOST_Process_FS();
//
//					MX_USB_HOST_Process_HS();

					IWDG_delay_ms(0);

					if (i < 9) {
						i++;
					} else {
						UsrLog("DS3231_GetDateTime ERROR");

						HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin,
								GPIO_PIN_RESET); // I2C1 Power Off

						break;
					}

					IWDG_delay_ms(0);
				}

				i = 0;

				while (DS3231_GetDateTime(&tm1) != HAL_OK) // [x] verification added
				{
//					MX_USB_HOST_Process_FS();
//
//					MX_USB_HOST_Process_HS();

					IWDG_delay_ms(0);

					if (i < 9) {
						i++;
					} else {
						UsrLog("DS3231_GetDateTime ERROR");

						HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin,
								GPIO_PIN_RESET); // I2C1 Power Off

						break;
					}

					IWDG_delay_ms(0);
				}

				//HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin,
				//		GPIO_PIN_RESET); // I2C1 Power Off
			}

//			MX_USB_HOST_Process_FS();
//
//			MX_USB_HOST_Process_HS();

			IWDG_delay_ms(0);

		}

		//		check_point = 10;

		if (lcd_refresh == 1) {
			// while (lcd_inter_busy == TRUE) {
			IWDG_delay_ms(0);

//			MX_USB_HOST_Process_FS();
//
//			MX_USB_HOST_Process_HS();
			// }

			lcd_loop_busy = TRUE;

			if (lcd_loop() == TRUE) {
				lcd_refresh--;
			}

			lcd_loop_busy = FALSE;
		}

		//		check_point = 11;

		if (start_MSC_Application1 == 2) {
			start_MSC_Application1 = 0;

			export_data_enabled = 1;

			lcd_refresh = 3;

			export_data_step = 0;

			userFunction_phase = 1;
		}

		if (start_MSC_Application2 == 2) {
			start_MSC_Application2 = 0;

			save_BMP_enabled = 1;

			save_BMP_step = 0;

			save_BMP_phase = 1;
		}

		if (lcd_screen == LcdScreenCfgCh) {
			if (label_index_edit2 != label_index_edit) { // from: stm32f4xx_it.c
				if (ch_status_tmp[ch_config_index] == 0) {
					lcd_config_ch_label_color = ILI9341_BLUE;
				} else {
					lcd_config_ch_label_color = ILI9341_GREEN;
				}

				lcd_config_ch_label(ch_config_index, lcd_config_ch_label_color);
				label_index_edit2 = label_index_edit;
			}
		}

		//		check_point = 12;

//		MX_USB_HOST_Process_FS();
//
//		//		check_point = 13;
//
//		MX_USB_HOST_Process_HS();

		IWDG_delay_ms(0);

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */
	HAL_GPIO_WritePin(GPIOE, LED2_Pin, GPIO_PIN_RESET); // LED2 ON

	if ((HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port, AC_DC_STATE_Pin)
			== GPIO_PIN_SET)
			|| (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_SET)) {
		HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin, GPIO_PIN_SET);

		lcd_enabled = true;

		HAL_GPIO_WritePin(LCD_PWR_GPIO_Port, LCD_PWR_Pin, GPIO_PIN_SET); // LCD Power On

		//		if (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_SET) {
		//			//lcd_screen = LcdScreenGraphic;
		//		}
	} else {
		lcd_enabled = false;

		//UsrLog("System Clock Config 40MHz");

		SystemClock_Config_40MHz();

		HAL_Init();

		/* USER CODE BEGIN Init */
		if (DWT_Delay_Init()) {
			Error_Handler(); /* Call Error Handler */
		}

		/* Initialize all configured peripherals */
		MX_GPIO_Init();
	}
	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void) {

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc2.Init.Resolution = ADC_RESOLUTION_12B;
	hadc2.Init.ScanConvMode = DISABLE;
	hadc2.Init.ContinuousConvMode = DISABLE;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DMAContinuousRequests = DISABLE;
	hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc2) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */

	/* USER CODE END ADC2_Init 2 */

}

/**
 * @brief ADC3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC3_Init(void) {

	/* USER CODE BEGIN ADC3_Init 0 */

	/* USER CODE END ADC3_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC3_Init 1 */

	/* USER CODE END ADC3_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc3.Init.Resolution = ADC_RESOLUTION_12B;
	hadc3.Init.ScanConvMode = DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.NbrOfConversion = 1;
	hadc3.Init.DMAContinuousRequests = DISABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc3) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC3_Init 2 */

	/* USER CODE END ADC3_Init 2 */

}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
 * @brief ETH Initialization Function
 * @param None
 * @retval None
 */
static void MX_ETH_Init(void) {

	/* USER CODE BEGIN ETH_Init 0 */
	return;
	/* USER CODE END ETH_Init 0 */

	static uint8_t MACAddr[6];

	/* USER CODE BEGIN ETH_Init 1 */

	/* USER CODE END ETH_Init 1 */
	heth.Instance = ETH;
	MACAddr[0] = 0x00;
	MACAddr[1] = 0x80;
	MACAddr[2] = 0xE1;
	MACAddr[3] = 0x00;
	MACAddr[4] = 0x00;
	MACAddr[5] = 0x00;
	heth.Init.MACAddr = &MACAddr[0];
	heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
	heth.Init.TxDesc = DMATxDscrTab;
	heth.Init.RxDesc = DMARxDscrTab;
	heth.Init.RxBuffLen = 1524;

	/* USER CODE BEGIN MACADDRESS */

	/* USER CODE END MACADDRESS */

	if (HAL_ETH_Init(&heth) != HAL_OK) {
		Error_Handler();
	}

	memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
	TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM
			| ETH_TX_PACKETS_FEATURES_CRCPAD;
	TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
	TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
	/* USER CODE BEGIN ETH_Init 2 */

	/* USER CODE END ETH_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief I2C3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C3_Init(void) {

	/* USER CODE BEGIN I2C3_Init 0 */

	/* USER CODE END I2C3_Init 0 */

	/* USER CODE BEGIN I2C3_Init 1 */

	/* USER CODE END I2C3_Init 1 */
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = 400000;
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c3.Init.OwnAddress1 = 0;
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c3.Init.OwnAddress2 = 0;
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C3_Init 2 */

	/* USER CODE END I2C3_Init 2 */

}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void) {

	/* USER CODE BEGIN IWDG_Init 0 */
	HAL_IWDG_Refresh(&hiwdg);
	/* USER CODE END IWDG_Init 0 */

	/* USER CODE BEGIN IWDG_Init 1 */

	/* USER CODE END IWDG_Init 1 */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 2047;
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN IWDG_Init 2 */
	HAL_IWDG_Refresh(&hiwdg);
	/* USER CODE END IWDG_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief SPI3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI3_Init(void) {

	/* USER CODE BEGIN SPI3_Init 0 */
	if (lcd_enabled == false) {
		return;
	}
	/* USER CODE END SPI3_Init 0 */

	/* USER CODE BEGIN SPI3_Init 1 */

	/* USER CODE END SPI3_Init 1 */
	/* SPI3 parameter configuration*/
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI3_Init 2 */

	/* USER CODE END SPI3_Init 2 */

}

/**
 * @brief TIM7 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM7_Init(void) {

	/* USER CODE BEGIN TIM7_Init 0 */

	/* USER CODE END TIM7_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM7_Init 1 */

	/* USER CODE END TIM7_Init 1 */
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 16799;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 900;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM7_Init 2 */
	if (lcd_enabled == true) {
		htim7.Init.Prescaler = 1679;
		htim7.Init.Period = 50;

		if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
			Error_Handler();
		}
	}
	/* USER CODE END TIM7_Init 2 */

}

/**
 * @brief TIM8 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM8_Init(void) {

	/* USER CODE BEGIN TIM8_Init 0 */
	if (lcd_enabled == false) {
		return;
	}
	/* USER CODE END TIM8_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM8_Init 1 */

	/* USER CODE END TIM8_Init 1 */
	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 1;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 1023;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;
	htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_OC_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM8_Init 2 */

	/* USER CODE END TIM8_Init 2 */
	HAL_TIM_MspPostInit(&htim8);

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			BUZZER_Pin | DC_DC_ON_Pin | I2C3_PWR_Pin | ETH_PWR_Pin
					| SENSOR_PWR_Pin | LED2_Pin | RTC_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(HC595_OE_GPIO_Port, HC595_OE_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, HC595_LAT_Pin | HC595_DAT_Pin | HC595_CLK_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, USB_HS_PWR_Pin | UC_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD,
			FLASH_WP_Pin | FLASH_PWR_Pin | LCD_PWR_Pin | LCD_RST_Pin
					| LCD_DC_Pin | LCD_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : BUZZER_Pin DC_DC_ON_Pin I2C3_PWR_Pin ETH_PWR_Pin
	 SENSOR_PWR_Pin LED2_Pin RTC_PWR_Pin */
	GPIO_InitStruct.Pin = BUZZER_Pin | DC_DC_ON_Pin | I2C3_PWR_Pin | ETH_PWR_Pin
			| SENSOR_PWR_Pin | LED2_Pin | RTC_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : CHARGER_STATE_Pin */
	GPIO_InitStruct.Pin = CHARGER_STATE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CHARGER_STATE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : RTC_RESET_Pin USB_FS_PWR_Pin */
	GPIO_InitStruct.Pin = RTC_RESET_Pin | USB_FS_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : HC595_OE_Pin HC595_LAT_Pin HC595_DAT_Pin HC595_CLK_Pin */
	GPIO_InitStruct.Pin = HC595_OE_Pin | HC595_LAT_Pin | HC595_DAT_Pin
			| HC595_CLK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : BOOT1_Pin */
	GPIO_InitStruct.Pin = BOOT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SENSOR1_Pin SENSOR2_Pin SENSOR3_Pin SENSOR4_Pin
	 SENSOR5_Pin SENSOR6_Pin SENSOR7_Pin SENSOR8_Pin */
	GPIO_InitStruct.Pin = SENSOR1_Pin | SENSOR2_Pin | SENSOR3_Pin | SENSOR4_Pin
			| SENSOR5_Pin | SENSOR6_Pin | SENSOR7_Pin | SENSOR8_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_HS_PWR_Pin FLASH_PWR_Pin UC_PWR_Pin LCD_PWR_Pin
	 LCD_RST_Pin LCD_DC_Pin */
	GPIO_InitStruct.Pin = USB_HS_PWR_Pin | FLASH_PWR_Pin | UC_PWR_Pin
			| LCD_PWR_Pin | LCD_RST_Pin | LCD_DC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_HS_OC_Pin */
	GPIO_InitStruct.Pin = USB_HS_OC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USB_HS_OC_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : BUTTON1_Pin */
	GPIO_InitStruct.Pin = BUTTON1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(BUTTON1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : RTC_INT_STATE_Pin AC_DC_STATE_Pin */
	GPIO_InitStruct.Pin = RTC_INT_STATE_Pin | AC_DC_STATE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : BUTTON2_Pin */
	GPIO_InitStruct.Pin = BUTTON2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(BUTTON2_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_FS_OC_Pin */
	GPIO_InitStruct.Pin = USB_FS_OC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USB_FS_OC_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : FLASH_CS_Pin */
	GPIO_InitStruct.Pin = FLASH_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(FLASH_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : FLASH_WP_Pin LCD_CS_Pin */
	GPIO_InitStruct.Pin = FLASH_WP_Pin | LCD_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void string2byte(const char *input, uint32_t inputLength, uint8_t *output) {
	while (inputLength--) {
		*output++ = *input++;
	}
}

void byte2string(const uint8_t *input, uint32_t inputLength, char *output) {
	while (inputLength--) {
		*output++ = *input++;
	}
}

uint32_t sizeof_array(const char *input) {
	uint32_t ret = 0;
	while (input[ret] != 0) {
		ret++;
	}

	//UsrLog("ret: %lu", ret);

	return ret;
}

uint32_t USB_OC_Tick = 0;
uint32_t BUTTON1_debounce = 0;

/* Handle PC0 interrupt */
// EXTI Line5 External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == CHARGER_STATE_Pin) // If The INT Source Is EXTI Line0 (PC0 Pin)
	{
		battery_failure_cnt++;

		//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // Toggle The Output (LED) Pin
	} else if (GPIO_Pin == USB_FS_OC_Pin) {
		if (HAL_GPIO_ReadPin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin)
				== GPIO_PIN_RESET) { // ON?
			if (V_DC_DC_val > 4.0) {
				if ((HAL_GetTick() - USB_OC_Tick) >= 250) {
					USB_OC_Tick = HAL_GetTick();

					USB_FS_OC_Error = 1;
					//HAL_GPIO_ReadPin(USB_FS_OC_GPIO_Port,
					//USB_FS_OC_Pin);

					HAL_GPIO_WritePin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin,
							GPIO_PIN_SET); // USB off
				}
			}
		}
	} else if (GPIO_Pin == USB_HS_OC_Pin) {
		if (HAL_GPIO_ReadPin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin) // ON?
		== GPIO_PIN_RESET) {
			if (V_DC_DC_val > 4.0) {
				if ((HAL_GetTick() - USB_OC_Tick) >= 250) {
					USB_OC_Tick = HAL_GetTick();

					USB_HS_OC_Error = 1;
					// HAL_GPIO_ReadPin(USB_HS_OC_GPIO_Port,
					// USB_HS_OC_Pin);

					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_SET); // USB off

					//beep(1);
				}
			}
		}
	} else if (GPIO_Pin == BUTTON1_Pin) {
//		if ((HAL_GetTick() - EXTI_tick250ms) >= 250) {
//			EXTI_tick250ms = HAL_GetTick();

		if (button1_restart_enabled == 1) {
			if (lcd_screen == LcdScreenUsbExport) {
				if (USB_HS_OC_Error == 1) {
					HAL_GPIO_WritePin(USB_HS_PWR_GPIO_Port, USB_HS_PWR_Pin,
							GPIO_PIN_RESET); // USB on
				}
			}

			if (USB_FS_OC_Error == 1) {
				HAL_GPIO_WritePin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin,
						GPIO_PIN_RESET); // USB on
			}

			USB_FS_OC_Error = 0;
			USB_HS_OC_Error = 0;

			if ((HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin)
					== GPIO_PIN_SET) && (button_old == 0)) {
				button_old = 1;

				if ((HAL_GetTick() - BUTTON1_debounce) >= 750) {
					BUTTON1_debounce = HAL_GetTick();

					beep(1, 0);

					if (button1_stage < 2) {
						button1_stage++;
					} else {
						button1_stage = 0;
					}

					if (button1_stage == ScreenActive) {
						button1_flag = 1;
						lcd_bright_set = lcd_bright_max;

						if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port,
						AC_DC_STATE_Pin) == GPIO_PIN_SET) {
							lcd_timeout0 = lcd_timeout0recall2;
						} else {
							lcd_timeout0 = lcd_timeout0recall;
						}
					} else if (button1_stage == ScreenDimm) {
						lcd_bright_set = lcd_bright_min;

						lcd_timeout0 = 0;
					} else if (button1_stage == ScreenOff) {
						//button1_info = 1;
						lcd_bright_set = lcd_bright_off;

						lcd_timeout0 = 0;
					}
				}
			} else if ((HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin)
					== GPIO_PIN_RESET) && (button_old == 1)) {
				//button1_info = 0;

				button1_flag = 0;

				button_old = 0;
			}

		}
//		}
	}
}

void ADC_get(void) {
	get_ADC_Vref();

	AD1_RES_ADC_DC_DC_5V = 0;
	AD2_RES_ADC_CHARGER = 0;
	AD3_RES_ADC_VBAT = 0;

	// Set ADC Channel 4 (PA4)
	hadc1.Instance->SQR3 = ADC_CHANNEL_4;

	// Set ADC Channel 3 (PA3)
	hadc2.Instance->SQR3 = ADC_CHANNEL_3;

	// Set ADC Channel 0 (PA0)
	hadc3.Instance->SQR3 = ADC_CHANNEL_0;

	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 5; i++) {
			// Start ADC Conversion
			HAL_ADC_Start(&hadc1);
			HAL_ADC_Start(&hadc2);
			HAL_ADC_Start(&hadc3);
			// Poll ADC1 Perihperal & TimeOut = 1mSec
			HAL_ADC_PollForConversion(&hadc1, 1);
			HAL_ADC_PollForConversion(&hadc2, 1);
			HAL_ADC_PollForConversion(&hadc3, 1);
			// Read The ADC Conversion Result & Map It To PWM DutyCycle
			AD1_RES_ADC_DC_DC_5V += HAL_ADC_GetValue(&hadc1);
			AD2_RES_ADC_CHARGER += HAL_ADC_GetValue(&hadc2);
			AD3_RES_ADC_VBAT += HAL_ADC_GetValue(&hadc3);

		}

		IWDG_delay_ms(1);
	}

	float f_adc1 = 0, f_adc2 = 0, f_adc3 = 0; //, f_vref = 0;

	f_adc1 = AD1_RES_ADC_DC_DC_5V / 50;
	f_adc2 = AD2_RES_ADC_CHARGER / 50;
	f_adc3 = AD3_RES_ADC_VBAT / 50;

//	f_adc1: 3514.000000
//	f_adc2: 782.000000
//	f_adc3: 2644.000000

//	UsrLog("f_adc1: %04f", f_adc1);
//	UsrLog("f_adc2: %04f", f_adc2);
//	UsrLog("f_adc3: %04f", f_adc3);

	// (3.3 / 4096) = 0,000805664
	//
	// R1=10k; R2=10k
	// 3.3V=10k; 20k=2*3.3
	// 0,000805664 * 2 = 0,001611328
	// 0,001611328 * 0,916996047 = 0,001477581
	//
	// R1=15k; R2=1k
	// 3.3V=1k; 16k=16*3.3
	// 0,000805664 * 16 = 0,012890624
	// 0,012890624 * 0,916996047 = 0,011820651

	f_adc1 = f_adc1 * const_adc_x2;
	f_adc2 = f_adc2 * const_adc_x16;
	f_adc3 = f_adc3 * const_adc_x2;

	V_DC_DC_val = f_adc1;
	V_Charger_val = f_adc2;
	// V_Bat_val = f_adc3;

	if (V_DC_DC_val > 5.5) {
		HAL_GPIO_WritePin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin, GPIO_PIN_RESET); // DC-DC Off

		UsrLog("ERROR: Over voltage at DC-DC (>5.5V)");
		UsrLog("V_DC_DC_val (0 or 5V): %.3f V", V_DC_DC_val);

		if (init_flag == 1) {
			//while (1) {
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // BUZZER On
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off

			IWDG_delay_ms(2000);

			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // BUZZER Off
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On

			IWDG_delay_ms(2000);
			//}
		} else {
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // BUZZER On
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off

			IWDG_delay_ms(250);

			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // BUZZER Off
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On

//			IWDG_delay_ms(2000);
		}
	} else if (V_DC_DC_val < 4.0) {
		if (HAL_GPIO_ReadPin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin)
				== GPIO_PIN_SET) {

			DC_DC_Error = 1;

			HAL_GPIO_WritePin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin, GPIO_PIN_RESET); // DC-DC Off
		}
	}

	if (init_flag == 0) {
		V_Bat_val = f_adc3;
	} else {
		float V_Bat_val_tmp = f_adc3;

		V_Bat_val_tmp -= 3.5;

		V_Bat_val_tmp /= 0.004;

		if (V_Bat_val_tmp > 255) {
			V_Bat_val_tmp = 255;
		}

		V_Bat_val_old[V_Bat_val_index] = V_Bat_val_tmp;

		if (V_Bat_val_index < (V_Bat_val_index_max - 1)) {
			V_Bat_val_index++;
		} else {
			V_Bat_val_index = 0;
		}

		float V_Bat_val_tmp2 = 0;

		V_Bat_val_tmp = 0;

		for (int i = 0; i < (V_Bat_val_index_max - 1); i++) {
			V_Bat_val_tmp2 = V_Bat_val_old[i];
			V_Bat_val_tmp2 *= 0.004;
			V_Bat_val_tmp2 += 3.5;
			V_Bat_val_tmp += V_Bat_val_tmp2;
		}

		V_Bat_val_tmp /= V_Bat_val_index_max;

		V_Bat_val = V_Bat_val_tmp;
	}

	float batt_percent = 0.0;
	//float V_Bat_val_temp = 0.0;

	if (V_Bat_val > 3.5) {
		// V_Bat_val_temp = (V_Bat_val - 3.5) * 100; // 100% / 4

		// 3.5V ~ 3.9V = 0 ~ 100%

		if (battery_status == Battery_in_use) {
			if (V_Bat_val <= 3.75) {
				// 0.25: 100%
				// y:      x%
				// 0:      0%
				// x%: (y*100%)/0.25

				batt_percent = V_Bat_val; //map(V_Bat_val - 3.5, 0.0, 0.25, 0.0, 99.9);
				batt_percent -= 3.5;
				batt_percent *= 100.0;
				batt_percent /= 0.25;

			} else {
				batt_percent = 99.9;
			}
		} else {
			batt_percent = (V_Bat_val - 3.5) * 285.7; // 100% / 0.35V
		}

		if (batt_percent < 0.0) {
			batt_percent = 0.0;
		} else if (batt_percent > 99.9) {
			batt_percent = 99.9;
		}

		if (battery_status == Battery_in_use) {
			float batt_val10_tmp = (batt_val10 * 10) + batt_val1;

			if (batt_percent > batt_val10_tmp) {
				batt_val10 = batt_percent / 10.0;
				batt_val1 = batt_percent - (batt_val10 * 10.0);
			}
		} else {
			batt_val10 = batt_percent / 10.0;
			batt_val1 = batt_percent - (batt_val10 * 10.0);
		}
	} else {
		batt_val10 = 0;
		batt_val1 = 0;
	}

	//UsrLog("ADC1 %0.1fV, ADC2 %0.1fV, ADC3 %0.1fV", f_adc1, f_adc2, f_adc3);
	//UsrLog("TP4056 LED CHRG freq: %lu", TP4056_charger_freq);
}

void get_ADC_Vref(void) {
	// Set ADC Channel 6 (PA6)
	hadc1.Instance->SQR3 = ADC_CHANNEL_6;

	AD1_RES_ADC_VREF = 0;

	for (int i = 0; i < 25; i++) {
		// Start ADC Conversion
		HAL_ADC_Start(&hadc1);
		// Poll ADC1 Perihperal & TimeOut = 1mSec
		HAL_ADC_PollForConversion(&hadc1, 1);
		// Read The ADC Conversion Result & Map It To PWM DutyCycle
		AD1_RES_ADC_VREF += HAL_ADC_GetValue(&hadc1);

		IWDG_delay_ms(1);
	}

	// Set ADC Channel 4 (PA4)
	hadc1.Instance->SQR3 = ADC_CHANNEL_4;

	AD1_RES_ADC_VREF /= 25;

	//UsrLog("AD1_RES_ADC_VREF: %lu", AD1_RES_ADC_VREF);

	float f_vref = AD1_RES_ADC_VREF;

	f_vref = 2.495 / f_vref;

	//UsrLog("f_vref: %04f", f_vref);

	// (3.3 / 4096) = 0,000805664
	// R1=10k; R2=10k
	// 3.3V=10k; 20k=2*3.3
	// 0,000805664 * 2 = 0,001611328
	// 0,001611328 * 0,916996047 = 0,001477581
	// R1=15k; R2=1k
	// 3.3V=1k; 16k=16*3.3
	// 0,000805664 * 16 = 0,012890624
	// 0,012890624 * 0,916996047 = 0,011820651

	const_adc_x2 = (f_vref * 2);
	const_adc_x16 = (f_vref * 16);
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

void UART_Printf(const char *fmt, ...) {
	char buff[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, sizeof(buff), fmt, args);
	HAL_UART_Transmit(&huart1, (uint8_t*) buff, strlen(buff), 100); // HAL_MAX_DELAY
	va_end(args);
}

static void usart_print_main_menu(void) {
	UsrLog("~$ Menu options:");
	UsrLog("~$ 1: Configure CH1");
	UsrLog("~$ 2: Configure CH2");
	UsrLog("~$ 3: Configure CH3");
	UsrLog("~$ 4: Configure CH4");
	UsrLog("~$ 5: Configure CH5");
	UsrLog("~$ 6: Configure CH6");
	UsrLog("~$ 7: Configure CH7");
	UsrLog("~$ 8: Configure CH8");
	UsrLog("~$ E: Export data");
	UsrLog("~$ P: Configure password");
	UsrLog("~$ S: Sensors monitor");
	UsrLog("~$ T: Time clock adjust");
	UsrLog("~$ V: Memory test");
	UsrLog("~$");
	UsrLog("~$ Esc: Logout");
	UsrLog("~$");
}

static void usart_print_config_ch(uint8_t ch) {
	UsrLog("~$ Configure CH%d", ch);
	UsrLog("~$");
	UsrLog("~$ Menu:");
	UsrLog("~$ 1: Change Record Data State");
	UsrLog("~$ 2: Back to main Menu");
	UsrLog("~$");
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//    if (huart == huart1)
//    {

//	char uart_txt[BUFFER_SIZE_UART];

//	memcpy(uart_txt, debug_rx_buffer, BUFFER_SIZE_UART);

//	if ((debug_rx_buffer[0] >= 32) && (debug_rx_buffer[0] <= 126)) {
//		UsrLog("%c", debug_rx_buffer[0]);
//	} else {
//		UsrLog("%02X", debug_rx_buffer[0]);
//	}

//	UsrLog ("HAL_UART_RxCpltCallback data received: %d", debug_rx_buffer[1]);
//	UsrLog ("HAL_UART_RxCpltCallback data received: %d", debug_rx_buffer[2]);

	//    }
//    else if (huart == rs485_uart())
//    {
//        memcpy(debug_tx_buffer, rs485_rx_buffer, BUFFER_SIZE);
//        debug_tx(debug_tx_buffer, BUFFER_SIZE);
//
//        HAL_UART_Receive_DMA(rs485_uart(), rs485_rx_buffer, BUFFER_SIZE);
//    }

	uint8_t key = debug_rx_buffer[0];

	if (usart_login == 0) {
		if (usart_login_pw_index == 0) {
			UsrLog("~$ Login password:");

			usart_login_pw_index++;
		} else if (usart_login_pw_index > 6) {
			usart_login_pw_index = 0;
			UsrLog("~$ Password error!");
			UsrLog("~$ Login password:");
		} else {
			if (password_code[usart_login_pw_index - 1] == key) {
				usart_login_pw_index_cmp++;

				if (usart_login_pw_index_cmp == 6) {
					UsrLog("~$ Welcome!");
					UsrLog("~$");

					usart_print_main_menu();

					usart_login = 1;

					usart_menu = 0;
//				} else {
//					UsrLog("usart_login_pw_index_cmp: %d", usart_login_pw_index_cmp);
				}
			} else {
				usart_login_pw_index_cmp = 0;
			}
			usart_login_pw_index++;
		}
	} else {
		if (usart_menu == 0) {
			if (key == 0x1B) { // ESC key
				usart_login = 0;
				usart_login_pw_index = 1;
				usart_login_pw_index_cmp = 0;

				UsrLog("~$ Logout");
				UsrLog("~$ Login password:");
			} else if ((key >= 49) && ((key <= 56))) { // 1: Configure CH1 ~ CH8
				usart_print_config_ch(key - 48);
				usart_menu = key - 48;
			} else {
				UsrLog("Unknown option: 0x%02X", key);
			}
		} else if ((usart_menu >= 1) && (usart_menu <= 8)) {
			if (key == 49) {
				UsrLog("~$ Configure CH%d", usart_menu);
			} else if (key == 50) {
				usart_print_main_menu();
				usart_menu = 0;
			} else {
				UsrLog("Unknown option: 0x%02X", key);
			}
		} else {
			UsrLog("Unknown option: 0x%02X", key);
		}
	}

//	password_code

	HAL_UART_Receive_IT(&huart1, debug_rx_buffer, BUFFER_SIZE_UART);
}

// https://forum.arduino.cc/t/how-map-loses-precision-and-how-to-fix-it/371026/4
float map(float x, float in_min, float in_max, float out_min, float out_max) {
//	return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1)
//			+ out_min;
	float out_range;
	float in_range;

	out_range = out_max - out_min;
	if (out_range > 0)
		++out_range;
	else if (out_range < 0)
		--out_range;
	else
		return (0);

	in_range = in_max - in_min;
	if (in_range > 0)
		++in_range;
	else if (in_range < 0)
		--in_range;
	else
		// Is actually infinity but long has no such thing.  The least negative long is another choice.
		return (0);

	return (((x - in_min) * (out_range)) / (in_range)) + out_min;
}

HAL_StatusTypeDef DS3231_Setup(void) {
	//	DS3231_Alarm(ALARM_1); // clear the alarm flag
	//
	//	DS3231_Alarm(ALARM_2); // clear the alarm flag

	if (DS3231_SquareWave(SQWAVE_NONE) != HAL_OK) {
		UsrLog("DS3231_SquareWave");
		return HAL_ERROR;
	} else if (DS3231_SetAlarmSec(ALM2_EVERY_MINUTE, 0, 0, 0, 0) != HAL_OK) // set Alarm 2 to occur after every minute
			{
		UsrLog("DS3231_SetAlarmSec");
		return HAL_ERROR;
	} else if (DS3231_AlarmInterrupt(ALARM_1, ALARME1_ENABLE) != HAL_OK) // clear the alarm flag
			{
		UsrLog("DS3231_AlarmInterrupt 1");
		return HAL_ERROR;
	} else if (DS3231_AlarmInterrupt(ALARM_2, ALARME2_ENABLE) != HAL_OK) // clear the alarm flag
			{
		UsrLog("DS3231_AlarmInterrupt 2");
		return HAL_ERROR;
	} else {
		return HAL_OK;
	}
}

void print_temperatures(void) {
	for (int i = 0; i < maxChannel; i++) {
		uint8_t sensor_ID[maxChannel];

		if ((SENSORS[i].FLAG_ERROR) || (SENSORS[i].FLAG_ID_ERROR)) {
			//UsrLog("Sensor %d: ERROR (Not Found?)", i + 1);
		} else if ((SENSORS[i].FLAG_ENABLED == 1)
				&& ((ch_config_index != i)
						|| (lcd_screen != LcdScreenCfgChMemErase))) {
			for (int j = 0; j < maxChannel; j++) {
				sensor_ID[j] = SENSORS[i].SERIAL_ID[j];
			}

			DS18B20_hex_id(sensor_ID, str);

			UsrLog("Sensor %d Temper: %.4f*C ID: %s", i + 1,
					SENSORS[i].TEMPERATURE, str);
		} else {
			UsrLog("Sensor %d: Disabled", i + 1);
		}
	}
}

void Sleep_wakeup_by_Timer(void) {
	HAL_ADC_DeInit(&hadc1);

	HAL_I2C_DeInit(&hi2c3); // MPU6050

	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

	HAL_SuspendTick();

	HAL_TIM_Base_Start_IT(&htim7); // for wake up

//	UsrLog("Sleep");
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI); // wait for wake up from timer7 interrupt

//	UsrLog("Wake Up");
	HAL_ResumeTick();

	HAL_TIM_Base_Stop_IT(&htim7);
}

void beep(uint8_t repetions, uint8_t is_error) {
#if DISABLE_BEEP_FOR_DEBUG == 0
	IWDG_delay_ms(0);

	while (repetions--) {
		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // BUZZER On
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); // LED2 Off

		if ((is_error != 0) || (init_flag == 0)) {
			IWDG_delay_ms(100);
		} else {
			IWDG_delay_ms(5);
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // BUZZER Off
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On
		}

		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // BUZZER Off
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // LED2 On

		if (repetions > 0) {
			IWDG_delay_ms(25);
		}
	}
#endif // #if DISABLE_BEEP_FOR_DEBUG == 1
}

HAL_StatusTypeDef readTemperatureSensors(void) {
	uint8_t error_status_ID = 0;
	uint8_t error_status_PRESENT = 0;

	//UsrLog("DS18B20_readSensors");

//	for (int i = 0; i < maxChannel; i++) {
//		SENSORS[i].FLAG_TIMEOUT = 0;
//	}

	if (DS18B20_readSensors(DS18B20_READ, SENSORS) != HAL_OK) {
		UsrLog("DS18B20_READ ERROR");

		return HAL_ERROR;
	}

	if (DS18B20_readSensors(DS18B20_START_CONVERSION, SENSORS) != HAL_OK) {
		UsrLog("DS18B20_START_CONVERSION ERROR");

		return HAL_ERROR;
	}

	for (int i = 0; i < maxChannel; i++) {
		if (SENSORS[i].FLAG_TIMEOUT == 1) { // sensor error
			SENSORS[i].FLAG_TIMEOUT = 0;

			SENSORS[i].FLAG_ERROR = 1;

			if (SENSORS[i].FLAG_ID_ERROR == 1) {
				error_status_ID = 1;

//				UsrLog("ERROR: readTemperatureSensors ID_ERROR CH%d", i + 1);
			}

		} else if ((SENSORS[i].FLAG_ENABLED == 1)
				&& (SENSORS[i].FLAG_TIMEOUT == 0)) {
			SENSORS[i].FLAG_ERROR = 0;
		}

		if ((SENSORS[i].FLAG_ENABLED == 1) && (SENSORS[i].FLAG_PRESENT == 0)) {
			error_status_PRESENT = 1;

//			UsrLog("ERROR: readTemperatureSensors NOT PRESENT CH%d", i + 1);
		}
	}

//	if (error_status_PRESENT == 1) {
////		UsrLog("ERROR: readTemperatureSensors NOT PRESENT");
//	}
//
//	if (error_status_ID == 1) {
////		UsrLog("ERROR: readTemperatureSensors ID_ERROR");
//	}

	if ((error_status_PRESENT == 1) || (error_status_ID == 1)) {
//		beep(1, 1);

		return HAL_ERROR;
	} else {
		return HAL_OK;
	}
}

void DS3231_init(void) {
	uint8_t OscStopped = { 0 };

	uint8_t i = 0;

	HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin, GPIO_PIN_SET);

	IWDG_delay_ms(250);

	HAL_GPIO_WritePin(RTC_RESET_GPIO_Port, RTC_RESET_Pin, GPIO_PIN_RESET);

	IWDG_delay_ms(250);

	while (DS3231_OscStopped(NoClearOSF, &OscStopped) != HAL_OK) {
		if (i < 9) {
			i++;
		} else {
			break;
		}
		IWDG_delay_ms(0);
	}

	if (OscStopped == 1) {
		UsrLog("DS3231_OscStopped");

		ILI9341_WriteString(10, 50, "Bateria baixa!", Font_16x26, ILI9341_RED,
		ILI9341_BLACK);
		ILI9341_WriteString(10, 100, "RTC: parado.", Font_16x26, ILI9341_RED,
		ILI9341_BLACK);

		IWDG_delay_ms(2000);

		ILI9341_WriteString(10, 150, "RTC: reiniciar...", Font_16x26,
		ILI9341_RED, ILI9341_BLACK);

		OscStopped = 0;

		i = 0;

		while (OscStopped == 1) {
			if (DS3231_OscStopped(ClearOSF, &OscStopped) != HAL_OK) {
				UsrLog("DS3231 ERROR");
			}

			UsrLog("DS3231_OscStopped");
			IWDG_delay_ms(200);

			if (i < 9) {
				i++;
			} else {
				break;
			}
			IWDG_delay_ms(0);
		}

		ILI9341_FillScreen(ILI9341_BLACK);
	}

	if (DS3231_GetDateTime(&tm1) == HAL_OK) {
//		UsrLog("DS3231_GetDateTime Ok");
		UsrLog("RTC DateTime: %02d/%02d/%02d %02d:%02d", tm1.date, tm1.month,
				tm1.year, tm1.hours, tm1.minutes);
	} else {
		i = 0;

		while (DS3231_GetDateTime(&tm1) != HAL_OK) {
			UsrLog("DS3231: Error");

			if (i < 9) {
				i++;
			} else {
				break;
			}
			IWDG_delay_ms(0);
		}
	}

	if (DS3231_Setup() == HAL_OK) {
//		UsrLog("DS3231_Setup Ok");
	} else {
		i = 0;

		while (DS3231_Setup() != HAL_OK) {
			UsrLog("DS3231: Error");

			if (i < 99) {
				i++;
			} else {
				break;
			}
			IWDG_delay_ms(0);
		}
	}
}

uint32_t Crc32(uint32_t Crc, uint32_t Data) {
	uint8_t index;
#ifdef VERBOSE_PRINTF_CRC32
	printf("\n\rPREPARATION\tINIT_CRC=0x%08X\tDATA=0x%08X\t", Crc, Data);
#endif
	Crc = Crc ^ Data;
#ifdef VERBOSE_PRINTF_CRC32
	printf("NEW CRC=0x%8X\n\r", Crc);
#endif

	for (index = 0; index < 32; index++) {
#ifdef VERBOSE_PRINTF_CRC32
		printf("Index=%2d\tINIT_CRC=0x%08X\tPOLY=0x04C11DB7\t",index,Crc);
#endif
		if (Crc & 0x80000000)
			Crc = (Crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
		else
			Crc = (Crc << 1);
#ifdef VERBOSE_PRINTF_CRC32
		printf("NEW CRC=0x%08X\n\r",Crc);
#endif
	}
	return (Crc);
}

void load_settings(void) {
	ConfigStruct_TypeDef conf_dat;

	W25Q64_read_config(&conf_dat);

	for (int i = 0; i < 6; i++) {
		password_code[i] = conf_dat.PW_Code[i];
	}

	if ((password_code[0] < 48) || (password_code[0] > 57)
			|| (password_code[1] < 48) || (password_code[1] > 57)
			|| (password_code[2] < 48) || (password_code[2] > 57)
			|| (password_code[3] < 48) || (password_code[3] > 57)
			|| (password_code[4] < 48) || (password_code[4] > 57)
			|| (password_code[5] < 48) || (password_code[5] > 57)) {

		for (int i = 0; i < 6; i++) {
			password_code[i] = '1';
		}
	}

	for (int i = 0; i < 8; i++) {
		sensor_offset[i] = conf_dat.SensorOffset[i];

		if (sensor_offset[i] > 300.0) {
			sensor_offset[i] = 150.0;
		}

		sensor_offset[i] -= 150.0; // 2456 - 1500 = 956
		// sensor_offset[i] /= 10.0; // 956 / 100 = 9.56

//		int32_t tmp = sensor_offset[i] * 10.0;
//
//		sensor_offset[i] = tmp / 10.0;
	}

	if (conf_dat.Language > lang_ptBr) {
		language_current = lang_en;
	} else {
		language_current = conf_dat.Language;
	}

	if (conf_dat.DecimalSeparator > sepComma) {
		decimal_separator = sepDot;
	} else {
		decimal_separator = conf_dat.DecimalSeparator;
	}

	if (conf_dat.DateFormat > ddmmyy) {
		date_format = mmddyy;
	} else {
		date_format = conf_dat.DateFormat;
	}

	if (conf_dat.FileType > datFile64) {
		file_type = datFile64;
	} else {
		file_type = conf_dat.FileType;
	}

	if (conf_dat.FileCntCsv > 999) {
		file_cnt_csv = 0;
	} else {
		file_cnt_csv = conf_dat.FileCntCsv;
	}

	if (conf_dat.FileCntDat > 999) {
		file_cnt_dat = 0;
	} else {
		file_cnt_dat = conf_dat.FileCntDat;
	}

	if (conf_dat.TemperatureUnit > TemperatureUnit_C) {
		temperature_unit = TemperatureUnit_F;
	} else {
		temperature_unit = conf_dat.TemperatureUnit;
	}

	for (int i = 0; i < 8; i++) { // maxChannel
		ch_exported[i] = conf_dat.ChExported[i];
	}
}

void main_init(void) {
	SENSORS[0].FLAG_TIMEOUT = 1;

	HAL_GPIO_WritePin(I2C3_PWR_GPIO_Port, I2C3_PWR_Pin, GPIO_PIN_SET); // I2C3 Power On

	HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_SET); // I2C Power On

	HAL_GPIO_WritePin(SENSOR_PWR_GPIO_Port, SENSOR_PWR_Pin, GPIO_PIN_SET); // sensors on

	if (DS18B20_init() != HAL_OK) {
		UsrLog("ERROR: DS18B20_init");

		beep(1, 1);
	}

	//DS18B20_start_conversion(SENSORS, NumOfSensors);
	DS18B20_readSensors(DS18B20_START_CONVERSION, SENSORS);

	IWDG_delay_ms(0);

	if (MPU6050_reset() != HAL_OK) {
		UsrLog("ERROR: MPU6050_reset");

		beep(1, 1);
	}

	IWDG_delay_ms(0);

	if (STM32F4_getUID(uC_ID) != HAL_OK) {
		UsrLog("ERROR: STM32F4_getUID");

		beep(1, 1);
	}

	UsrLog("uC UID:  %08lX %08lX %08lX", uC_ID[0], uC_ID[1], uC_ID[2]);

	if (W25Q64_init() != HAL_OK) {
		UsrLog("ERROR: W25Q64_init");

		beep(1, 1);
	}

	IWDG_delay_ms(0);

	load_settings();

	if (lcd_enabled == TRUE) { // --> check inside ADC_Init routine
		lcd_show_logo = 1;

		HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin, GPIO_PIN_SET); // uC Power On

//		IWDG_delay_ms(500);
		uint32_t tick1 = HAL_GetTick();

		while ((HAL_GetTick() - tick1) >= 250) {
			tick1 = HAL_GetTick();
		}

		HAL_NVIC_DisableIRQ(EXTI0_IRQn);
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

		HAL_TIM_Base_Stop_IT(&htim7);

		ILI9341_Unselect();
		IWDG_delay_ms(5);
		ILI9341_Init();

		ILI9341_Unselect();
		IWDG_delay_ms(5);
		ILI9341_Init();

		MX_SPI3_Init2();

		IWDG_delay_ms(5);

		ILI9341_FillScreen(ILI9341_BLACK);

		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

		HAL_TIM_Base_Start_IT(&htim7);

		if (FLASH_Error == TRUE) {
			ILI9341_WriteString(0, 0, "Flash Error", Font_16x26, ILI9341_RED,
			ILI9341_BLACK);

			IWDG_delay_ms(2000);

			ILI9341_FillScreen(ILI9341_BLACK);
		}
	} else {
		HAL_GPIO_WritePin(LCD_PWR_GPIO_Port, LCD_PWR_Pin, GPIO_PIN_RESET); // LCD Power Off
	}

	IWDG_delay_ms(0);

	ADC_get();

	UsrLog("V_Charger_val (9 to 24 VDC): %.3f V", V_Charger_val);
	UsrLog("V_Bat_val (3.5 to 4.2 VDC): %.3f V", V_Bat_val);
	UsrLog("V_DC_DC_val (0 or 5V): %.3f V", V_DC_DC_val);

	if (V_Bat_val < 3.5) {
		UsrLog("ERROR: Low battery");

		beep(1, 1);
	}

	if (V_Charger_val > 30.0) {
		UsrLog("ERROR: Charger has high voltage");

		beep(1, 1);
	} else if (V_Charger_val < 8.75) {
		if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port, AC_DC_STATE_Pin)
				== GPIO_PIN_SET) {
			UsrLog("ERROR: Charger has low voltage");

			beep(1, 1);
		}
	}

	if (V_DC_DC_val > 4.2) {
		UsrLog("ERROR: Could not disable DC-DC");

		beep(1, 1);
	}

	DS3231_init();

//	UsrLog("ADC_DeInit");

	IWDG_delay_ms(0);

	if (lcd_enabled == TRUE) {
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

		HAL_TIM_Base_Start_IT(&htim7);

		if (lcd_screen != LcdScreenBattChr) {
			lcd_bright_set = lcd_bright_off;
		} else {
			lcd_bright_set = lcd_bright_med;
		}
	} else {
		for (int i = 0; i < maxChannel; i++) {
			W25Q64_read_header(i, &data_header);

			SENSORS[i].FLAG_ENABLED = data_header.FLAG_ENABLED;
		}

		if (MPU6050_init(NO_CALC_ERROR) != HAL_OK) {
			UsrLog("ERROR: MPU6050_init");

			beep(1, 1);
		}

		if (MPU6050_acc() != HAL_OK) {
			UsrLog("ERROR: MPU6050_acc");

			beep(1, 1);
		}

		HAL_GPIO_WritePin(I2C3_PWR_GPIO_Port, I2C3_PWR_Pin, GPIO_PIN_RESET); // I2C3 Power Off

		UsrLog("Acc X:%d; Y:%d", MPU6050_AccAngleX, MPU6050_AccAngleY);

		Sleep_wakeup_by_Timer(); // wait a while for DS18B20 conversion at 12 bits (750ms)

		//IWDG_delay_ms(1000);

		//if (DS18B20_read_all(SENSORS, NumOfSensors) != HAL_OK) {
		if (DS18B20_readSensors(DS18B20_READ, SENSORS) != HAL_OK) {
			uint8_t beep_error = 0;

			for (uint8_t i = 0; i < maxChannel; i++) {
				if ((SENSORS[i].FLAG_ENABLED == 1)
						& (SENSORS[i].FLAG_TIMEOUT == 1)) // sensor error
						{
					beep_error = 1;
				}
			}

			if (beep_error == 1) // sensor error
					{
				//UsrLog("Beep");
				UsrLog("ERROR: DS18B20_readSensors");

				beep(1, 1);
			}
		}

		IWDG_delay_ms(0);

		print_temperatures();

		for (int i = 0; i < maxChannel; i++) {
			if (SENSORS[i].FLAG_ENABLED == 1) {
				HAL_StatusTypeDef result_write_mark = HAL_BUSY;

				write_mark_steps = W25Q64_read_mark_preset(i);

				while (result_write_mark == HAL_BUSY) {
					result_write_mark = W25Q64_write_mark(i);

					if (result_write_mark == HAL_ERROR) {
						UsrLog("W25Q64 write mark %d: error", i);
					}
				}

				read_mark_steps = W25Q64_read_mark_preset(i);

				HAL_StatusTypeDef result_read_mark = HAL_BUSY;

				while (result_read_mark == HAL_BUSY) {
					result_read_mark = W25Q64_read_mark(i, ch_mark_cnt);

					if (result_read_mark == HAL_ERROR) {
						UsrLog("B) W25Q64 read mark %d: error", i);
						//} else if (result_read_mark == HAL_OK) {
						//UsrLog("B) W25Q64 ch%d mark: %lu", i, ch_mark_cnt[i]);
					}
				}

				if (SENSORS[i].FLAG_ENABLED == 1) {
					if (ch_mark_cnt[i] >= Max_Record_Data) { // 54541
						ch_config_stop(i);

//						UsrLog("B) ch %d config stop", i);
					}
				}

				mem_data_buffer.Temperature[0] =
						(uint8_t) (SENSORS[i].TEMPERATURE_RAW & 0xFF);

				mem_data_buffer.Temperature[1] =
						(uint8_t) ((SENSORS[i].TEMPERATURE_RAW >> 8) & 0xFF);

				for (int j = 0; j < maxChannel; j++) {
					mem_data_buffer.Sensor_Id[j] = SENSORS[i].SERIAL_ID[j];
				}

				mem_data_buffer.AccX = MPU6050_AccAngleX + 127;

				mem_data_buffer.AccY = MPU6050_AccAngleY + 127;

				float V_Bat_val_tmp = V_Bat_val;

				V_Bat_val_tmp -= 3.5;

				V_Bat_val_tmp /= 0.004;

				if (V_Bat_val_tmp > 255)
					V_Bat_val_tmp = 255;

				mem_data_buffer.BattV = V_Bat_val_tmp;

				float V_Charger_val_ptm = V_Charger_val;

				V_Charger_val_ptm -= 7.0;

				V_Charger_val_ptm /= 0.1;

				if (V_Charger_val_ptm > 255)
					V_Charger_val_ptm = 255;

				mem_data_buffer.ChargerV = V_Charger_val_ptm;

				mem_data_buffer.Date[0] = tm1.date;
				mem_data_buffer.Date[1] = tm1.month;
				mem_data_buffer.Date[2] = tm1.year;

				mem_data_buffer.Time[0] = tm1.hours;
				mem_data_buffer.Time[1] = tm1.minutes;

				write_data_steps = W25Q64_read_mark_preset(i);

				HAL_StatusTypeDef result_write_data = HAL_BUSY;

				//UsrLog("lcd off) write data ch:%d addr:0x%05lX temp:%.4f", i,
//						ch_mark_cnt[i], SENSORS[i].TEMPERATURE);

				while (result_write_data == HAL_BUSY) {
					result_write_data = W25Q64_write_data(i, ch_mark_cnt[i] - 1,
							&mem_data_buffer);
				}
			}
		}

		UsrLog("Power Off");

		uint8_t alarmFlagBit[1];

		for (uint8_t i = 0; i < 2; i++) {
			if (DS3231_Alarm(ALARM_1, alarmFlagBit) != HAL_OK) {
				UsrLog("DS3231_Alarm 1 ERROR");
			}
		}

		while (HAL_GPIO_ReadPin(RTC_INT_STATE_GPIO_Port, RTC_INT_STATE_Pin)
				== GPIO_PIN_SET) {
			uint8_t i = 0;

			while (alarmFlagBit[0] == 1) {
				if (DS3231_Alarm(ALARM_1, alarmFlagBit) != HAL_OK) {
					UsrLog("DS3231_Alarm 1 ERROR");
				}

				UsrLog("DS3231_Alarm ALARM_1: error");

				if (i < 9)
					i++;
				else
					break;

				IWDG_delay_ms(0);
			}

			for (i = 0; i < 2; i++) {
				if (DS3231_Alarm(ALARM_2, alarmFlagBit) != HAL_OK) {
					UsrLog("DS3231_Alarm 2 ERROR");
				}
			}

			i = 0;

			while (alarmFlagBit[0] == 1) {
				if (DS3231_Alarm(ALARM_2, alarmFlagBit) != HAL_OK) {
					UsrLog("DS3231_Alarm 2 ERROR");
				}

				UsrLog("DS3231_Alarm ALARM_2: error");

				if (i < 9)
					i++;
				else
					break;

				IWDG_delay_ms(0);
			}
		}

		HAL_GPIO_WritePin(UC_PWR_GPIO_Port, UC_PWR_Pin, GPIO_PIN_RESET); // Turn uC Off

		IWDG_delay_ms(2000);
	}

	/************* BEGIN: NORMAL INIT FOR BUTTON AND AC POWER ************/

	//HAL_GPIO_WritePin(RTC_PWR_GPIO_Port, RTC_PWR_Pin, GPIO_PIN_RESET); // I2C Power Off
	HAL_GPIO_WritePin(DC_DC_ON_GPIO_Port, DC_DC_ON_Pin, GPIO_PIN_SET); // USB Power On

	uint8_t dc_dc_timeout = 0;

	while (V_DC_DC_val < 4.5) {
		ADC_get();

		IWDG_delay_ms(10);

		if (dc_dc_timeout < 100) {
			dc_dc_timeout++;
		} else {
			break;
		}
	}

	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

	HAL_TIM_Base_Stop_IT(&htim7);

	MPU6050_init(NO_CALC_ERROR);

	MPU6050_acc();

	if (HAL_GPIO_ReadPin(AC_DC_STATE_GPIO_Port, AC_DC_STATE_Pin)
			== GPIO_PIN_SET) {

		user_pwm_setvalue(0);

		draw_logo(STM32F4_logo);

		for (int i = 0; i < 1023; i++) {
			user_pwm_setvalue(i);

			if (i < 256) {
				IWDG_delay_ms(2);
			} else {
				IWDG_delay_ms(1);
			}
		}

		for (int i = 1023; i > 0; i--) {
			user_pwm_setvalue(i);

			IWDG_delay_ms(1);
		}

		ILI9341_FillScreen(ILI9341_BLACK);

		if (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_SET) {
			user_pwm_setvalue(lcd_bright_max);

			ILI9341_WriteString(5, 5, Firm_Mod, Font_11x18, ILI9341_GREY,
			ILI9341_BLACK);

			sprintf(str, "version: %s", Firm_Ver);

			ILI9341_WriteString(5, 35, str, Font_11x18, ILI9341_GREY,
			ILI9341_BLACK);

			sprintf(str, "by %s", byRef);

			ILI9341_WriteString(5, 65, str, Font_11x18, ILI9341_GREY,
			ILI9341_BLACK);

			ILI9341_WriteString(5, 95, compile_datetime, Font_11x18,
			ILI9341_GREY,
			ILI9341_BLACK);

			IWDG_delay_ms(2000);

			ILI9341_FillScreen(ILI9341_BLACK);
		}
	} else {
		ILI9341_FillScreen(ILI9341_BLACK);
	}

	while (1) {
		if (lcd_loop() == TRUE)
			break;
		IWDG_delay_ms(0);
	}

	lcd_bright_set = 0;

	lcd_bright_old = 0;

	lcd_show_logo = 0;

	refresh_info = TRUE;

	lcd_bright_set = lcd_bright_max;

	lcd_refresh = 3;

	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	HAL_TIM_Base_Start_IT(&htim7);

	HAL_GPIO_WritePin(USB_FS_PWR_GPIO_Port, USB_FS_PWR_Pin, GPIO_PIN_RESET); // USB On
}

void password_user_clear(void) {
	for (int i = 0; i < 6; i++) {
		password_code_tmp[i] = 0;

		sprintf(password_code_tmp_str[i], "[ ]");
	}
}

void password_config_clear(void) {
	for (int i = 0; i < 6; i++) {
		password_code_tmp0[0][i] = 0;
		password_code_tmp0[1][i] = 0;
		password_code_tmp0[2][i] = 0;

		sprintf(password_config_code_tmp_str0[i], "[ ]");
		sprintf(password_config_code_tmp_str1[i], "[ ]");
		sprintf(password_config_code_tmp_str2[i], "[ ]");
	}
}

void get_data_from_mem_for_main(void) {
	for (int i = 0; i < maxChannel; i++) {
		W25Q64_read_name(i, channelLabel[i]);
		W25Q64_read_header(i, &data_header);

		ch_status[i] = data_header.FLAG_ENABLED & 1;
		ch_status_tmp[i] = data_header.FLAG_ENABLED & 1;
		SENSORS[i].FLAG_ENABLED = data_header.FLAG_ENABLED & 1;

		channel_header_current[i].start_day = data_header.START_DAY;
		channel_header_current[i].start_month = data_header.START_MONTH;
		channel_header_current[i].start_year = data_header.START_YEAR;
		channel_header_current[i].start_hour = data_header.START_HOUR;
		channel_header_current[i].start_minute = data_header.START_MINUTE;

		channel_header_tmp[i].start_day = data_header.START_DAY;
		channel_header_tmp[i].start_month = data_header.START_MONTH;
		channel_header_tmp[i].start_year = data_header.START_YEAR;
		channel_header_tmp[i].start_hour = data_header.START_HOUR;
		channel_header_tmp[i].start_minute = data_header.START_MINUTE;

		channel_header_current[i].stop_day = data_header.STOP_DAY;
		channel_header_current[i].stop_month = data_header.STOP_MONTH;
		channel_header_current[i].stop_year = data_header.STOP_YEAR;
		channel_header_current[i].stop_hour = data_header.STOP_HOUR;
		channel_header_current[i].stop_minute = data_header.STOP_MINUTE;

		channel_header_tmp[i].stop_day = data_header.STOP_DAY;
		channel_header_tmp[i].stop_month = data_header.STOP_MONTH;
		channel_header_tmp[i].stop_year = data_header.STOP_YEAR;
		channel_header_tmp[i].stop_hour = data_header.STOP_HOUR;
		channel_header_tmp[i].stop_minute = data_header.STOP_MINUTE;

		HAL_StatusTypeDef result_read = HAL_BUSY;

		read_mark_steps = W25Q64_read_mark_preset(i);

		//UsrLog("W25Q64_read_mark_preset %lu", read_mark_steps);

		while (result_read != HAL_OK) {
			result_read = W25Q64_read_mark(i, ch_mark_cnt);

			if (result_read == HAL_ERROR) {
				UsrLog("A) W25Q64 read mark %d: error", i);
				break;
			} else if (result_read == HAL_OK) {
				//UsrLog("A) W25Q64 ch%d mark: %lu", i, ch_mark_cnt[i]);
			}

			IWDG_delay_ms(0);
		}

		channel_header_current[i].lenght_day = ch_mark_cnt[i] / 1440;

		channel_header_current[i].lenght_hour = (ch_mark_cnt[i]
				- (channel_header_current[i].lenght_day * 1440)) / 60;

		channel_header_current[i].lenght_minute = ch_mark_cnt[i]
				- (channel_header_current[i].lenght_day * 1440)
				- (channel_header_current[i].lenght_hour * 60);

//		UsrLog("ch_mark_cnt[%d] = %lu", i, ch_mark_cnt[i]);
	}

	for (int i = 0; i < maxChar_x2; i++) {
		channelLabel_tmp[i] = 0;
	}

	for (int i = 0; i < maxChar; i++) {
		channelLabel_tmp[i] = channelLabel[ch_config_index][i];
	}
}

//// data_in: 60 1e c3 13 77 57 89 a5 b7 a7 f5 04 bb f3 d2 28 f4 43 e3 ca 4d 62 b5 9a ca 84 e9 90 ca ca f5 c5 2b 09 30 da a2 3d e9 4c e8 70 17 ba 2d 84 98 8d df c9 c5 8d b6 7a ad a6 13 c2 dd 08 45 79 41 a6
//// check:   60 1e c3 13 77 57 89 a5 b7 a7 f5 04 bb f3 d2 28 f4 43 e3 ca 4d 62 b5 9a ca 84 e9 90 ca ca f5 c5 2b 09 30 da a2 3d e9 4c e8 70 17 ba 2d 84 98 8d df c9 c5 8d b6 7a ad a6 13 c2 dd 08 45 79 41 a6
//// Key: 60 3d eb 10 15 ca 71 be 2b 73 ae f0 85 7d 77 81 1f 35 2c 07 3b 61 08 d7 2d 98 10 a3 09 14 df f4
//// IV: f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff
//// data_out: 6b c1 be e2 2e 40 9f 96 e9 3d 7e 11 73 93 17 2a ae 2d 8a 57 1e 03 ac 9c 9e b7 6f ac 45 af 8e 51 30 c8 1c 46 a3 5c e4 11 e5 fb c1 19 1a 0a 52 ef f6 9f 24 45 df 4f 9b 17 ad 2b 41 7b e6 6c 37 10
//// check:    6b c1 be e2 2e 40 9f 96 e9 3d 7e 11 73 93 17 2a ae 2d 8a 57 1e 03 ac 9c 9e b7 6f ac 45 af 8e 51 30 c8 1c 46 a3 5c e4 11 e5 fb c1 19 1a 0a 52 ef f6 9f 24 45 df 4f 9b 17 ad 2b 41 7b e6 6c 37 10
//static int test_xcrypt_ctr(const char *xcrypt) {
//	uint8_t key[32] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b,
//			0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07,
//			0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf,
//			0xf4 };
//
//	uint8_t in[64] = { 0x60, 0x1e, 0xc3, 0x13, 0x77, 0x57, 0x89, 0xa5, 0xb7,
//			0xa7, 0xf5, 0x04, 0xbb, 0xf3, 0xd2, 0x28, 0xf4, 0x43, 0xe3, 0xca,
//			0x4d, 0x62, 0xb5, 0x9a, 0xca, 0x84, 0xe9, 0x90, 0xca, 0xca, 0xf5,
//			0xc5, 0x2b, 0x09, 0x30, 0xda, 0xa2, 0x3d, 0xe9, 0x4c, 0xe8, 0x70,
//			0x17, 0xba, 0x2d, 0x84, 0x98, 0x8d, 0xdf, 0xc9, 0xc5, 0x8d, 0xb6,
//			0x7a, 0xad, 0xa6, 0x13, 0xc2, 0xdd, 0x08, 0x45, 0x79, 0x41, 0xa6 };
//
//	uint8_t iv[16] = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
//			0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
//
//	uint8_t out[64] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9,
//			0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57,
//			0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e,
//			0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb,
//			0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x24, 0x45, 0xdf,
//			0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
//
//	struct AES_ctx ctx;
//
//	AES_init_ctx_iv(&ctx, key, iv);
//	AES_CTR_xcrypt_buffer(&ctx, in, 64);
//
//	UsrLog("CTR %s: ", xcrypt);
//
//	if (0 == memcmp((char*) out, (char*) in, 64)) {
//		UsrLog("SUCCESS!\n");
//		return (0);
//	} else {
//		UsrLog("FAILURE!\n");
//		return (1);
//	}
//}

//static int test_encrypt_ctr(void) {
//	return test_xcrypt_ctr("encrypt");
//}
//
//static int test_decrypt_ctr(void) {
//	return test_xcrypt_ctr("decrypt");
//}

void MX_SPI3_Init2(void) {

	/* USER CODE BEGIN SPI3_Init 0 */
	if (lcd_enabled == false) {
		return;
	}
	/* USER CODE END SPI3_Init 0 */

	/* USER CODE BEGIN SPI3_Init 1 */

	/* USER CODE END SPI3_Init 1 */
	/* SPI3 parameter configuration*/
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI3_Init 2 */

	/* USER CODE END SPI3_Init 2 */

}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	//__disable_irq();
	UsrLog("Error_Handler");
	beep(3, 1);
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
