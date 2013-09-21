/* 
 * File:   main.c
 * 
 * @author: krisdover@hotmail.com
 *
 * @change Created on February 10, 2013, 9:08 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "routines.h"

// generic macro & constants
#define BAUD_RATE               19200
#define _XTAL_FREQ              8000000ul // i.e. 8 MHz Fosc
#define TMR1_MAX_COUNT          0xfffful
#define TMR1_PRESCALAR          8 // i.e. assume 1:8 TMR1 prescalar
#define TMR2_PRESCALAR          4 // i.e. assume 1:4 TMR2 prescalar
#define COUNTS_PER_N_100us_INTERVALS(N,PRESCALAR) ((N * _XTAL_FREQ) / (4 * PRESCALAR * 10000ul)) // i.e. TMR clock = (Fosc/4)

// UART specific constants
#define MAX_BUFFER_SIZE         30
char putch_buffer[MAX_BUFFER_SIZE];
int head = 0;
int tail = 0;

// servo specific macros & constants
#define _20ms_PWM_PERIOD        COUNTS_PER_N_100us_INTERVALS(200, TMR1_PRESCALAR) // i.e. 200 * 100us = 20ms servo pulse period (20 KHz pulse rate)
#define MAX_PULSE_WIDTH         COUNTS_PER_N_100us_INTERVALS(27, TMR1_PRESCALAR) // i.e. 27 * 100us = 2.7ms
#define MIN_PULSE_WIDTH         COUNTS_PER_N_100us_INTERVALS(5, TMR1_PRESCALAR) // i.e. 5 * 100us = 0.5ms
#define MAX_SERVO_POS           0x3f
#define CENTER_SERVO_POS        (MAX_SERVO_POS + 1) / 2
#define POS_2_WIDTH_RATIO       ((MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / MAX_SERVO_POS)
#define SERVO_POS_2_WIDTH(P)    POS_2_WIDTH_RATIO * P
#define PERIOD_2_TMR1PRESET(P)  (TMR1_MAX_COUNT - (P))
char servo_pos = 0;
int pwm_pulse_width = 0;
int pwm_state_tmr_presets[2]; // a 2-element array which when indexed by 0 -> PWM PULSE_OFF or 1 -> PULSE_ON returns associated timer presets
char pwm_current_state = 0; // current output state i.e. 1 -> PULSE_ON or 0 -> PULSE_OFF
int pwm_cycle_cnt_debug = 0;

#define PULSE_OFF               0
#define PULSE_ON                1
#define INVERT_STATE(S)         (S ^ 0x01)

// motor ctrl specific constants
#define _100us_PWM_PERIOD       COUNTS_PER_N_100us_INTERVALS(10, TMR2_PRESCALAR) // i.e. 1000us or 1KHz
#define _100us_PERIOD_10BIT_RES COUNTS_PER_N_100us_INTERVALS(10, TMR2_PRESCALAR/4) // i.e. TMR2 extended to 10-bits with 2-bits of prescalar

#define MOTOR_SPEED_MASK        0x1f
#define MOTOR_COMMAND           0x40

int motor_speed = 0;
int motor_duty_cycle = 0;

int controller_input_timeout = 0;

void init_osc() {
    // setup INTRC frequency
    OSCCONbits.IRCF = 0b111; // enable 8 Mhz internal clock
    while (!IOFS)
        continue; // wait for frequency stable flag
}

void init_uart() {
    SPBRG = 25; // (_XTAL_FREQ / BAUD_RATE / 16) - 1;
    BRGH = 1; // high speed
    SYNC = 0; // asynchronous mode
    TX9 = 0;  // 8-bit frames
    SPEN = 1; // enable  serial port
    TXEN = 1; // enable transmitter
    CREN = 1; // enable receiver

    RCIF = 0; // clear any RX interrupts
    RCIE = 1; // enable RX interrupts
}

void init_interrupts() {
    GIE = 1; // global interrupt enabled
    PEIE = 1; // peripheral interrupts enabled
}

/*------------------------ UART IO routines -----------------------*/

void putch(char data) {
    putch_buffer[tail++] = data;
    if (tail == MAX_BUFFER_SIZE) {
        tail = 0;
    }
    if (!TXIE) {
        TXIE = 1;
    }
}

/*------------- Servo Steering Controller using Timer driven PWM ----------------*/

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

/*------------------------ Motor PWM Speed Controller -----------------------*/

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

/*------------------------ sys management routines -----------------------*/

void interrupt handle_int() {
    // servo pulse timer events: transition PWM output state
    if (TMR1IE && TMR1IF) {
        TMR1IF = 0;
        apply_next_pwm_state();
        pwm_cycle_cnt_debug++;

        // leverage servo PWM TMR1 to reset controls after input timeout
//        if (pwm_current_state == 1) { // this is true every 20ms
//            if (--controller_input_timeout == 0) {
//                set_motor_speed_and_dir(0);
//                set_servo_position(CENTER_SERVO_POS);
//            }
//        }
        return;
    }

    // UART receiver events: received control instructions on serial
    if (RCIE && RCIF) {
        // read data & clear interrupt flag
        char ctrl_byte = RCREG;
        controller_input_timeout = 50; // 50 * 20ms = 1s timeout
        if (ctrl_byte & MOTOR_COMMAND) {
            set_motor_speed_and_dir(ctrl_byte);
        } else {
            set_servo_position(ctrl_byte);
        }
        return;
    }

    // UART transmitter events: write available data to serial
    if (TXIE && TXIF) {
        if (head != tail) {
            TXREG = putch_buffer[head++];
            if (head == MAX_BUFFER_SIZE) {
                head = 0;
            }
        } else {
            TXIE = 0;
        }
        return;
    }
}

void bootstrap() {
    //init_osc();
    init_interrupts();
    init_uart();

    // init motor pwm
    init_ccp();
    init_timer2();
    init_motor_ctrl_outputs();

    // init servo pwm
    init_servo_pwm_output();
    init_timer1();
}


int main() {
    bootstrap();

    printf("SERVO >>>>>\r\n");
    printf("_20ms_PWM_PERIOD: %lu\r\n", _20ms_PWM_PERIOD);
    printf("MAX_PULSE_WIDTH: %u\r\n", MAX_PULSE_WIDTH);
    printf("MIN_PULSE_WIDTH: %u\r\n", MIN_PULSE_WIDTH);
    printf("MOTOR >>>>\r\n");
    printf("_100us_PWM_PERIOD: %u\r\n", _100us_PWM_PERIOD);

    while (1) {
        printf("SERVO >> pos: %d, pulse-width: %u, cycle-count: %u\r\n",
                servo_pos, pwm_pulse_width, pwm_cycle_cnt_debug);
//        printf("SERVO >> on-time-preset: %u, off-time-preset: %u\r\n",
//                pwm_state_tmr_presets[1],
//                pwm_state_tmr_presets[0]);
        _delay(4000000);
        printf("MOTOR >> speed: %d, direction: %d, duty-cycle: %u\r\n",
                motor_speed, RB1, motor_duty_cycle);
        _delay(4000000);
    }
    
    return (EXIT_SUCCESS);
}

