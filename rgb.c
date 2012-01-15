#include "msp430.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "dcocal.h"
#include "uart.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum Channels {
	CH_R = INCH_0,
	CH_G = INCH_2,
	CH_B = INCH_3,
	CH_L = INCH_4
};

enum Port1Pins {
	IN_R = BIT0,
	IN_G = BIT2,
	//IN_B = BIT3,	// TODO enable
	BUTTON  = BIT3,	// TODO move
	IN_L = BIT4,
	OUT_R = BIT5,
	OUT_G = BIT6,
	LED   = BIT6,
	OUT_B = BIT7,
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
	// internal 1.5V reference
	ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
	ADC10AE0 = IN_R | IN_G; // | IN_B | IN_L;	// TODO
}

void initPins() {
	// red, green, blue, TX and LED outputs
	P1DIR = OUT_R | OUT_G | OUT_B | UART_PIN | LED;
}

void sample(int channel) {
	printx("Sample ", channel);
	adcSampling = channel;
	ADC10CTL1 = channel;
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	//LPM0;
	__bis_SR_register(CPUOFF + GIE);        // LPM0 until the sampling completes
}

void sampleDone(int channel, int value) {
	//printx("Ch: ", channel);
	//printv("Val:", value);

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
			pwm.b = MIN(255, value);
			prints("RGB ");
			printi(pwm.r, 10);
			prints(" ");
			printi(pwm.g, 10);
			prints(" ");
			printi(pwm.b, 10);
			prints("\n");
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

	P1REN |= BUTTON;
	P1OUT |= BUTTON;
	P1IFG &= ~BUTTON;
	__eint();

	for (;;) {
		// sleep until button pressed
		P1IE |= BUTTON;
		LPM0;
		prints("go!\n");
		// button off
		P1IE &= ~BUTTON;

		// sample - TODO move to interrupt handler
		samplingBusy = true;
		sample(CH_R);
		while (samplingBusy) {
			LPM0;
		}
		// button on
		P1IE |= BUTTON;
	}

	return 0;
}

__attribute__((interrupt(PORT1_VECTOR)))
void p1() {
	if (P1IFG & BUTTON) {
		P1IFG &= ~BUTTON;
		prints("button\n");
		LPM0_EXIT;
	}
}

__attribute__((interrupt(ADC10_VECTOR)))
void adc10() {
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

