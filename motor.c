#include "motor.h"

/*------------------------ Motor PWM Speed Controller -----------------------*/


int motor_speed = 0;
int motor_duty_cycle = 0;


#pragma interrupt_level 1
void set_motor_speed_and_dir(char speed_and_dir) {
    RB1 = (speed_and_dir >> 5) & 0x1; // set motor direction using bit 5
    motor_speed = speed_and_dir & MOTOR_SPEED_MASK; // set motor speed using bits 4:0
    motor_duty_cycle = (_100us_PERIOD_10BIT_RES * motor_speed) >> 5; // optimised division by max motor speed => MOTOR_SPEED_MASK;
    CCPR1L = motor_duty_cycle >> 2; // set duty cycle reg top 8 most significant bits
    CCP1CON = ((0b11 & motor_duty_cycle) << 4) | (0xf & CCP1CON); // set duty cycle reg bottom 2 least significant bits
}

void init_ccp() {
    // enable compare mode
    CCP1CONbits.CCP1M = 0b1100; // CCP module in PWM mode
}

void init_motor_ctrl_outputs() {
    TRISB0 = 0; // enable PWM output on RB0/CCP1
    TRISB1 = 0; // set RB1 as motor direction output
    set_motor_speed_and_dir(0);
}

void init_timer2() {
    T2CONbits.T2CKPS = 0b01; // 1:4 prescalar
    PR2 = _100us_PWM_PERIOD;
    TMR2IE = 0; // disable TMR2 interrupts
    TMR2ON = 1; // enable TMR2
}