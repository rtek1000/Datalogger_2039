/*
 * log_uart.h
 *
 *  Created on: Jun 15, 2023
 *      Author: user
 */

#ifndef INC_LOG_UART_H_
#define INC_LOG_UART_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

#ifndef UsrLog
#define  UsrLog(...)   do { \
                            printf(__VA_ARGS__); \
                            printf("\n"); \
} while (0)
#endif

#ifndef SerialSend
#define  SerialSend(...)   do { \
		printf(__VA_ARGS__); \
		printf("\r\n"); \
 } while (0)
#endif


#endif /* INC_LOG_UART_H_ */
