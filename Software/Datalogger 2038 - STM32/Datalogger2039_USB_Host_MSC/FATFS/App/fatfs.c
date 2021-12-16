/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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

#include "fatfs.h"

uint8_t retUSBH;    /* Return value for USBH */
char USBHPath[4];   /* USBH logical drive path */
FATFS USBHFatFS;    /* File system object for USBH logical drive */
FIL USBHFile;       /* File object for USBH */

/* USER CODE BEGIN Variables */
extern volatile DS3231_Time_t tm1;
extern uint16_t year_century;
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USBH driver ###########################*/
  retUSBH = FATFS_LinkDriver(&USBH_Driver, USBHPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
	return  ((DWORD)((year_century + tm1.year) - 1980) << 25)  // Year 2014
	        | ((DWORD)tm1.month << 21)            // Month 7
	        | ((DWORD)tm1.date << 16)           // Mday 10
	        | ((DWORD)tm1.hours << 11)           // Hour 16
	        | ((DWORD)tm1.minutes << 5)             // Min 0
	        | ((DWORD)tm1.seconds >> 1);            // Sec 0

  //return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
