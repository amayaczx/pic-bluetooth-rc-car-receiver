/* 
 * File:   main.c
 * 
 * @author: krisdover@hotmail.com
 *
 * @change Created on February 10, 2013, 9:08 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "motor.h"
#include "servo.h"
#include "uart.h"


int controller_input_timeout = 0;

void init_osc() {
    // setup INTRC frequency
    OSCCONbits.IRCF = 0b111; // enable 8 Mhz internal clock
    while (!IOFS)
        continue; // wait for frequency stable flag
}

void init_interrupts() {
    GIE = 1; // global interrupt enabled
    PEIE = 1; // peripheral interrupts enabled
}

/*------------------------ sys management routines -----------------------*/

void interrupt handle_int() {
    // servo pulse timer events: transition PWM output state
    if (TMR1IE && TMR1IF) {
        TMR1IF = 0;
        apply_next_pwm_state();
        pwm_cycle_cnt_debug++;

        // leverage servo PWM TMR1 to reset controls after input timeout
        if (pwm_current_state == 1) { // this is true every 20ms
            if (--controller_input_timeout == 0) {
                set_motor_speed_and_dir(0);
                set_servo_position(CENTER_SERVO_POS);
            }
        }
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
    init_osc();
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
    printf("_100us_PWM_PERIOD: %u\r\n", PWM_PERIOD);

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

