/*
 * MPU6050.h
 *
 *  Created on: Feb 3, 2020
 *      Author: UserZ
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "main.h"

#define PI 3.141592

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef TRUE
#define TRUE  1U
#endif

typedef enum {
	MPU6050_CLOCK_INTERNAL = 0x0,
	MPU6050_CLOCK_PLL_XGYRO
} MPU6050ClockEnumTypeDef;

typedef enum {
	MPU6050_ACCEL_FULL_SCALE_2G = 0x0,
} MPU6050AccFullScaleEnumTypeDef;

typedef enum {
	MPU6050_GYRO_FULL_SCALE_250_DEG = 0x0,
} MPU6050GyroFullScaleEnumTypeDef;

typedef enum {
	MPU6050_SMPLRT_DIV = 0x19,
	MPU6050_CONFIG = 0x1a,
	MPU6050_GYRO_CONFIG = 0x1b,
	MPU6050_ACCEL_CONFIG = 0x1c,
	MPU6050_ACCEL_XOUT_H = 0x3b,
	MPU6050_ADDR = 0xD0, // 0x68 << 1
	MPU6050_PWR_MGMT_1 = 0x6b,
	MPU6050_PWR_MGMT_2 = 0x6c,
	MPU6050_WHO_AM_I = 0x75
} MPU6050AddrEnumTypeDef;

typedef enum {
	NO_CALC_ERROR = 0,
    CALC_ERROR,
} CalcErrorEnumTypeDef;

typedef  struct  _MPU6050
{
	uint16_t AccX_RAW;
	uint16_t AccY_RAW;
	uint16_t AccZ_RAW;
	float AccX;
	float AccY;
	float AccZ;
	float AccErrorX;
	float AccErrorY;
	float accAngleX;
	float accAngleY;
}
MPU6050TypeDef;

HAL_StatusTypeDef MPU6050_send(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef MPU6050_receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef MPU6050_init(uint8_t calc_error);
HAL_StatusTypeDef MPU6050_acc(void);
HAL_StatusTypeDef MPU6050_reset(void);
HAL_StatusTypeDef MPU6050_who_am_i(void);
HAL_StatusTypeDef MPU6050_ready(void);
HAL_StatusTypeDef MPU6050_setReg(uint8_t reg, uint8_t val);
HAL_StatusTypeDef MPU6050_acc_error(void);

#endif /* INC_MPU6050_H_ */
