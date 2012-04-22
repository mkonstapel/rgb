#include "msp430.h"
#include "stdio.h"
#include "stdlib.h"
#include "dcocal.h"
#include "uart.h"

#define SEND(b) if (b) P1OUT |= UART_PIN; else P1OUT &= ~UART_PIN; __delay_cycles(UART_BITTIME);

static char formatbuf[8];

void printc(char c) {
	__dint();
	SEND(0);	// start bit
	SEND(c & BIT0);
	SEND(c & BIT1);
	SEND(c & BIT2);
	SEND(c & BIT3);
	SEND(c & BIT4);
	SEND(c & BIT5);
	SEND(c & BIT6);
	SEND(c & BIT7);
	SEND(1);	// stop bit	
	__eint();
}

void prints(const char* s) {
	while (*s) {
		printc(*s);
		s++;
	}
}

void printi(int i, int base) {
	itoa(i, formatbuf, base);
	prints(formatbuf);
}

void printv(const char* s, int i) {
	prints(s);
	printi(i, 10);
	printc('\n');
}

void printx(const char* s, int i) {
	prints(s);
	prints("0x");
	printi(i, 16);
	printc('\n');
}

#ifdef UART_TEST_MAIN
int main() {
	uint8_t count = 0;

	// WDT
	WDTCTL = WDTPW + WDTHOLD;		// Stop WDT

	// clocks
	BCSCTL1 = CALBC1_1MHZ;	// 1 MHz preset
	DCOCTL = CALDCO_1MHZ; 	// 1 MHz preset
	BCSCTL3 |= XCAP_3;	// ~12.5 pF

	// TX on 1.1
	P1DIR |= UART_PIN;
	P1OUT |= UART_PIN;

	// SMCLK on 1.4
	P1DIR |= BIT4;
	P1SEL |= BIT4;

	while (1) {
		//prints("UUUUUUUU");
		printv("Count: ", count++);
	}

	return 0;
}
#endif

