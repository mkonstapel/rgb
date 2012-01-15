#ifndef UART_H
#define UART_H

#define UART_BITTIME_1MHZ 104 - 11
#define UART_BITTIME_16MHZ 104*16

#define UART_PIN BIT1	// P1.x
#define UART_BITTIME UART_BITTIME_16MHZ

void printc(char c);
void prints(const char* s);
void printi(int i, int base);
void printv(const char* s, int i);
void printx(const char* s, int i);

#endif

