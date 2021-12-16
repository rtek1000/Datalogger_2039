/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"

/* USER CODE BEGIN Includes */
#include "fatfs.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
FATFS USBDISKFatHs; /* File system object for USB disk logical drive */
FIL MyFile; /* File object */
char USBDISKPath[4] = { 48, 58, 47, 0 }; /* USB Host logical drive path */
USBH_HandleTypeDef hUSB_Host; /* USB Host handle */
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
uint8_t MSC_Application(void);
void USB_Error_Handler(uint16_t code);
/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */
uint8_t MSC_Application(void) {
	FRESULT res; /* FatFs function common result code */
	uint32_t byteswritten, bytesread; /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[100]; /* File read buffer */

	/* Register the file system object to the FatFs module */
	if (f_mount(&USBDISKFatHs, (TCHAR const*) USBDISKPath, 0) != FR_OK) {
		/* FatFs Initialization Error */
		USB_Error_Handler(0);
	} else {
		USBH_UsrLog(
				"f_mount(&USBDISKFatFs, (TCHAR const*)USBDISKPath, 0) == FR_OK");
		/* Create and Open a new text file object with write access */

		if (f_open(&MyFile, "Even.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
			/* 'STM32.TXT' file Open for write Error */
			USB_Error_Handler(1);
		} else {
			/* Write data to the text file */
			res = f_write(&MyFile, wtext, sizeof(wtext), (void*) &byteswritten);

			if ((byteswritten == 0) || (res != FR_OK)) {
				/* 'STM32.TXT' file Write or EOF Error */
				USB_Error_Handler(2);
			} else {
				USBH_UsrLog("(byteswritten != 0) && (res == FR_OK)");
				/* Close the open text file */
				f_close(&MyFile);

				/* Open the text file object with read access */
				if (f_open(&MyFile, "Even.TXT", FA_READ) != FR_OK) {
					/* 'STM32.TXT' file Open for read Error */
					USB_Error_Handler(3);
				} else {
					USBH_UsrLog(
							"f_open(&MyFile, 'Even.TXT', FA_READ) == FR_OK");
					/* Read data from the text file */
					res = f_read(&MyFile, rtext, sizeof(rtext),
							(void*) &bytesread);

					if ((bytesread == 0) || (res != FR_OK)) {
						/* 'STM32.TXT' file Read or EOF Error */
						USB_Error_Handler(4);
					} else {
						USBH_UsrLog("(bytesread != 0) && (res == FR_OK)");
						/* Close the open text file */
						f_close(&MyFile);

						/* Compare read data with the expected data */
						if ((bytesread != byteswritten)) {
							/* Read data is different from the expected data */
							USB_Error_Handler(5);
						} else {
							USBH_UsrLog("(bytesread == byteswritten");
							/* Success of the demo: no error occurrence */
							//BSP_LED_On(LED D2);
							HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0,
									GPIO_PIN_RESET);
						}
					}
				}
			}
		}
	}

	/* Unlink the USB disk I/O driver */
	//FATFS_UnLinkDriver(USBDISKPath);
	//USBH_HS_Restart();
	return TRUE;
}

void USB_Error_Handler(uint16_t code) {
	/* USER CODE BEGIN USB_Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
	//HAL_GPIO_WritePin(GPIOE,GPIO_PIN_0,GPIO_PIN_RESET); // LED D2
	//	while(1)
	//	{
	//	}
	USBH_UsrLog("USB_Error_Handler: %d", code);
	/* USER CODE END USB_Error_Handler */
}
/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostHS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostHS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
  break;

  case HOST_USER_DISCONNECTION:
  Appli_state = APPLICATION_DISCONNECT;
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
  break;

  case HOST_USER_CLASS_ACTIVE:
  Appli_state = APPLICATION_READY;
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  break;

  case HOST_USER_CONNECTION:
  Appli_state = APPLICATION_START;
  break;

  default:
  break;
  }
  /* USER CODE END CALL_BACK_1 */
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
