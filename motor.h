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
#define TMR2_PRESCALAR          4 // i.e. assume 1:4 TMR2 prescalar
#define N_100us                 1 // i.e. 100us period or 10KHz PWM frequency
#define PWM_PERIOD              COUNTS_PER_N_100us_INTERVALS(N_100us, TMR2_PRESCALAR)
#define PWM_FULL_DUTYCYCLE      COUNTS_PER_N_100us_INTERVALS(N_100us, TMR2_PRESCALAR/4) // i.e. TMR2 is extended to 10-bits with 2-bits of prescalar

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

