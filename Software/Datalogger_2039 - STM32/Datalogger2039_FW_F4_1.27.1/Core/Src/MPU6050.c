// Based on source: https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/

/*
 * MPU6050.c
 *
 *  Created on: Feb 3, 2020
 *      Author: UserZ
 */

#ifndef SRC_MPU6050_C_
#define SRC_MPU6050_C_

#include "main.h"
#include "MPU6050.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t lcd_inter_busy;
extern uint8_t acc_busy;
extern uint8_t rtc_busy;
extern I2C_HandleTypeDef hi2c3;
//extern TIM_HandleTypeDef htim7;
extern void IWDG_delay_ms(uint32_t delay);

uint32_t MPU6050_RESET_TIMEOUT = 0;

uint8_t MPU6050_AccDataRaw[6];
float MPU6050_AccAngleErrorX = 0;
float MPU6050_AccAngleErrorY = 0;
int MPU6050_AccAngleX = 0;
int MPU6050_AccAngleY = 0;

/*
#define  UsrLog(...)   do { \
		printf(__VA_ARGS__); \
		printf("\n"); \
} while (0)
*/

static void MX_I2C3_Init(void)
{

	/* USER CODE BEGIN I2C3_Init 0 */

	/* USER CODE END I2C3_Init 0 */

	/* USER CODE BEGIN I2C3_Init 1 */

	/* USER CODE END I2C3_Init 1 */
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = 10000;
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c3.Init.OwnAddress1 = 0;
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c3.Init.OwnAddress2 = 0;
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c3) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C3_Init 2 */

	/* USER CODE END I2C3_Init 2 */
}

HAL_StatusTypeDef MPU6050_send(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	acc_busy = TRUE;

	while(rtc_busy == TRUE)
	{
		IWDG_delay_ms(0);
	}

	HAL_StatusTypeDef result = HAL_I2C_Master_Transmit(hi2c, DevAddress, pData, Size, Timeout);

	acc_busy = FALSE;

	return result;
}

HAL_StatusTypeDef MPU6050_receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	acc_busy = TRUE;

	while(rtc_busy == TRUE)
	{
		IWDG_delay_ms(0);
	}

	HAL_StatusTypeDef result = HAL_I2C_Master_Receive(hi2c, DevAddress, pData, Size, Timeout);

	acc_busy = FALSE;

	return result;
}

HAL_StatusTypeDef MPU6050_setReg(uint8_t reg, uint8_t val)
{
	uint8_t data[2];

	data[0] = reg;
	data[1] = val;

	if(MPU6050_send(&hi2c3, MPU6050_ADDR, data, 2, 5) != HAL_OK)
	{
		UsrLog("MPU6050_update MPU6050_send 1 ERROR");
		MX_I2C3_Init();

		return HAL_ERROR;
	}
	else
	{
		if(MPU6050_send(&hi2c3, MPU6050_ADDR, data, 1, 5) != HAL_OK)
		{
			UsrLog("MPU6050_update MPU6050_send 2 ERROR");
			MX_I2C3_Init();

			return HAL_ERROR;
		}
		else
		{
			if(MPU6050_receive(&hi2c3, MPU6050_ADDR, data, 1, 5) != HAL_OK)
			{
				UsrLog("MPU6050_update MPU6050_receive 1 ERROR");
				MX_I2C3_Init();

				return HAL_ERROR;
			}
			else
			{
				if(data[0] != val)
				{
					return HAL_ERROR;
				}
				else
				{
					return HAL_OK;
				}
			}
		}
	}
}

HAL_StatusTypeDef MPU6050_ready(void)
{
	if(HAL_I2C_IsDeviceReady(&hi2c3, MPU6050_ADDR, 1, 5) != HAL_OK)
	{
		MX_I2C3_Init();
	}

	return HAL_I2C_IsDeviceReady(&hi2c3, MPU6050_ADDR, 1, 5);
}

HAL_StatusTypeDef MPU6050_init(uint8_t calc_error)
{
	uint8_t i = 0;

	//UsrLog("MPU6050_init");

	if(MPU6050_RESET_TIMEOUT != 0 )
	{
		while((HAL_GetTick() - 100) < MPU6050_RESET_TIMEOUT)
		{
			IWDG_delay_ms(0);
			UsrLog("MPU6050: device is not Ready; MPU6050_RESET_TIMEOUT:%lu", MPU6050_RESET_TIMEOUT);
			//return HAL_BUSY;
		}
	}

	MPU6050_RESET_TIMEOUT = 0;

	while(i < 9)
	{
		i++;

		if(MPU6050_ready() != HAL_OK)
		{
			MX_I2C3_Init();
		}

		if(MPU6050_ready() != HAL_OK)
		{
			UsrLog("MPU6050: Ready; ERROR; Step: %d", i);
			MX_I2C3_Init();
		}
		else if(MPU6050_who_am_i() != HAL_OK)
		{
			UsrLog("MPU6050: Who am i; ERROR; Step: %d", i);
			MX_I2C3_Init();
		}
		else if(MPU6050_setReg(MPU6050_PWR_MGMT_1, MPU6050_CLOCK_INTERNAL) != HAL_OK) // gyro off
		{
			UsrLog("MPU6050: MPU6050_PWR_MGMT_1; ERROR; Step: %d", i);
			MX_I2C3_Init();
		}
		else if(MPU6050_setReg(MPU6050_GYRO_CONFIG, MPU6050_GYRO_FULL_SCALE_250_DEG) != HAL_OK)
		{
			UsrLog("MPU6050: MPU6050_GYRO_CONFIG; ERROR; Step: %d", i);
			MX_I2C3_Init();
		}
		else if(MPU6050_setReg(MPU6050_ACCEL_CONFIG, MPU6050_ACCEL_FULL_SCALE_2G) != HAL_OK)
		{
			UsrLog("MPU6050: MPU6050_ACCEL_CONFIG; ERROR; Step: %d", i);
			MX_I2C3_Init();
		}
		else
		{
			if(calc_error == 1)
			{
				UsrLog("MPU6050 calculating the Acc error...");

				if(MPU6050_acc_error() != HAL_OK)
				{
					return HAL_ERROR;
				}
				else
				{
					return HAL_OK;
				}
			}
			else
			{
				return HAL_OK;
			}
		}
	}

	return HAL_ERROR;
}

HAL_StatusTypeDef MPU6050_acc(void)
{
	uint16_t rawAccX = 0, rawAccY = 0, rawAccZ = 0;
	int16_t rawAccXint = 0, rawAccYint = 0, rawAccZint = 0;
	int32_t rawAccXsum = 0, rawAccYsum = 0, rawAccZsum = 0;
	float accX = 0, accY = 0, accZ = 0;

	float accAngX = 0, accAngY = 0;
	uint16_t i = 0;
	float j = 0;

	// UsrLog("MPU6050_acc");

	if(MPU6050_RESET_TIMEOUT != 0)
	{
		UsrLog("MPU6050: device is not Ready; MPU6050_RESET_TIMEOUT:%lu", MPU6050_RESET_TIMEOUT);
		return HAL_BUSY;
	}

	while(i < 9)
	{
		i++;

		if(HAL_I2C_IsDeviceReady(&hi2c3, MPU6050_ADDR, 5, 5) != HAL_OK)
		{
			UsrLog("MPU6050_acc: device is not Ready; ERROR; Step: %d", i);
			MX_I2C3_Init();
		}
		else
		{
			for(uint8_t k = 0; k < 100; k++) // +95 tries
			{
				IWDG_delay_ms(0);

				uint8_t data[1] = {MPU6050_ACCEL_XOUT_H};
				if(MPU6050_send(&hi2c3, MPU6050_ADDR, data, 1, 5) != HAL_OK)
				{
					UsrLog("MPU6050_update MPU6050_send 1 ERROR");
					MX_I2C3_Init();
				}
				else
				{
					if(MPU6050_receive(&hi2c3, MPU6050_ADDR, MPU6050_AccDataRaw, 6, 30) != HAL_OK)
					{
						UsrLog("MPU6050_update MPU6050_receive 1 ERROR");
						MX_I2C3_Init();
					}
					else
					{
						rawAccX = (MPU6050_AccDataRaw[0] << 8) | MPU6050_AccDataRaw[1]; // from 8 bits to 16 bits (uint)
						rawAccY = (MPU6050_AccDataRaw[2] << 8) | MPU6050_AccDataRaw[3];
						rawAccZ = (MPU6050_AccDataRaw[4] << 8) | MPU6050_AccDataRaw[5];

						rawAccXint = (int16_t)rawAccX; // from uint to int and sum
						rawAccYint = (int16_t)rawAccY;
						rawAccZint = (int16_t)rawAccZ;

						//UsrLog("A Acc X:%05d; Y:%05d; Z:%05d", rawAccXint, rawAccYint, rawAccZint);

						rawAccXsum += (int32_t)rawAccXint; // sum
						rawAccYsum += (int32_t)rawAccYint;
						rawAccZsum += (int32_t)rawAccZint;

						if(j < 3)
						{
							j++;
						}
						else
						{
							if(k < 99)
							{
								k = 100;
							}
							else
							{
								return HAL_ERROR;
							}
						}
					}
				}
			}

			accX = (float)rawAccXsum / j;
			accY = (float)rawAccYsum / j;
			accZ = (float)rawAccZsum / j;

			//UsrLog("B Acc X:%05d; Y:%05d; Z:%05d", (int16_t)accX, (int16_t)accY, (int16_t)accZ);

			accX /= 16384.0;
			accY /= 16384.0;
			accZ /= 16384.0;

			//UsrLog("C Acc X:%.5f; Y:%.5f; Z:%.5f", accX, accY, accZ);

			accAngX = ((atan(accY / sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / PI) - MPU6050_AccAngleErrorX);
			accAngY = ((atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / PI) - MPU6050_AccAngleErrorY);

			//			accAngX = atan2(accY, accZ + abs(accX)) * 360 / 2.0 / PI;
			//			accAngY = atan2(accX, accZ + abs(accY)) * 360 / -2.0 / PI;

			MPU6050_AccAngleX = accAngX - 90;
			MPU6050_AccAngleY = accAngY;

			// UsrLog("Acc X:%d; Y:%d", MPU6050_AccAngleX, MPU6050_AccAngleY);

			return HAL_OK;
		}
	}

	return HAL_ERROR;
}

HAL_StatusTypeDef MPU6050_acc_error(void)
{
	uint16_t rawAccX = 0, rawAccY = 0, rawAccZ = 0;
	int16_t rawAccXint = 0, rawAccYint = 0, rawAccZint = 0;
	int32_t rawAccXsum = 0, rawAccYsum = 0, rawAccZsum = 0;
	float accX = 0, accY = 0, accZ = 0;

	float accErrorX = 0, accErrorY = 0;
	uint16_t i = 0;
	float j = 0;

	UsrLog("MPU6050_acc_error");

	if(MPU6050_RESET_TIMEOUT != 0)
	{
		UsrLog("MPU6050: device is not Ready; MPU6050_RESET_TIMEOUT:%lu", MPU6050_RESET_TIMEOUT);
		return HAL_BUSY;
	}

	while(i < 9)
	{
		i++;

		if(HAL_I2C_IsDeviceReady(&hi2c3, MPU6050_ADDR, 5, 5) != HAL_OK)
		{
			UsrLog("MPU6050_acc_error: device is not Ready; ERROR");
			MX_I2C3_Init();
		}
		else
		{
			for(i = 0; i < 1000; i++) // +800 tries
			{
				IWDG_delay_ms(0);

				uint8_t data[1] = {MPU6050_ACCEL_XOUT_H};
				if(MPU6050_send(&hi2c3, MPU6050_ADDR, data, 1, 5) != HAL_OK)
				{
					UsrLog("MPU6050_update MPU6050_send 1 ERROR");
					MX_I2C3_Init();
				}
				else
				{
					if(MPU6050_receive(&hi2c3, MPU6050_ADDR, MPU6050_AccDataRaw, 6, 30) != HAL_OK)
					{
						UsrLog("MPU6050_update MPU6050_receive 1 ERROR");
						MX_I2C3_Init();
					}
					else
					{
						rawAccX = (MPU6050_AccDataRaw[0] << 8) | MPU6050_AccDataRaw[1]; // from 8 bits to 16 bits (uint)
						rawAccY = (MPU6050_AccDataRaw[2] << 8) | MPU6050_AccDataRaw[3];
						rawAccZ = (MPU6050_AccDataRaw[4] << 8) | MPU6050_AccDataRaw[5];

						rawAccXint = (int16_t)rawAccX; // from uint to int and sum
						rawAccYint = (int16_t)rawAccY;
						rawAccZint = (int16_t)rawAccZ;

						//UsrLog("A Acc X:%05d; Y:%05d; Z:%05d", rawAccXint, rawAccYint, rawAccZint);

						rawAccXsum += (int32_t)rawAccXint; // sum
						rawAccYsum += (int32_t)rawAccYint;
						rawAccZsum += (int32_t)rawAccZint;

						if(j < 200)
						{
							j++;
						}
						else
						{
							if(i < 999)
							{
								i = 1000;
							}
							else
							{
								return HAL_ERROR;
							}
						}
					}
				}
			}

			accX = (float)rawAccXsum / j;
			accY = (float)rawAccYsum / j;
			accZ = (float)rawAccZsum / j;

			//UsrLog("B Acc X:%05d; Y:%05d; Z:%05d", (int16_t)accX, (int16_t)accY, (int16_t)accZ);

			accX /= 16384.0;
			accY /= 16384.0;
			accZ /= 16384.0;

			//UsrLog("C Acc X:%f; Y:%f; Z:%f", accX, accY, accZ);

			accErrorX = atan2(accY, accZ + abs(accX)) * 360 / 2.0 / PI;
			accErrorY = atan2(accX, accZ + abs(accY)) * 360 / -2.0 / PI;

			MPU6050_AccAngleErrorX = accErrorX;
			MPU6050_AccAngleErrorY = accErrorY;

			UsrLog("D Acc X:%f; Y:%f", accErrorX, accErrorY);


			return HAL_OK;
		}
	}

	return HAL_ERROR;
}

HAL_StatusTypeDef MPU6050_reset(void)
{
	uint8_t data[2];
	uint8_t i = 0;

//	UsrLog("MPU6050 reset");

	while(i < 5)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); // I2C3 Power Off

		IWDG_delay_ms(50);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET); // I2C3 Power On

		IWDG_delay_ms(50);

		if(HAL_I2C_IsDeviceReady(&hi2c3, MPU6050_ADDR, 5, 5) != HAL_OK) // start I2C3
		{
			MX_I2C3_Init();
		}

		if(HAL_I2C_IsDeviceReady(&hi2c3, MPU6050_ADDR, 5, 5) != HAL_OK)
		{
			UsrLog("MPU6050_reset: device is not Ready; ERROR");
			MX_I2C3_Init();
		}
		else
		{
			data[0] = MPU6050_PWR_MGMT_1;
			data[1] = 0x80;

			if(MPU6050_send(&hi2c3, MPU6050_ADDR, data, 2, 5) != HAL_OK)
			{
				UsrLog("MPU6050_update MPU6050_send 1 ERROR");
				MX_I2C3_Init();
			}
			else
			{
				MPU6050_RESET_TIMEOUT = HAL_GetTick();
//				UsrLog("MPU6050_reset: please wait 100ms, Tick:%lu", MPU6050_RESET_TIMEOUT);
				IWDG_delay_ms(0);

				return HAL_OK;
			}
		}

		i++;
	}

	return HAL_ERROR;
}

HAL_StatusTypeDef MPU6050_who_am_i(void)
{
	uint8_t data[2];

	//	UsrLog("MPU6050 who_am_i");

	data[0] = MPU6050_WHO_AM_I;
	data[1] = 0x00;

	if(MPU6050_RESET_TIMEOUT != 0)
	{
		UsrLog("MPU6050: device is not Ready; MPU6050_RESET_TIMEOUT:%lu", MPU6050_RESET_TIMEOUT);
		return HAL_BUSY;
	}

	if(MPU6050_send(&hi2c3, MPU6050_ADDR, data, 1, 10) != HAL_OK)
	{
		UsrLog("MPU6050_who_am_i MPU6050_send 1 ERROR");
		MX_I2C3_Init();
	}
	else
	{
		if(MPU6050_receive(&hi2c3, MPU6050_ADDR, data, 1, 5) != HAL_OK)
		{
			UsrLog("MPU6050_who_am_i MPU6050_receive 1 ERROR");
			MX_I2C3_Init();
		}
		else
		{
			if(data[0] == 0x68)
			{
				//				UsrLog("MPU6050_who_am_i OK, received value: 0x%02x", data[0]);
				return HAL_OK;
			}
			else
			{
				UsrLog("MPU6050_who_am_i ERROR, received value: 0x%02x", data[0]);
			}
		}
	}

	return HAL_ERROR;
}

#endif /* SRC_MPU6050_C_ */
