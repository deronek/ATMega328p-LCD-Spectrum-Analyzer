/*
 * adc.c
 *
 * Created: 15.09.2020 20:07:19
 *  Author: kunek
 */ 

#include <avr/io.h>

void ADC_Init()
{
	// Turn off digital input
	DDRC &= ~(PC0);
	
	// Use ADC0 input, AVcc with capacitor at AREF voltage reference
	ADMUX = _BV(REFS0);
	
	// Set 8x prescaler - 500 MHz ADC frequency - around 38,462 kHz sampling frequency
	ADCSRA = _BV(ADPS2) | _BV(ADPS0);
	
	// Set auto trigger enable
	ADCSRA |= _BV(ADATE);
	
	// Set free running mode
	ADCSRB = 0;
	
	// Enable ADC & interrupts
	ADCSRA |= _BV(ADEN) | _BV(ADIE);
	
	// Start first conversion
	ADCSRA |= _BV(ADSC);
}