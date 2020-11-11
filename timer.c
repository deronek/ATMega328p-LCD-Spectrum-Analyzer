/*
 * timer.c
 *
 * Created: 21.09.2020 16:35:17
 *  Author: kunek
 */ 

#include <avr/io.h>

void Timer_Init()
{
	// Set PWM timer: fast non-inverting PWM mode
	TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
	
	// Set no prescaler
	TCCR0B = _BV(CS00);
	
	// Set no interrupts
	TIMSK0 = 0;
}