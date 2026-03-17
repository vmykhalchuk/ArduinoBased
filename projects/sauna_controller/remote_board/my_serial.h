#ifndef MY_SERIAL_H
#define MY_SERIAL_H

#include <Arduino.h>

void uart_init(unsigned long baud);
void uart_send_byte(uint8_t data);
bool is_uart_idle();

#endif