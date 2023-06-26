/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbh_def.h"
#include "ILI9341.h"
#include <stdbool.h>
#include <stdio.h>
#include "DS3231.h"
#include "MPU6050.h"
#include "SCREEN.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
//typedef enum {
//	LcdScreenGraphic = 0,
//	LcdScreenMenu,
//	LcdScreenCfgCh,
//	LcdScreenCfgClk,
//	LcdScreenMonCh
//}LcdScreenTypeDef;
uint32_t tick1ms = 0;

//extern uint32_t led_blink;
//extern uint32_t check_point;

extern uint8_t ctrl_atl_del_restart;
extern uint8_t ctrl_atl_del_timeout;

extern uint8_t button_old;

extern uint32_t tick100ms;
extern uint32_t tick250ms;
extern uint32_t tick500ms;
extern uint32_t tick1000ms;

uint32_t sysTick1000 = 0;
uint8_t sysTick15000 = 0;

uint8_t pwrOffLcdBackOld = 0;
extern uint8_t update_info;
extern char str2[5];
int cnt_tmr7 = 0;

extern uint8_t blink_cnt1;
extern uint8_t blink_passwd;
extern uint8_t blink_cnt2;
extern uint8_t blink_text;

//uint16_t color_label_ch = 0;

//uint16_t progress_bar_usb_export_value;
extern HID_KEYBD_Info_TypeDef *keybd_info1;

extern uint8_t language_current;

extern int usb_power_restart_timeout;
extern int usb_power_restart_count;
extern int usb_power_recycle_count;

//extern uint16_t progress_usb_export; // 0-100

extern USBH_ClassTypeDef HID_Class;
#define USBH_HID_CLASS    &HID_Class

extern uint8_t pwrOffLcdBack;
extern uint8_t button1_restart_enabled;
extern uint8_t blink;

extern I2C_HandleTypeDef hi2c2;
//extern bool USB_FS_enabled;
extern uint8_t get_temperatures;
extern uint32_t timeout_temperature_read;
extern float temperatures[8];

extern int pwm_value;
extern int step;
extern uint32_t enumTimeout;

extern ADC_HandleTypeDef hadc1;
//extern int lcd_bright_max;
//extern int lcd_bright_min;
extern int lcd_bright_off;
extern int lcd_bright_new;
extern int lcd_bright_set;
extern int lcd_bright_old;
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
extern uint8_t seconds_old;
extern char clock_date[14];
extern char clock_time[9];
extern int clock_index_edit;
extern int clock_index_edit_mode;
extern int ch_config_index_edit;
extern int ch_config_index_edit_mode;
extern int ch_status_tmp[8];
extern char str[80];

//extern int keyboard_status;
extern int keyboard_timeout_reset;
//extern bool keyboard_insert;
extern int ch_config_index;

extern USBH_HandleTypeDef hUsbHostFS;

extern int keyboard_timeout;
extern int keyboard_timeout2;

extern bool lcd_updateInfo;
extern uint8_t lcd_refresh;
extern int lcd_phase;
extern bool lcd_enabled;
extern uint8_t init_flag;

//extern const uint16_t lcd_timeout0recall;
//extern const uint16_t lcd_timeout0recall2;
//extern uint8_t lcd_timeout0;

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
extern char channelLabel[8][57];
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
extern void beep(uint8_t repetions, uint8_t is_error);
extern void lcd_password_blink(uint16_t color);
extern void lcd_config_ch_blink_status(void);
extern void lcd_config_ch_blink_status2(void);
extern void IWDG_delay_ms(uint32_t delay);
extern void DS18B20_read(int mode, float *temperatures);
extern void MX_USB_HOST_Process(void);
extern void user_pwm_setvalue(uint16_t value);

extern USBH_StatusTypeDef USBH_FS_Restart(void);

extern void drawInfo(void);

extern void password_user_clear(void);
extern void MX_SPI3_Init(void);
extern void MX_SPI3_Init2(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern TIM_HandleTypeDef htim7;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
	UsrLog("NMI_Handler");
	beep(4, 1);
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
	  UsrLog("HardFault_Handler");

	  HAL_Delay(2000);

	  HAL_NVIC_SystemReset();

	  break;
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
	  UsrLog("MemManage_Handler");

	  HAL_Delay(2000);

	  HAL_NVIC_SystemReset();

	  break;
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
	  UsrLog("BusFault_Handler");

	  HAL_Delay(2000);

	  HAL_NVIC_SystemReset();

	  break;
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
	  UsrLog("UsageFault_Handler");

	  HAL_Delay(2000);

	  HAL_NVIC_SystemReset();

	  break;
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
	UsrLog("PendSV_Handler");
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
	// check_point = 1;

//	if (led_blink < 5000) {
//		led_blink++;
//	} else {
//		led_blink = 0;
//
//		UsrLog("check_point: %ld", check_point);
//
////		beep(check_point, 0);
//
//		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // LED2 Toggle
//	}

	if ((HAL_GetTick() - 999) >= sysTick1000) {
		sysTick1000 = HAL_GetTick();

		if (ctrl_atl_del_restart == 1) {
			if (ctrl_atl_del_timeout > 0) {
				ctrl_atl_del_timeout--;
			} else {
				HAL_NVIC_SystemReset();
			}
		}

		if (button1_restart_enabled == 1) {
			if (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin)
					== GPIO_PIN_SET) {
				if (sysTick15000 < 14) // 15s reset uC
						{
					sysTick15000++;
				} else {
					sysTick15000 = 0;

					HAL_NVIC_SystemReset();
				}
			} else {
				sysTick15000 = 0;
			}
		}

		if (tm1.seconds < 59) {
			tm1.seconds++;
		} else {
			tm1.seconds = 0;

			if (tm1.minutes < 59) {
				tm1.minutes++;
			} else {
				tm1.minutes = 0;

				if (tm1.hours < 23) {
					tm1.hours++;
				} else {
					tm1.hours = 0;
				}
			}
		}
	}
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(CHARGER_STATE_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(USB_FS_OC_Pin);
  HAL_GPIO_EXTI_IRQHandler(USB_HS_OC_Pin);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(BUTTON1_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}


/**
  * @brief This function handles ADC1, ADC2 and ADC3 global interrupts.
  */
void ADC_IRQHandler(void)
{
  /* USER CODE BEGIN ADC_IRQn 0 */

  /* USER CODE END ADC_IRQn 0 */
	  HAL_ADC_IRQHandler(&hadc1);
	  HAL_ADC_IRQHandler(&hadc2);
	  HAL_ADC_IRQHandler(&hadc3);
  /* USER CODE BEGIN ADC_IRQn 1 */

  /* USER CODE END ADC_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */

  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */
	if (lcd_enabled != TRUE)
		return;

// init_flag

	if (HAL_GetTick() > tick1ms) {
		tick1ms = HAL_GetTick();

		if (lcd_bright_old < lcd_bright_set) {
			lcd_bright_old++;

			user_pwm_setvalue(lcd_bright_old);
		} else if (lcd_bright_old > lcd_bright_set) {
			lcd_bright_old--;

			user_pwm_setvalue(lcd_bright_old);
		}
	}

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS End Point 1 Out global interrupt.
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_EP1_OUT_IRQn 0 */

  /* USER CODE END OTG_HS_EP1_OUT_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_EP1_OUT_IRQn 1 */

  /* USER CODE END OTG_HS_EP1_OUT_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS End Point 1 In global interrupt.
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_EP1_IN_IRQn 0 */

  /* USER CODE END OTG_HS_EP1_IN_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_EP1_IN_IRQn 1 */

  /* USER CODE END OTG_HS_EP1_IN_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS global interrupt.
  */
void OTG_HS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_IRQn 0 */

  /* USER CODE END OTG_HS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_IRQn 1 */

  /* USER CODE END OTG_HS_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
