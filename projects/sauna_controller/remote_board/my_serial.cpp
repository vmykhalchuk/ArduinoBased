#include "my_serial.h"

void uart_init(unsigned long baud) {
  // Calculate UBRR value for the baud rate
  unsigned int ubrr = (F_CPU / 16 / baud) - 1;
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;

  // Enable transmitter only (UCSR0B)
  UCSR0B = (1 << TXEN0);

  // Set frame format: 8 data bits, 1 stop bit (UCSR0C)
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send_byte(uint8_t data) {
  // Wait for the Transmit Data Buffer to be empty (UDRE flag)
  while (!(UCSR0A & (1 << UDRE0)));
  
  // Put data into the buffer, which sends the byte
  UDR0 = data;
}

bool is_uart_idle() {
  // Returns true only if the hardware is completely finished
  if (UCSR0A & (1 << TXC0)) {
    UCSR0A |= (1 << TXC0); // Clear the flag
    return true;
  }
  return false;
}