/*
 * DS3231.h
 *
 *  Created on: 21 de jun de 2021
 *      Author: r
 */

#ifndef INC_DS3231_H_
#define INC_DS3231_H_

typedef struct {
	uint8_t seconds; /*!< Seconds parameter, from 00 to 59 */
	uint8_t minutes; /*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;   /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t day;     /*!< Day in a week, from 1 to 7 */
	uint8_t date;    /*!< Date in a month, 1 to 31 */
	uint8_t month;   /*!< Month in a year, 1 to 12 */
	uint8_t year;    /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
} DS3231_Time_t;

#endif /* INC_DS3231_H_ */
