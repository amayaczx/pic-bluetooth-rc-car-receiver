/* 
 * File:   servo.h
 * Author: Kris
 *
 * Created on September 21, 2013, 11:18 PM
 */

#ifndef SERVO_H
#define	SERVO_H

#include "common.h"

// servo specific macros & constants
#define PULSE_OFF               0
#define PULSE_ON                1
#define TMR1_MAX_COUNT          0xfffful
#define TMR1_PRESCALAR          8 // i.e. assume 1:8 TMR1 prescalar
#define _20ms_PWM_PERIOD        COUNTS_PER_N_100us_INTERVALS(200, TMR1_PRESCALAR) // i.e. 200 * 100us = 20ms servo pulse period (20 KHz pulse rate)
#define MAX_PULSE_WIDTH         COUNTS_PER_N_100us_INTERVALS(27, TMR1_PRESCALAR) // i.e. 27 * 100us = 2.7ms
#define MIN_PULSE_WIDTH         COUNTS_PER_N_100us_INTERVALS(5, TMR1_PRESCALAR) // i.e. 5 * 100us = 0.5ms
#define MAX_SERVO_POS           0x3f
#define CENTER_SERVO_POS        (MAX_SERVO_POS + 1) / 2
#define POS_2_WIDTH_RATIO       ((MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / MAX_SERVO_POS)

#define SERVO_POS_2_WIDTH(P)    POS_2_WIDTH_RATIO * P
#define PERIOD_2_TMR1PRESET(P)  (TMR1_MAX_COUNT - (P))
#define INVERT_STATE(S)         (S ^ 0x01)


#ifdef	__cplusplus
extern "C" {
#endif

extern char servo_pos;
extern int pwm_pulse_width;
extern int pwm_state_tmr_presets[];
extern char pwm_current_state;
extern int pwm_cycle_cnt_debug;

void set_servo_position(char pos);
void apply_next_pwm_state();
void init_servo_pwm_output();
void init_timer1();

#ifdef	__cplusplus
}
#endif

#endif	/* SERVO_H */

