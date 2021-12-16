/* Ref.:
 * https://github.com/MYaqoobEmbedded/STM32-Tutorials/tree/master/Tutorial%2030%20-%20FLASH%20Memory
 * */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

#include "stm32f4xx_hal_def.h"

/* Exported functions ------------------------------------------------------- */
void EE_ReadVariable(uint32_t VirtAddress, uint8_t *Data);
void EE_WriteVariable(uint32_t VirtAddress, uint8_t Data);
void EE_Erase_Sector(void);

#endif /* __EEPROM_H */
