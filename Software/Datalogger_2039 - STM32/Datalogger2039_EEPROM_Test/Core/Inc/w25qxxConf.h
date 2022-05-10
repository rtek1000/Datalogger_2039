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

#ifndef INC_W25QXXCONF_H_
#define INC_W25QXXCONF_H_

#include "main.h"

extern SPI_HandleTypeDef hspi1;

#define		_W25QXX_SPI										hspi1
#define		_W25QXX_CS_GPIO								FLASH_CS_GPIO_Port
#define		_W25QXX_CS_PIN								FLASH_CS_Pin
#define		_W25QXX_USE_FREERTOS					0
#define		_W25QXX_DEBUG									0

#endif /* INC_W25QXXCONF_H_ */
