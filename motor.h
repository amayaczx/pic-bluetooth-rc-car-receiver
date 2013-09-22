/* 
 * File:   motor.h
 * Author: Kris
 *
 * Created on September 21, 2013, 10:54 PM
 */

#ifndef MOTOR_H
#define	MOTOR_H

#include "common.h"

// motor specific macros & constants
#define _100us_PWM_PERIOD       COUNTS_PER_N_100us_INTERVALS(10, TMR2_PRESCALAR) // i.e. 1000us or 1KHz
#define _100us_PERIOD_10BIT_RES COUNTS_PER_N_100us_INTERVALS(10, TMR2_PRESCALAR/4) // i.e. TMR2 extended to 10-bits with 2-bits of prescalar

#define MOTOR_SPEED_MASK        0x1f
#define MOTOR_COMMAND           0x40


#ifdef	__cplusplus
extern "C" {
#endif

extern int motor_speed;
extern int motor_duty_cycle;

void set_motor_speed_and_dir(char speed_and_dir);
void init_ccp();
void init_motor_ctrl_outputs();
void init_timer2();

#ifdef	__cplusplus
}
#endif

#endif	/* MOTOR_H */

