#include "msp430.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "dcocal.h"
#include "uart.h"

#define LED BIT6
#define PIN_S2 BIT3

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum Channels {
	CH_R = INCH_0,
	CH_G = INCH_2,
	CH_B = INCH_5, // TODO back to 3?
	CH_L = INCH_4
};

enum OutputPins {
	OUT_R = 1 << 5,
	OUT_G = 1 << 6,
	OUT_B = 1 << 7,
};

struct {
	uint8_t count;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t l;
} pwm;

int adcSampling;
bool samplingBusy;

void initClocks() {
	// stop WDT, run at 16 MHz
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;
}

void initAdc() {
	// internal 1.5V reference, inputs on P1.0, 1.2, 1.3, 1.4
	ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
	ADC10AE0 = BIT0 | BIT2 | BIT5 | BIT4;
}

void initPins() {
	// red, green, blue, and TXD outputs
	P1DIR = OUT_R | OUT_G | OUT_B | UART_PIN | LED;
}

void sample(int channel) {
	printx("Sample ", channel);
	adcSampling = channel;
	ADC10CTL1 = channel;
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	__bis_SR_register(CPUOFF + GIE);        // LPM0 until the sampling completes
}

void sampleDone(int channel, int value) {
	printx("Ch: ", channel);
	printv("Val:", value);

	switch (channel) {
		case CH_R:
			pwm.r = MIN(255, value);
			sample(CH_G);
			break;

		case CH_G:
			pwm.g = MIN(255, value);
			sample(CH_B);
			break;

		case CH_B:
			printv("B: ", pwm.b);
			pwm.b = MIN(255, value);
			//printv("R: ", pwm.r);
			//printv("G: ", pwm.g);
			printv("B: ", pwm.b);
			samplingBusy = false;
			break;

		case CH_L:
			pwm.l = value;
			//sample(CH_R);
			break;
	}	
}

void pwmPin(int count, int val, int pin) {
	if (val > count)
		P1OUT |= pin;
	else
		P1OUT &= ~pin;
}

void doPwm() {
	pwm.count++;
	pwmPin(pwm.count, pwm.r, OUT_R);
	pwmPin(pwm.count, pwm.g, OUT_G);
	pwmPin(pwm.count, pwm.b, OUT_B);
}

int main(void)
{
	initClocks();
	initAdc();
	initPins();
	prints("init\n");

	P1REN |= PIN_S2;
	P1OUT |= PIN_S2;
	P1IFG &= ~PIN_S2;

	for (;;) {
		P1IE |= PIN_S2;
		//__bis_SR_register(CPUOFF + GIE);        // sleep until button pressed
		LPM0;
		prints("go!\n");
		P1IE &= ~PIN_S2;	// button off
		samplingBusy = true;
		sample(CH_R);
		while (samplingBusy) {
			LPM0;
		}
	}

	return 0;
}

__attribute__((interrupt(PORT1_VECTOR)))
void p1() {
	if (P1IFG & PIN_S2) {
		P1IFG &= ~PIN_S2;
		prints("button\n");
		if (!samplingBusy) {
		}

		LPM0_EXIT;
	}
}

__attribute__((interrupt(ADC10_VECTOR)))
void adc10() {
	//__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
	LPM0_EXIT;
	sampleDone(adcSampling, ADC10MEM);
}

/*
__attribute__((interrupt(TIMERA0_VECTOR)))
void ta0() {
	TACTL = 0;
	LPM0_EXIT;                                // Exit LPM0 on return
}
*/

