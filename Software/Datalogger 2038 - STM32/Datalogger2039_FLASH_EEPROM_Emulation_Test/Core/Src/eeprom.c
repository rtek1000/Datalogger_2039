/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"

#define sector_base_address 0x080E0000 // See RM0090 (Pg.75)
#define sector_name 11

void EE_ReadVariable(uint32_t VirtAddress, uint8_t *Data) {
	uint32_t address = sector_base_address + VirtAddress;

	*Data = *(uint8_t *)address;
}

void EE_WriteVariable(uint32_t VirtAddress, uint8_t Data) {
	uint32_t address = sector_base_address + VirtAddress;

	HAL_FLASH_Unlock();

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, Data);

	HAL_FLASH_Lock();
}

void EE_Erase_Sector(void) {
	HAL_FLASH_Unlock();

	FLASH_Erase_Sector(sector_name, FLASH_VOLTAGE_RANGE_3);

	HAL_FLASH_Lock();
}
