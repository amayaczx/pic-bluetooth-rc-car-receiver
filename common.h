/* 
 * File:   common.h
 * Author: Kris
 *
 * Created on September 21, 2013, 10:57 PM
 */

#ifndef COMMON_H
#define	COMMON_H

#include "config.h"
#include "routines.h"

// generic macro & constants
#define BAUD_RATE               19200
#define _XTAL_FREQ              8000000ul // i.e. 8 MHz Fosc
#define TMR1_MAX_COUNT          0xfffful
#define TMR1_PRESCALAR          8 // i.e. assume 1:8 TMR1 prescalar
#define TMR2_PRESCALAR          4 // i.e. assume 1:4 TMR2 prescalar
#define COUNTS_PER_N_100us_INTERVALS(N,PRESCALAR) ((N * _XTAL_FREQ) / (4 * PRESCALAR * 10000ul)) // i.e. TMR clock = (Fosc/4)

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_H */

