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

/* Calculates the count a given TMR can make within an N * 100us interval
 * COUNTS = [TMR clock freq = (Fosc/4)/PRESCALAR] * N * [100us = 1/10000 s]
 */
#define COUNTS_PER_N_100us_INTERVALS(N,PRESCALAR) (((N) * _XTAL_FREQ) / (4 * PRESCALAR * 10000ul))

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_H */

