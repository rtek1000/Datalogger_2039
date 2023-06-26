/*
 * EEPROM.h
 *
 *  Created on: 11 de ago de 2020
 *      Author: UserZ
 */

#ifndef INC_EEPROM_EXTERN_H_
#define INC_EEPROM_EXTERN_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "w25qxx.h"
#include <stdio.h>
#include <stdbool.h>

// Máximo de leituras por canal:
//1024 KiB (total) - 12 KiB (index) = 1012 KiB
//1012 KiB * 1024 = 1036288 Bytes
//1036288 Bytes / 19 bytes (por leitura) = 54541 leituras
#define Max_Record_Data 54541

//
uint8_t read_data_ch(uint8_t ch, uint32_t position, uint8_t *pBuffer);
uint8_t write_data_ch(uint8_t ch, uint32_t position, uint8_t *pBuffer);

HAL_StatusTypeDef W25Q64_init(void);
void W25Q64_get(void);
void W25Q64_put(void);
HAL_StatusTypeDef W25Q64_channel_erase(uint8_t channel);
HAL_StatusTypeDef W25Q64_channel_zerofill(uint8_t channel);
HAL_StatusTypeDef W25Q64_channel_check(uint8_t channel, uint8_t data);
HAL_StatusTypeDef W25Q64_channel_is_empty(uint8_t channel);
HAL_StatusTypeDef W25Q64_update_name(uint8_t channel, char *name);
HAL_StatusTypeDef W25Q64_read_name(uint8_t channel, char *name);
HAL_StatusTypeDef W25Q64_update_header(uint8_t channel, DATAHEADER_TypeDef* date_time);
HAL_StatusTypeDef W25Q64_read_header(uint8_t channel, DATAHEADER_TypeDef* date_time);
HAL_StatusTypeDef W25Q64_write_mark(uint8_t channel);
uint32_t W25Q64_read_mark_preset(uint8_t channel);
HAL_StatusTypeDef W25Q64_read_mark(uint8_t channel, uint32_t *mark_cnt);
HAL_StatusTypeDef W25Q64_write_data(uint8_t channel, uint32_t start_addr, DadaStruct_TypeDef* data);
HAL_StatusTypeDef W25Q64_read_data_sensor_ID(uint8_t channel, uint32_t index_cnt,
		DadaStruct_TypeDef *data_out);
HAL_StatusTypeDef W25Q64_read_data(uint8_t channel, uint32_t index_cnt, DadaStruct_TypeDef *data_in);
HAL_StatusTypeDef W25Q64_update_config(ConfigStruct_TypeDef *config);
HAL_StatusTypeDef W25Q64_read_config(ConfigStruct_TypeDef *config);
//HAL_StatusTypeDef W25Q64_update_stop(uint8_t channel, uint8_t *date_time);
//HAL_StatusTypeDef W25Q64_read_stop(uint8_t channel, uint8_t *name);

#endif /* INC_EEPROM_EXTERN_H_ */
