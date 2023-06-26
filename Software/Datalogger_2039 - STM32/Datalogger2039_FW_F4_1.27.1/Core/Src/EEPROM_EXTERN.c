/*
 * EEPROM.c
 *
 *  Created on: Aug 10, 2020
 *      Author: UserZ
 */

#include "EEPROM_EXTERN.h"

//	W25Q64 MAP Address byte:
//
//	Address             Begin
//	Channel 1 name:     0x000000 + 0x01000 (1 Block)    = 0x000FFF
//	Channel 1 index:    0x001000 + 0x03000 (3 Blocks)   = 0x003FFF
//	Channel 1 data :    0x004000 + 0xFA000 (250 Blocks) = 0x0FDFFF
//	Channel 1 reserved: 0x0FE000 + 0x02000 (2 Block)    = 0X0FFFFF

#define mem_addr_ch_begin        0x00000
#define mem_addr_ch_name         0x01000
#define mem_addr_ch_index        0x03000
#define mem_addr_ch_data         0xFA000
#define mem_addr_ch_header       0x01000
#define mem_addr_ch_rsv          0x01000
#define mem_addr_ch_end          0xFFFFF

#define mem_addr_ch_name_begin(ch)     ((uint32_t)(mem_addr_ch_begin + (ch * (mem_addr_ch_end + 0x01))))
#define mem_addr_ch_name_end(ch)       ((uint32_t)(mem_addr_ch_name_begin(ch) + mem_addr_ch_name - 0x01))
#define mem_addr_ch_index_begin(ch)    ((uint32_t)(mem_addr_ch_name_end(ch) + 0x01))
#define mem_addr_ch_index_end(ch)      ((uint32_t)(mem_addr_ch_index_begin(ch) + mem_addr_ch_index - 0x01))
#define mem_addr_ch_data_begin(ch)     ((uint32_t)(mem_addr_ch_index_end(ch) + 0x01))
#define mem_addr_ch_data_end(ch)       ((uint32_t)(mem_addr_ch_data_begin(ch) + mem_addr_ch_data - 0x01))
#define mem_addr_ch_header_begin(ch)   ((uint32_t)(mem_addr_ch_data_end(ch) + 0x01))
#define mem_addr_ch_header_end(ch)     ((uint32_t)(mem_addr_ch_header_begin(ch) + mem_addr_ch_header - 0x01))
#define mem_addr_ch_rsv_begin(ch)      ((uint32_t)(mem_addr_ch_header_end(ch) + 0x01))
#define mem_addr_ch_rsv_end(ch)        ((uint32_t)(mem_addr_ch_rsv_begin(ch) + mem_addr_ch_rsv - 0x01))

uint8_t FLASH_Error = FALSE;
uint8_t Flash_DataBuffer[256];

uint16_t isEmpty_steps1 = 0;
uint16_t isEmpty_steps2 = 0;
uint16_t erase_steps1 = 0;
uint16_t erase_steps2 = 0;
uint16_t zerofill_steps1 = 0;
uint16_t zerofill_steps2 = 0;
uint16_t channel_check_steps = 0;
uint32_t write_mark_steps = 0;
uint32_t read_mark_steps = 0;
uint32_t write_data_steps = 0;
uint32_t read_data_steps = 0;
uint8_t previous_cell_index = 0;
uint8_t previous_cell_is_full = 0;
uint8_t read_previous_cell_is_full = 0;
uint8_t dataReadBuff[19] = { 0 };

extern uint8_t MEM_ID[8];

HAL_StatusTypeDef W25Q64_init(void) {
	if (!W25qxx_Init()) {
		UsrLog("W25qxx_Init: fail");

		for (uint8_t i = 0; i < 4; i++) {
			if (!W25qxx_Init()) {
				UsrLog("W25qxx_Init: fail");
			} else {
				UsrLog("W25qxx_Init: ok");
				i = 4;
			}

			IWDG_delay_ms(500);
		}
//	} else {
//		UsrLog("W25qxx_Init: ok");
	}

	IWDG_delay_ms(0);

//	UsrLog("%lu", HAL_GetTick());

	if (w25qxx.ID != W25Q64_ID) {
		for (uint8_t i = 0; i < 4; i++) {
			IWDG_delay_ms(500);
		}

		FLASH_Error = TRUE;

		UsrLog("W25Q64_ID: error");
	} else {
		FLASH_Error = FALSE;

//		UsrLog("W25Q64_ID: ok");
	}

	W25qxx_ReadUniqID();

	for (int i = 0; i <= 7; i++) {
		MEM_ID[i] = w25qxx.UniqID[i];
	}

	UsrLog("W25Q64 UniqID: %02X%02X%02X%02X %02X%02X%02X%02X", w25qxx.UniqID[0],
			w25qxx.UniqID[1], w25qxx.UniqID[2], w25qxx.UniqID[3],
			w25qxx.UniqID[4], w25qxx.UniqID[5], w25qxx.UniqID[6],
			w25qxx.UniqID[7]);

	if (FLASH_Error == TRUE) {
		return HAL_ERROR;
	} else {
		return HAL_OK;
	}
}

HAL_StatusTypeDef W25Q64_channel_erase(uint8_t channel) {
	uint32_t i = 0;
//	uint32_t t = 0;
	uint32_t begin = mem_addr_ch_name_begin(channel);
	uint32_t finish = mem_addr_ch_data_end(channel);

//	UsrLog("Erase block start: %05lX; stop: %05lX", begin, (finish + 1));

	i = ((finish + 1) - begin) / 65536;

//	UsrLog("64 KiB blocks: %lu", i);
	if (erase_steps1 < ((finish + 1) / 65536)) {
		if (erase_steps1 < (begin / 65536)) {
			erase_steps1 = (begin / 65536);
		}

		IWDG_delay_ms(0);

		//UsrLog("Block 64KiB erase addr: %05X", erase_steps1 * 65536);

		if (W25qxx_IsEmptyBlock(erase_steps1, 0x00, 0x00) == false) {
			W25qxx_EraseBlock64(erase_steps1);  // 150ms
		}

		erase_steps1++;
	} else if (erase_steps1 == ((finish + 1) / 65536)) {
		if (erase_steps2 < ((finish + 1) / 4096)) {
			if (erase_steps2 < ((begin + (i * 65536)) / 4096)) {
				erase_steps2 = ((begin + (i * 65536)) / 4096);
			}

			IWDG_delay_ms(0);

			//UsrLog("Sector 4KiB erase addr: %05X", erase_steps2 * 4096);

			if (W25qxx_IsEmptySector(erase_steps2, 0x00, 0x00) == false) {
				W25qxx_EraseSector(erase_steps2);  // 30ms
			}

			erase_steps2++;
		} else if (erase_steps2 == ((finish + 1) / 4096)) {
			erase_steps1 = 0;
			erase_steps2 = 0;
			return HAL_OK;
		}
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_channel_zerofill(uint8_t channel) {
	uint32_t i = 0;
//	uint32_t t = 0;
	uint32_t begin = mem_addr_ch_name_begin(channel);
	uint32_t finish = mem_addr_ch_data_end(channel);

	//UsrLog("Zero fill block start: %05lX; stop: %05lX", begin, (finish + 1));

	i = ((finish + 1) - begin) / 65536;

	//UsrLog("64 KiB blocks: %lu", i);

	if (zerofill_steps1 < ((finish + 1) / 65536)) {
		if (zerofill_steps1 < (begin / 65536)) {
			zerofill_steps1 = (begin / 65536);
		}

		IWDG_delay_ms(0);

		//UsrLog("Block 64KiB zero fill addr: %05X", zerofill_steps1 * 65536);

		W25qxx_WriteBlockZeroFill(zerofill_steps1);  // 150ms

		zerofill_steps1++;
	} else if (zerofill_steps1 == ((finish + 1) / 65536)) {
		if (zerofill_steps2 < ((finish + 1) / 4096)) {
			if (zerofill_steps2 < ((begin + (i * 65536)) / 4096)) {
				zerofill_steps2 = ((begin + (i * 65536)) / 4096);
			}

			IWDG_delay_ms(0);

			//UsrLog("Sector 4KiB zero fill addr: %05X", zerofill_steps2 * 4096);

			W25qxx_WriteSectorZeroFill(zerofill_steps2);  // 30ms

			zerofill_steps2++;
		} else if (zerofill_steps2 == ((finish + 1) / 4096)) {
			zerofill_steps1 = 0;
			zerofill_steps2 = 0;
			return HAL_OK;
		}
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_channel_check(uint8_t channel, uint8_t data) {
//	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;
	uint8_t buff[256];
	uint32_t check = 0;
	uint32_t begin = mem_addr_ch_name_begin(channel);
	uint32_t finish = mem_addr_ch_data_end(channel);

	if (channel_check_steps < ((finish + 1) / 256)) {

		if (channel_check_steps < (begin / 256)) {
			channel_check_steps = (begin / 256);
		}

		IWDG_delay_ms(0);

		W25qxx_ReadPage(buff, channel_check_steps, 0, 256);

		check = 0;

		for (j = 0; j < 256; j++) {
			if (buff[j] != data) {

				//UsrLog("2) Page not all %02X: %05lX", data,
				//		(channel_check_steps * 256) + j);

				check++;
			}
		}

		if (check != 0) {
			channel_check_steps = 0;
			return HAL_ERROR;
		}

		if (k < 99) {
			k++;
		} else {
			k = 0;

			//UsrLog("W25Q64 channel check: %05X", (channel_check_steps * 256));
		}

		channel_check_steps++;
	} else if (channel_check_steps == ((finish + 1) / 256)) {
		channel_check_steps = 0;
		return HAL_OK;
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_channel_is_empty(uint8_t channel) {
	uint32_t i = 0;
//	uint32_t t = 0;
	uint32_t begin = mem_addr_ch_name_begin(channel);
	uint32_t finish = mem_addr_ch_data_end(channel);

//	UsrLog("Erase block start: %05lX; stop: %05lX", begin, (finish + 1));

	i = ((finish + 1) - begin) / 65536;

//	UsrLog("64 KiB blocks: %lu", i);
	if (isEmpty_steps1 < ((finish + 1) / 65536)) {
		if (isEmpty_steps1 < (begin / 65536)) {
			isEmpty_steps1 = (begin / 65536);
		}

		IWDG_delay_ms(0);

		//UsrLog("Block 64KiB erase addr: %05X", erase_steps1 * 65536);

		if (W25qxx_IsEmptyBlock(isEmpty_steps1, 0x00, 0x00) == true) {  // 150ms
			isEmpty_steps1++;
		} else {
			//UsrLog("Error is not empty block");
			isEmpty_steps1 = 0;
			isEmpty_steps2 = 0;
			return HAL_ERROR;
		}
	} else if (isEmpty_steps1 == ((finish + 1) / 65536)) {
		if (isEmpty_steps2 < ((finish + 1) / 4096)) {
			if (isEmpty_steps2 < ((begin + (i * 65536)) / 4096)) {
				isEmpty_steps2 = ((begin + (i * 65536)) / 4096);
			}

			IWDG_delay_ms(0);

			//UsrLog("Sector 4KiB erase addr: %05X", erase_steps2 * 4096);

			if (W25qxx_IsEmptySector(isEmpty_steps2, 0x00, 0x00) == true) { // 30ms
				isEmpty_steps2++;
			} else {
				//UsrLog("Error is not empty sector");
				isEmpty_steps1 = 0;
				isEmpty_steps2 = 0;
				return HAL_ERROR;
			}
		} else if (isEmpty_steps2 == ((finish + 1) / 4096)) {

			//UsrLog("OK all empty");
			isEmpty_steps1 = 0;
			isEmpty_steps2 = 0;
			return HAL_OK;
		}
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_update_name(uint8_t channel, char *name) {
	uint8_t buff[256];
	uint32_t begin = mem_addr_ch_name_begin(channel);
	uint32_t Page_Address = begin / 256;
	uint32_t Sector_Address = begin / 4096;
	uint32_t OffsetInByte = 0;
	uint32_t NumByteToWrite_up_to_PageSize = 0;
	uint32_t NumByteToWrite_up_to_SectorSize = 0;
	uint8_t test_count = 0;

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

	uint8_t trim_name = 1;

	while (trim_name == 1) {
		for (int i = maxChar; i >= 0; i--) {
			if (i == 0) {
				trim_name = 0;
				break;
			} else {
				if ((name[i] == ' ') && (name[i - 1] == ' ')) {
					name[i] = 0;
				}
			}
		}
	}

	for (int i = 0; i < maxChar; i++) {
		if (name[i] != buff[i]) {
			test_count++;
		}
	}

	if (test_count == 0) {
		//UsrLog("1) Name OK");
		return HAL_OK;
	}

//	UsrLog("CH%d Name: %s", channel, name);

	for (int i = 0; i < maxChar; i++) {
		buff[i] = name[i];
	}

	if (W25qxx_IsEmptySector(Sector_Address, OffsetInByte,
			NumByteToWrite_up_to_SectorSize) != true) {  // 30ms
		//UsrLog("Update 1: Is not empty sector");

		W25qxx_EraseSector(Sector_Address);  // 30ms

		if (W25qxx_IsEmptySector(Sector_Address, OffsetInByte,
				NumByteToWrite_up_to_SectorSize) != true) {  // 30ms
			//UsrLog("Update 1: Is not empty sector");

			//UsrLog("2) Name not OK");
			return HAL_ERROR;
		}
	}

	W25qxx_WritePage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

	for (int i = 0; i < maxChar; i++) {
		if (name[i] != buff[i]) {
			test_count++;
		}
	}

	if (test_count != 0) {
		//UsrLog("3) Name not OK");
		return HAL_ERROR;
	}

	//UsrLog("4) Name OK");

	return HAL_OK;
}

HAL_StatusTypeDef W25Q64_read_name(uint8_t channel, char *name) {
	uint8_t buff[256];
//	char buffStr[maxChar + 1];

	uint32_t begin = mem_addr_ch_name_begin(channel);
	uint32_t Page_Address = begin / 256;
//	uint32_t Sector_Address = begin / 4096;

	uint32_t OffsetInByte = 0;
	uint32_t NumByteToWrite_up_to_PageSize = 256;
//	uint32_t NumByteToWrite_up_to_SectorSize = 0;

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

	for (int i = 0; i < maxChar; i++) {
//		buffStr[i] = buff[i];
		if (buff[i] != 255) {
			name[i] = buff[i];
		} else {
			name[i] = 0;
		}
	}

	//UsrLog("CH%d Name: %s", channel, buffStr);

	return HAL_OK;
}

HAL_StatusTypeDef W25Q64_update_header(uint8_t channel,
		DATAHEADER_TypeDef *date_time) {
	uint8_t buff[256];
	uint32_t begin = mem_addr_ch_header_begin(channel);
	uint32_t Page_Address = begin / 256;
	uint32_t Sector_Address = begin / 4096;
	uint32_t OffsetInByte = 0;
	uint32_t NumByteToWrite_up_to_PageSize = 0;
	uint32_t NumByteToWrite_up_to_SectorSize = 0;
	uint8_t test_count = 0;
	uint8_t maxIndex = 11; // en dd mm yy hh mm dd mm yy hh mm
	uint8_t date_time_tmp[11];

//	UsrLog("W25Q64_update_header");

	date_time_tmp[0] = date_time->FLAG_ENABLED;
	date_time_tmp[1] = date_time->START_DAY;
	date_time_tmp[2] = date_time->START_MONTH;
	date_time_tmp[3] = date_time->START_YEAR;
	date_time_tmp[4] = date_time->START_HOUR;
	date_time_tmp[5] = date_time->START_MINUTE;
	date_time_tmp[6] = date_time->STOP_DAY;
	date_time_tmp[7] = date_time->STOP_MONTH;
	date_time_tmp[8] = date_time->STOP_YEAR;
	date_time_tmp[9] = date_time->STOP_HOUR;
	date_time_tmp[10] = date_time->STOP_MINUTE;

//	UsrLog("W25Q64_update_header 1");

//	UsrLog("Page_Address %lu", Page_Address);

//	UsrLog("w25qxx.Lock %d", w25qxx.Lock);

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	UsrLog("W25Q64_update_header 2");

	test_count = 0;

	for (int i = 0; i < maxIndex; i++) {
//		UsrLog("1) Update date_time[%d]: %02d buff: %02d", i, date_time_tmp[i],
//				buff[i]);

//		if (i == 0) {
		if (date_time_tmp[i] != buff[i]) {
			test_count++;
		}
//		}

	}

//	UsrLog("W25Q64_update_header 3");

	if (test_count == 0) {
		//UsrLog("1) Name OK");
		return HAL_OK;
	}

//	UsrLog("W25Q64_update_header 4");

//	UsrLog("CH%d Name: %s", channel, name);

	for (int i = 0; i < maxIndex; i++) {
		buff[i] = date_time_tmp[i];
	}

//	UsrLog("W25Q64_update_header 5");

	if (W25qxx_IsEmptySector(Sector_Address, OffsetInByte,
			NumByteToWrite_up_to_SectorSize) != true) {  // 30ms
//		UsrLog("Update 1: Is not empty sector");

		W25qxx_EraseSector(Sector_Address);  // 30ms

		if (W25qxx_IsEmptySector(Sector_Address, OffsetInByte,
				NumByteToWrite_up_to_SectorSize) != true) {  // 30ms
//			UsrLog("Update 2: Is not empty sector");

			//UsrLog("2) Name not OK");
			return HAL_ERROR;
		}
	}

//	UsrLog("W25Q64_update_header 6");

	W25qxx_WritePage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	UsrLog("W25Q64_update_header 7");

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	UsrLog("W25Q64_update_header 8");

	test_count = 0;

	for (int i = 0; i < maxIndex; i++) {
//		UsrLog("2) Update buff[%d]: %d", i, buff[i]);
		if (date_time_tmp[i] != buff[i]) {
			test_count++;
		}
	}

//	UsrLog("W25Q64_update_header 9");

	if (test_count != 0) {
		//UsrLog("3) Name not OK");
		return HAL_ERROR;
	}

//	UsrLog("W25Q64_update_header 10");

	//UsrLog("4) Name OK");

	return HAL_OK;
}

HAL_StatusTypeDef W25Q64_read_header(uint8_t channel,
		DATAHEADER_TypeDef *date_time) {
	uint8_t buff[256];
//	char buffStr[maxChar + 1];

	uint32_t begin = mem_addr_ch_header_begin(channel);
	uint32_t Page_Address = begin / 256;
//	uint32_t Sector_Address = begin / 4096;

	uint32_t OffsetInByte = 0;
	uint32_t NumByteToWrite_up_to_PageSize = 256;
//	uint32_t NumByteToWrite_up_to_SectorSize = 0;
//	uint8_t maxIndex = 11; // en dd mm yy hh mm dd mm yy hh mm
//	uint8_t date_time_tmp[11];

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	for (int i = 0; i < maxIndex; i++) {
//		if (buff[i] == 255) { // erased state
//			buff[i] = 0;
//		}
//	}

	date_time->FLAG_ENABLED = buff[0];
	date_time->START_DAY = buff[1];
	date_time->START_MONTH = buff[2];
	date_time->START_YEAR = buff[3];
	date_time->START_HOUR = buff[4];
	date_time->START_MINUTE = buff[5];
	date_time->STOP_DAY = buff[6];
	date_time->STOP_MONTH = buff[7];
	date_time->STOP_YEAR = buff[8];
	date_time->STOP_HOUR = buff[9];
	date_time->STOP_MINUTE = buff[10];

//UsrLog("CH%d Name: %s", channel, buffStr);

	return HAL_OK;
}

HAL_StatusTypeDef W25Q64_write_mark(uint8_t channel) {
//	uint32_t i = 0;
	uint32_t j = 0;
//	uint32_t k = 0;
	uint8_t buff[256];
//	uint32_t check = 0;
	uint32_t begin = mem_addr_ch_index_begin(channel);
	uint32_t finish = mem_addr_ch_index_end(channel);
//	uint8_t data = 0;
//	uint32_t OffsetInByte = 0;
//	uint32_t NumByteToWrite_up_to_PageSize = 256;

	if (write_mark_steps < ((finish + 1) / 256)) {

		if (write_mark_steps < (begin / 256)) {
			write_mark_steps = begin / 256;
		}

		IWDG_delay_ms(0);

		W25qxx_ReadPage(buff, write_mark_steps, 0, 256);

//		check = 0;

		for (j = 0; j < 256; j++) {
			if (buff[j] == 0xFF) {
				buff[j] = 0b11111110;
//				if (previous_cell_is_full == 1) {
//					previous_cell_is_full = 0;
//				UsrLog("channel: %d addr: %lu data: %d", channel, (write_mark_steps * 256) + j, buff[j]);

				W25qxx_WriteByte(buff[j], (write_mark_steps * 256) + j);

				write_mark_steps = 0;

				return HAL_OK;
//				}
			} else if (buff[j] != 0x00) {
				if (buff[j] == 0b11111110) {
					buff[j] = 0b11111100;
				} else if (buff[j] == 0b11111100) {
					buff[j] = 0b11111000;
				} else if (buff[j] == 0b11111000) {
					buff[j] = 0b11110000;
				} else if (buff[j] == 0b11110000) {
					buff[j] = 0b11100000;
				} else if (buff[j] == 0b11100000) {
					buff[j] = 0b11000000;
				} else if (buff[j] == 0b11000000) {
					buff[j] = 0b10000000;
				} else if (buff[j] == 0b10000000) {
					buff[j] = 0b00000000;
				}

				//UsrLog("channel: %d", channel);
//				UsrLog("channel: %d addr: %lu data: %d", channel, (write_mark_steps * 256) + j, buff[j]);

				W25qxx_WriteByte(buff[j], (write_mark_steps * 256) + j);

				write_mark_steps = 0;
				return HAL_OK;
			}
		}

		write_mark_steps++;
	} else if (write_mark_steps == ((finish + 1) / 256)) {
		write_mark_steps = 0;
		return HAL_ERROR;
	}

	return HAL_BUSY;
}

uint32_t W25Q64_read_mark_preset(uint8_t channel) {
	uint32_t begin = mem_addr_ch_index_begin(channel);
//	uint32_t finish = mem_addr_ch_index_end(channel);
	uint32_t Sector_Address = begin / 4096;
	uint32_t OffsetInByte = 0;
//	uint32_t NumByteToWrite_up_to_PageSize = 0;
	uint32_t NumByteToWrite_up_to_SectorSize = 0;

	if (W25qxx_IsEmptySector(Sector_Address + 2, OffsetInByte,
			NumByteToWrite_up_to_SectorSize) != true) {
		return ((Sector_Address + 2) * 16); // 4096 / 256 = 16
	} else {
		if (W25qxx_IsEmptySector(Sector_Address + 1, OffsetInByte,
				NumByteToWrite_up_to_SectorSize) != true) {
			return ((Sector_Address + 1) * 16);
		}
	}

	return 0;
}

HAL_StatusTypeDef W25Q64_read_mark(uint8_t channel, uint32_t *mark_cnt) {
//	uint32_t i = 0;
	uint32_t j = 0;
//	uint32_t k = 0;
	uint8_t buff[256];
//	uint32_t check = 0;
	uint32_t begin = mem_addr_ch_index_begin(channel);
	uint32_t finish = mem_addr_ch_index_end(channel);
	uint32_t Page_Address = begin / 256;
	uint32_t res;
//	uint32_t Sector_Address = begin / 4096;
//	uint32_t OffsetInByte = 0;
//	uint32_t NumByteToWrite_up_to_PageSize = 0;
//	uint32_t NumByteToWrite_up_to_SectorSize = 0;
//	uint8_t data = 0;
//	uint32_t OffsetInByte = 0;
//	uint32_t NumByteToWrite_up_to_PageSize = 256;

// 0000-0000 1111-1110
// 0000-0000 1111-1111
// 1111-1111 1111-1111

	/*
	 #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
	 #define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
	 */

//	UsrLog("channel: %d", channel);
	if (read_mark_steps < ((finish + 1) / 256)) {
		if (read_mark_steps < Page_Address) {
			read_mark_steps = Page_Address;
		}

//		UsrLog("cnt: %lu", read_mark_steps - (begin / 256));

//		UsrLog("cnt: %lu", ((read_mark_steps - ((begin / 256) + 1)) * 8) + (j * 8));

		IWDG_delay_ms(0);

		W25qxx_ReadPage(buff, read_mark_steps, 0, 256);

		for (j = 0; j < 256; j++) {
//			res = ((read_mark_steps - Page_Address) * 256 * 8)
//											+ (j * 8);

//			UsrLog("res: %lu", res);

//			UsrLog("Read page 0x%02lX buff[%lu]: "BYTE_TO_BINARY_PATTERN,
//					read_mark_steps, j, BYTE_TO_BINARY(buff[j]));

//			UsrLog("mark_cnt: %d %lu 0x%05lX 0x%02X", channel, j, read_mark_steps, buff[j]);
			if (buff[j] == 0xFF) {
//				UsrLog("mark_cnt: %d 0xFF", channel);
				res = ((read_mark_steps - Page_Address) * 256 * 8) + (j * 8);

//				UsrLog("mark_cnt[%d]: %lu", channel, res);

				mark_cnt[channel] = res;
//
				read_mark_steps = 0;

				return HAL_OK;

			} else if (buff[j] != 0x00) {
				if (buff[j] == 0b11111110) {
					previous_cell_index = 1;
				} else if (buff[j] == 0b11111100) {
					previous_cell_index = 2;
				} else if (buff[j] == 0b11111000) {
					previous_cell_index = 3;
				} else if (buff[j] == 0b11110000) {
					previous_cell_index = 4;
				} else if (buff[j] == 0b11100000) {
					previous_cell_index = 5;
				} else if (buff[j] == 0b11000000) {
					previous_cell_index = 6;
				} else if (buff[j] == 0b10000000) {
					previous_cell_index = 7;
				}

//				UsrLog("mark_cnt: %d ", channel);
				res = ((read_mark_steps - Page_Address) * 256 * 8) + (j * 8)
						+ previous_cell_index;

//				UsrLog("mark_cnt[%d]: %lu", channel, res);

				mark_cnt[channel] = res;
//

				read_mark_steps = 0;
				return HAL_OK;
			}
		}

		read_mark_steps++;
	} else if (read_mark_steps == ((finish + 1) / 256)) {
		read_mark_steps = 0;
		return HAL_ERROR;
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_write_data(uint8_t channel, uint32_t index_cnt,
		DadaStruct_TypeDef *data_in) {

//	return HAL_OK;
//	uint32_t i = 0;
//	uint32_t j = 0;
//	uint32_t k = 0;
	uint8_t buff;
//	uint32_t check = 0;
	uint32_t begin = mem_addr_ch_data_begin(channel) + (index_cnt * 19);
	uint32_t finish = begin + 19;
//	uint8_t data = 0;
//	uint32_t OffsetInByte = 0;
//	uint32_t NumByteToWrite_up_to_PageSize = 256;
	uint8_t data[19];

	data[0] = data_in->Temperature[0];
	data[1] = data_in->Temperature[1];
	data[2] = data_in->Sensor_Id[0];
	data[3] = data_in->Sensor_Id[1];
	data[4] = data_in->Sensor_Id[2];
	data[5] = data_in->Sensor_Id[3];
	data[6] = data_in->Sensor_Id[4];
	data[7] = data_in->Sensor_Id[5];
	data[8] = data_in->Sensor_Id[6];
	data[9] = data_in->Sensor_Id[7];
	data[10] = data_in->AccX;
	data[11] = data_in->AccY;
	data[12] = data_in->BattV;
	data[13] = data_in->ChargerV;
	//data[14] = data_in->Event;
	data[14] = data_in->Date[0];
	data[15] = data_in->Date[1];
	data[16] = data_in->Date[2];
	data[17] = data_in->Time[0];
	data[18] = data_in->Time[1];

//	UsrLog("ch: %d index: %lu", channel, index_cnt);

	if (write_data_steps < finish) {
		if (write_data_steps < begin) {
			write_data_steps = begin;
		}

		IWDG_delay_ms(0);

		//UsrLog("Block 64KiB zero fill addr: %05X", zerofill_steps1 * 65536);

		W25qxx_WriteByte(data[write_data_steps - begin], write_data_steps);

		W25qxx_ReadByte(&buff, write_data_steps);

		if (buff != data[write_data_steps - begin]) {
//			UsrLog("Error 0x%02X != 0x%02X step %lu addr %lu index %lu", buff,
//					data[write_data_steps - begin], write_data_steps - begin,
//					write_data_steps, index_cnt);
			write_data_steps = 0;
			return HAL_ERROR;
		} else {
//			UsrLog("Ok 0x%02X == 0x%02X step %lu addr %lu index %lu", buff,
//					data[write_data_steps - begin], write_data_steps - begin,
//					write_data_steps, index_cnt);
		}

		write_data_steps++;
	} else {
		write_data_steps = 0;
		return HAL_OK;
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_read_data_sensor_ID(uint8_t channel,
		uint32_t index_cnt, DadaStruct_TypeDef *data_out) {
	//	uint32_t i = 0;
	//	uint32_t j = 0;
	//	uint32_t k = 0;
	uint8_t buff;
	//	uint32_t check = 0;
	uint32_t begin = mem_addr_ch_data_begin(channel) + (index_cnt * 19);
	uint32_t finish = begin + 19;
	//	uint8_t data = 0;
	//	uint32_t OffsetInByte = 0;
	//	uint32_t NumByteToWrite_up_to_PageSize = 256;

	if (read_data_steps < finish) {
		if (read_data_steps < begin) {
			read_data_steps = begin;
		}

		IWDG_delay_ms(0);

		//UsrLog("Block 64KiB zero fill addr: %05X", zerofill_steps1 * 65536);

		//		W25qxx_WriteByte(data[read_data_steps - begin], begin + index_cnt);

		W25qxx_ReadByte(&buff, read_data_steps);

		dataReadBuff[read_data_steps - begin] = buff;

//		UsrLog("index %lu buff 0x%02X", read_data_steps - begin, buff);

		//		if (buff != data[read_data_steps - begin]) {
		//			return HAL_ERROR;
		//		}

		read_data_steps++;
	} else {
		for (int i = 2; i <= 18; i++) {
			if (dataReadBuff[i] == 255) {
				dataReadBuff[i] = 0;
			}
		}

//		data_out->Temperature[0] = dataReadBuff[0];
//		data_out->Temperature[1] = dataReadBuff[1];
		data_out->Sensor_Id[0] = dataReadBuff[2];
		data_out->Sensor_Id[1] = dataReadBuff[3];
		data_out->Sensor_Id[2] = dataReadBuff[4];
		data_out->Sensor_Id[3] = dataReadBuff[5];
		data_out->Sensor_Id[4] = dataReadBuff[6];
		data_out->Sensor_Id[5] = dataReadBuff[7];
		data_out->Sensor_Id[6] = dataReadBuff[8];
		data_out->Sensor_Id[7] = dataReadBuff[9];
//		data_out->AccX = dataReadBuff[10];
//		data_out->AccY = dataReadBuff[11];
//		data_out->BattV = dataReadBuff[12];
//		data_out->ChargerV = dataReadBuff[13];
		//data_out->Event = dataReadBuff[14];
//		data_out->Date[0] = dataReadBuff[14];
//		data_out->Date[1] = dataReadBuff[15];
//		data_out->Date[2] = dataReadBuff[16];
//		data_out->Time[0] = dataReadBuff[17];
//		data_out->Time[1] = dataReadBuff[18];

		read_data_steps = 0;
		return HAL_OK;
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_read_data(uint8_t channel, uint32_t index_cnt,
		DadaStruct_TypeDef *data_out) {
	//	uint32_t i = 0;
	//	uint32_t j = 0;
	//	uint32_t k = 0;
	uint8_t buff;
	//	uint32_t check = 0;
	uint32_t begin = mem_addr_ch_data_begin(channel) + (index_cnt * 19);
	uint32_t finish = begin + 19;
	//	uint8_t data = 0;
	//	uint32_t OffsetInByte = 0;
	//	uint32_t NumByteToWrite_up_to_PageSize = 256;

	if (read_data_steps < finish) {
		if (read_data_steps < begin) {
			read_data_steps = begin;
		}

		IWDG_delay_ms(0);

		//UsrLog("Block 64KiB zero fill addr: %05X", zerofill_steps1 * 65536);

		//		W25qxx_WriteByte(data[read_data_steps - begin], begin + index_cnt);

		W25qxx_ReadByte(&buff, read_data_steps);

		dataReadBuff[read_data_steps - begin] = buff;

//		UsrLog("index %lu buff 0x%02X", read_data_steps - begin, buff);

		//		if (buff != data[read_data_steps - begin]) {
		//			return HAL_ERROR;
		//		}

		read_data_steps++;
	} else {
		for (int i = 2; i <= 18; i++) {
			if (dataReadBuff[i] == 255) {
				dataReadBuff[i] = 0;
			}
		}

		data_out->Temperature[0] = dataReadBuff[0];
		data_out->Temperature[1] = dataReadBuff[1];
		data_out->Sensor_Id[0] = dataReadBuff[2];
		data_out->Sensor_Id[1] = dataReadBuff[3];
		data_out->Sensor_Id[2] = dataReadBuff[4];
		data_out->Sensor_Id[3] = dataReadBuff[5];
		data_out->Sensor_Id[4] = dataReadBuff[6];
		data_out->Sensor_Id[5] = dataReadBuff[7];
		data_out->Sensor_Id[6] = dataReadBuff[8];
		data_out->Sensor_Id[7] = dataReadBuff[9];
		data_out->AccX = dataReadBuff[10];
		data_out->AccY = dataReadBuff[11];
		data_out->BattV = dataReadBuff[12];
		data_out->ChargerV = dataReadBuff[13];
		//data_out->Event = dataReadBuff[14];
		data_out->Date[0] = dataReadBuff[14];
		data_out->Date[1] = dataReadBuff[15];
		data_out->Date[2] = dataReadBuff[16];
		data_out->Time[0] = dataReadBuff[17];
		data_out->Time[1] = dataReadBuff[18];

		read_data_steps = 0;
		return HAL_OK;
	}

	return HAL_BUSY;
}

HAL_StatusTypeDef W25Q64_update_config(ConfigStruct_TypeDef *config) {
	uint8_t buff[256];
	uint32_t begin = mem_addr_ch_rsv_begin(7);
	uint32_t Page_Address = begin / 256;
	uint32_t Sector_Address = begin / 4096;
	uint32_t OffsetInByte = 0;
	uint32_t NumByteToWrite_up_to_PageSize = 0;
	uint32_t NumByteToWrite_up_to_SectorSize = 0;
	uint8_t test_count = 0;
	uint8_t maxIndex = 72; // ConfigStruct_TypeDef / bytes
	uint8_t config_tmp[maxIndex];

//	uint16_t FileCntDat; 2
//	uint16_t FileCntCsv; 4
//	uint8_t FileType; 5
//	uint8_t DateFormat; 6
//	uint8_t DecimalSeparator; 7
//	uint8_t TemperatureUnit; 8
//	uint8_t Language; 9
//	uint8_t Key[32]; 41
//	char PW_Code[6]; 47
//	uint16_t SensorOffset[maxChannel]; 47 + (8 * 2) = 63

//	UsrLog("W25Q64_update_config");

	config_tmp[0] = config->FileCntDat & 0xFF;
	config_tmp[1] = (config->FileCntDat >> 8) & 0xFF;

	config_tmp[2] = config->FileCntCsv & 0xFF;
	config_tmp[3] = (config->FileCntCsv >> 8) & 0xFF;

	config_tmp[4] = config->FileType;
	config_tmp[5] = config->DateFormat;
	config_tmp[6] = config->DecimalSeparator;
	config_tmp[7] = config->TemperatureUnit;
	config_tmp[8] = config->Language;

	for (int i = 0; i < 32; i++) {
		config_tmp[i + 9] = config->Key[i];
	}

	for (int i = 0; i < 6; i++) {
		config_tmp[i + 9 + 32] = config->PW_Code[i];
	}

	for (int i = 0; i < 8; i++) {
		config_tmp[i + 9 + 32 + 6] = config->SensorOffset[i] / 256; // 6 + 9 + 32 + 8 = 55
	}

	for (int i = 0; i < 8; i++) {
		config_tmp[i + 9 + 32 + 6 + 8] = config->SensorOffset[i]; // 6 + 9 + 32 + 8 + 8 = 63
		config_tmp[i + 9 + 32 + 6 + 8] -= config_tmp[i + 9 + 32 + 6] * 256;
	}

	for (int i = 0; i < 8; i++) {
		config_tmp[i + 9 + 32 + 6 + 8 + 8] = config->ChExported[i] & 1; // 6 + 9 + 32 + 8 + 8 + 8 = 71
	}

//	UsrLog("W25Q64_update_config 1");

//	UsrLog("Page_Address %lu", Page_Address);

//	UsrLog("w25qxx.Lock %d", w25qxx.Lock);

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	UsrLog("W25Q64_update_config 2");

	test_count = 0;

	for (int i = 0; i < maxIndex; i++) {
//		UsrLog("1) Update config[%d]: %02d buff: %02d", i, config_tmp[i],
//				buff[i]);

//		if (i == 0) {
		if (config_tmp[i] != buff[i]) {
			test_count++;
		}
//		}

	}

//	UsrLog("W25Q64_update_config 3");

	if (test_count == 0) {
//		UsrLog("1) Name OK");
		return HAL_OK;
	}

//	UsrLog("W25Q64_update_config 4");

//	UsrLog("CH%d Name: %s", channel, name);

	for (int i = 0; i < maxIndex; i++) {
		buff[i] = config_tmp[i];
	}

//	UsrLog("W25Q64_update_config 5");

	if (W25qxx_IsEmptySector(Sector_Address, OffsetInByte,
			NumByteToWrite_up_to_SectorSize) != true) {  // 30ms
//		UsrLog("Update 1: Is not empty sector");

		W25qxx_EraseSector(Sector_Address);  // 30ms

		if (W25qxx_IsEmptySector(Sector_Address, OffsetInByte,
				NumByteToWrite_up_to_SectorSize) != true) {  // 30ms
//			UsrLog("Update 2: Is not empty sector");

//			UsrLog("2) Name not OK");
			return HAL_ERROR;
		}
	}

//	UsrLog("W25Q64_update_config 6");

	W25qxx_WritePage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	UsrLog("W25Q64_update_config 7");

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

//	UsrLog("W25Q64_update_config 8");

	test_count = 0;

	for (int i = 0; i < maxIndex; i++) {
//		UsrLog("2) Update buff[%d]: %d", i, buff[i]);
		if (config_tmp[i] != buff[i]) {
			test_count++;
		}
	}

//	UsrLog("W25Q64_update_config 9");

	if (test_count != 0) {
//		UsrLog("3) Name not OK");
		return HAL_ERROR;
	}

//	UsrLog("W25Q64_update_config 10");

//	UsrLog("4) Name OK");

	return HAL_OK;
}

HAL_StatusTypeDef W25Q64_read_config(ConfigStruct_TypeDef *config) {
	uint8_t buff[256];
//	char buffStr[maxChar + 1];

	uint32_t begin = mem_addr_ch_rsv_begin(7);
	uint32_t Page_Address = begin / 256;
//	uint32_t Sector_Address = begin / 4096;

	uint32_t OffsetInByte = 0;
	uint32_t NumByteToWrite_up_to_PageSize = 256;
//	uint32_t NumByteToWrite_up_to_SectorSize = 0;
	uint8_t maxIndex = 72; // ConfigStruct_TypeDef / bytes
//	uint8_t date_time_tmp[11];

	W25qxx_ReadPage(buff, Page_Address, OffsetInByte,
			NumByteToWrite_up_to_PageSize);

	for (int i = 0; i < maxIndex; i++) {
		if (buff[i] == 255) { // erased state
			buff[i] = 0;
		}
	}

	config->FileCntDat = buff[0]; // 1
	config->FileCntDat += buff[1] * 256; // 2

	config->FileCntCsv = buff[2]; // 3
	config->FileCntCsv += buff[3] * 256; // 4

	config->DateFormat = buff[4];
	config->DateFormat = buff[5];
	config->DecimalSeparator = buff[6];
	config->TemperatureUnit = buff[7];
	config->Language = buff[8];

	for (int i = 0; i < 32; i++) {
		config->Key[i] = buff[i + 9];
	}

	for (int i = 0; i < 6; i++) {
		config->PW_Code[i] = buff[i + 9 + 32]; // 6 + 9 + 32 = 47
	}

	for (int i = 0; i < 8; i++) {
		config->SensorOffset[i] = buff[i + 9 + 32 + 6] * 256; // 6 + 9 + 32 + 8 = 55
	}

	for (int i = 0; i < 8; i++) {
		config->SensorOffset[i] += buff[i + 9 + 32 + 6 + 8]; // 6 + 9 + 32 + 8 + 8 = 63
	}

	for (int i = 0; i < 8; i++) {
		config->ChExported[i] = buff[i + 9 + 32 + 6 + 8 + 8] & 1; // 6 + 9 + 32 + 8 + 8 + 8 = 71
	}

//UsrLog("CH%d Name: %s", channel, buffStr);

	return HAL_OK;
}
