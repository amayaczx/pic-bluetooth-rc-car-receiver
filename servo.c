#include "servo.h"


/*------------- Servo Steering Controller using Timer driven PWM ----------------*/

char servo_pos = 0;
int pwm_pulse_width = 0;
int pwm_state_tmr_presets[2]; // a 2-element array which when indexed by 0 -> PWM PULSE_OFF or 1 -> PULSE_ON returns associated timer presets
char pwm_current_state = 0; // current output state i.e. 1 -> PULSE_ON or 0 -> PULSE_OFF
int pwm_cycle_cnt_debug = 0;

#pragma interrupt_level 1
void set_servo_position(char pos) {
    servo_pos = pos & MAX_SERVO_POS;
    pwm_pulse_width = MIN_PULSE_WIDTH + SERVO_POS_2_WIDTH(servo_pos);
    // set the (pulse) ON timer preset; sets pulse period effectively
    pwm_state_tmr_presets[PULSE_ON] = PERIOD_2_TMR1PRESET(pwm_pulse_width);
    // set the OFF timer preset
    pwm_state_tmr_presets[PULSE_OFF] = PERIOD_2_TMR1PRESET(_20ms_PWM_PERIOD - pwm_pulse_width);
}

#pragma interrupt_level 1
void apply_next_pwm_state() {
    char next_state = INVERT_STATE(pwm_current_state); // set output to next state
    RA1 = next_state; // output next state
    TMR1 = pwm_state_tmr_presets[next_state]; // apply next state timer preset
    pwm_current_state = next_state; // save next state as current
}

void init_servo_pwm_output() {
    TRISA1 = 0; // set RA1 as servo PWM output
    set_servo_position(CENTER_SERVO_POS);
    apply_next_pwm_state();
}

/**
 * TMR1 used for servo PWM waveform timing
 */
void init_timer1() {
    TMR1CS = 0; // internal clock source => (FOSC = 8Mhz)/4
    T1CONbits.T1CKPS = 0b11; // apply 1:8 prescalar (gives 250 KHz from 2 Mhz internal clock; or a PWM resolution of 4us)
    TMR1ON = 1; // enable TMR1
    TMR1IF = 0; // clear TMR1 interrupt flag
    TMR1IE = 1; // enable interrupts for TMR1 overflow
}