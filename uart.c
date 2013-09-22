#include "uart.h"

/*------------------------ UART IO routines -----------------------*/
char putch_buffer[MAX_BUFFER_SIZE];
int head = 0;
int tail = 0;


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

void putch(char data) {
    putch_buffer[tail++] = data;
    if (tail == MAX_BUFFER_SIZE) {
        tail = 0;
    }
    if (!TXIE) {
        TXIE = 1;
    }
}