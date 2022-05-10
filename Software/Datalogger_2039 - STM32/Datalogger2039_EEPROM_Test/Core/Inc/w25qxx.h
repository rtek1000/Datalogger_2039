// https://github.com/nimaltd/w25qxx

/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Nima Askari, 2018
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

#ifndef INC_W25QXX_H_
#define INC_W25QXX_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"

//#include "spi.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef enum
{
	W25Q10=1,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,
	W25Q128,
	W25Q256,
	W25Q512,

}W25QXX_ID_t;

typedef struct
{
	W25QXX_ID_t	ID;
	uint8_t			UniqID[8];

	uint32_t    	PageSize;
	uint32_t		PageCount;
	uint32_t		SectorSize;
	uint32_t		SectorCount;
	uint32_t		BlockSize;
	uint32_t		BlockCount;

	uint32_t		CapacityInKiloByte;

	uint8_t	StatusRegister1;
	uint8_t	StatusRegister2;
	uint8_t	StatusRegister3;

	uint8_t	Lock;

}w25qxx_t;

extern w25qxx_t	w25qxx;
//############################################################################
// in Page,Sector and block read/write functions, can put 0 to read maximum bytes
//############################################################################
bool	W25qxx_Init(void);

uint32_t 	W25qxx_ReadID(void);
void 	W25qxx_ReadUniqID(void);

void	W25qxx_EraseChip(void);
void 	W25qxx_EraseSector(uint32_t SectorAddr);
void 	W25qxx_EraseBlock64(uint32_t BlockAddr);
void	W25qxx_EraseBlock32(uint32_t BlockAddr);

uint32_t	W25qxx_PageToSector(uint32_t	PageAddress);
uint32_t	W25qxx_PageToBlock(uint32_t	PageAddress);
uint32_t	W25qxx_SectorToBlock(uint32_t	SectorAddress);
uint32_t	W25qxx_SectorToPage(uint32_t	SectorAddress);
uint32_t	W25qxx_BlockToPage(uint32_t	BlockAddress);

bool 	W25qxx_IsEmptyPage(uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_PageSize);
bool 	W25qxx_IsEmptySector(uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_SectorSize);
bool 	W25qxx_IsEmptyBlock(uint32_t Block_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_BlockSize);
bool 	W25qxx_IsBusy(void);

void 	W25qxx_WriteByte(uint8_t pBuffer,uint32_t Bytes_Address);
void 	W25qxx_WritePage(uint8_t *pBuffer	,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToWrite_up_to_PageSize);
void 	W25qxx_WriteSector(uint8_t *pBuffer,uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToWrite_up_to_SectorSize);
void 	W25qxx_WriteBlock(uint8_t* pBuffer,uint32_t Block_Address,uint32_t OffsetInByte,uint32_t NumByteToWrite_up_to_BlockSize);

void	W25qxx_WriteSectorZeroFill(uint32_t Sector_Address);
void	W25qxx_WriteBlockZeroFill(uint32_t Block_Address);

void 	W25qxx_ReadByte(uint8_t *pBuffer,uint32_t Bytes_Address);
void 	W25qxx_ReadBytes(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead);
void 	W25qxx_ReadPage(uint8_t *pBuffer,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_PageSize);
void 	W25qxx_ReadSector(uint8_t *pBuffer,uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_SectorSize);
void 	W25qxx_ReadBlock(uint8_t* pBuffer,uint32_t Block_Address,uint32_t OffsetInByte,uint32_t	NumByteToRead_up_to_BlockSize);
//############################################################################
#ifdef __cplusplus
}
#endif

#endif /* INC_W25QXX_H_ */
