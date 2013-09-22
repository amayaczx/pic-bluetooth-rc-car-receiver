/* 
 * File:   uart.h
 * Author: Kris
 *
 * Created on September 21, 2013, 11:28 PM
 */

#ifndef UART_H
#define	UART_H

#include "common.h"

// UART specific constants
#define MAX_BUFFER_SIZE         30

#ifdef	__cplusplus
extern "C" {
#endif

extern char putch_buffer[];
extern int head;
extern int tail;

void init_uart();
void putch(char data);

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

