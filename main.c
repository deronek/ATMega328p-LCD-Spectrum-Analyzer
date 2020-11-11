/*
 * ATMega328p LCD Spectrum Analyzer.c
 *
 * Created: 14.09.2020 20:35:50
 * Author : kunek

-------------------------------------------------

Timer1, prescaler 256, -O2 optimization

Pobieranie 64 probek: 103 cykle - 1,65 ms (wartoœc spodziewana: 13,5 cykli * 64 probki / 500 MHz = 1,78 ms)

FFT: 176 cykli - 2,8 ms
Przygotowanie danych spektrum: 57 ± 10 cykli - 0,9 ms
Wysy³anie na LCD: 219 cykli - 3,5 ms

Pomiar ³¹czny: ok. 555 cykli - 8,88 ms - 113 aktualizacji na sekundê

Timer1 setup for measuring performance, 256 prescaler
	TCCR1A = 0;
	TCCR1B = _BV(CS12);
	TIMSK1 = 0;

-------------------------------------------------
*/

#define F_CPU   16000000UL
#define AVERAGING_SIZE 3
#define MULT_VALUE_1 4
#define MULT_VALUE_2 6
#define MULT_VALUE_3 8

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#include "usart.h"
#include "adc.h"
#include "ffft.h"
#include "config.h"
#include "timer.h"
#include "LCD_HD44780_IO.h"

uint8_t divide_steps = DIVIDE_STEPS_INIT;
unsigned long divide_steps_change_delay = DIVIDE_STEPS_CHANGE_DELAY;

volatile uint8_t buffer_index;
volatile uint8_t buffer_ready;
volatile uint8_t loudness;
volatile uint8_t spectrum_index;
volatile uint8_t mult = MULT_VALUE_2;
volatile uint8_t wait_for_number;

int16_t fft_buffer[FFT_N];
complex_t fft_bfly[FFT_N];
uint8_t spectrum[FFT_N/2];
uint8_t spectrum_history[FFT_N/2][AVERAGING_SIZE];
uint8_t lcd_output_H[FFT_N/4], lcd_output_L[FFT_N/4];

// Measured DC offset values for FFT_N = 64
uint8_t DCoffset[16] = {166, 113, 40, 25, 18, 14, 12, 10, 9, 8, 7, 7, 6, 6, 6, 6};
uint8_t LCDbars[9] = {BAR0, BAR1, BAR2, BAR3, BAR4, BAR5, BAR6, BAR7, BAR8};

// Temporary array for printing numbers to USART
char str[16];

char *columnHertz[] = {"0 Hz - 601 Hz: \t\t",
	"601 Hz - 1202 Hz: \t",
	"1202 Hz - 1803 Hz: \t",
	"1803 Hz - 2404 Hz: \t",
	"2404 Hz - 3005 Hz: \t",
	"3005 Hz - 3606 Hz: \t",
	"3606 Hz - 4207 Hz: \t",
	"4207 Hz - 4808 Hz: \t",
	"4808 Hz - 5409 Hz: \t",
	"5409 Hz - 6010 Hz: \t",
	"6010 Hz - 6611 Hz: \t",
	"6611 Hz - 7212 Hz: \t",
	"7212 Hz - 7813 Hz: \t",
	"7813 Hz - 8413 Hz: \t",
	"8413 Hz - 9014 Hz: \t",
	"9014 Hz - 9615 Hz: \t"
};

int main(void)
{
	USART_Init();
	Timer_Init();
	LCDinit();
	LCDprogramChars();
	
	// Set LED output
	DDRD |= _BV(PD6);
	
	ADC_Init();
	
	sei();
	
    while(1)
    {
		if(buffer_ready)
		{
			// Calculate FFT
			
			fft_input(fft_buffer, fft_bfly);
			fft_execute(fft_bfly);
			fft_output(fft_bfly, spectrum);
			
			// Calculate spectrum data for display
			
			loudness = 0;
			for(int i = 0; i < FFT_N/4; ++i)
			{
				// Apply DC offset removal with 0 as minimum output value
				if(spectrum[i] <= DCoffset[i]) spectrum[i] = 0;
				else spectrum[i] -= DCoffset[i];
				
				// Apply slope to emphasise higher frequencies
				spectrum[i] *= (float)i/(float)4 + 0.5;
				
				// Linear moving average of current and past spectrums
				uint8_t sum = 0;
				for(int p = 0; p < AVERAGING_SIZE; ++p)
				{
					sum += spectrum_history[i][p];
				}
				spectrum[i] = (sum + spectrum[i]) / (AVERAGING_SIZE + 1);
				
				// Limit signal maximum value display
				if(spectrum[i] > 16) spectrum[i] = 16;
				
				// Add current bar value to total loudness
				loudness += spectrum[i];
			}

			// Exponential LED brightness implementation
			// Value written to PWM counter compare register:
			// MULT_VALUE_1				0 - 15
			// MULT_VALUE_2 (default)	0 - 63
			// MULT_VALUE_3				0 - 255
			uint16_t bright = pow(2, (float)loudness/(255/mult)) - 1;
			
			// Set counter compare register with max value of 255 (safeguard)
			OCR0A = (bright > 255) ? 255 : bright;
			
			// Prepare spectrum graph to send to LCD
			for(int i = 0; i < 16; ++i)
			{
				if(spectrum[i] > 8)
				{
					lcd_output_L[i] = LCDbars[8];				// Full lower bar when signal is high
					lcd_output_H[i] = LCDbars[spectrum[i]-8];
				}
				else
				{
					lcd_output_L[i] = LCDbars[spectrum[i]];
					lcd_output_H[i] = LCDbars[0];				// Empty higher bar when signal is low
				}
			}
			
			// Send lower row to LCD
			for(int i = 0; i < 16; ++i)
			{
				LCDsendChar(lcd_output_L[i]);
			}
			
			// Move cursor to higher row
			LCDGotoXY(0, 0);
			
			// Send higher row to LCD
			
			for(int i = 0; i < 16; ++i)
			{
				LCDsendChar(lcd_output_H[i]);
			}
			
			// Set indices for next sample batch
			buffer_index = 0;
			buffer_ready = 0;
			
			// Turn on ADC interrupts
			ADCSRA |= _BV(ADIE);
			
			// Write latest spectrum data to history circular buffer
			if(spectrum_index >= AVERAGING_SIZE - 1) spectrum_index = 0;
			else ++spectrum_index;
			
			for(int i = 0; i < 16; ++i)
			{
				spectrum_history[i][spectrum_index] = spectrum[i];
			}
			
			// Prepare for next LCD write
			LCDGotoXY(0, 1);
		}
	}
}

ISR(ADC_vect)
{
	fft_buffer[buffer_index] = ADCL;
	fft_buffer[buffer_index] |= (ADCH << 8);
	++buffer_index;
	
	if(buffer_index == 64)
	{
		// Turn off ADC interrupts
		ADCSRA &= ~(_BV(ADIE));
		
		buffer_ready = 1;
	}
}

ISR(USART_RX_vect)
{
	unsigned char data = UDR0;
	
	if(wait_for_number)
	{
		switch(data)
		{
			case '1':
				mult = MULT_VALUE_1;
				break;
			case '2':
				mult = MULT_VALUE_2;
				break;
			case '3':
				mult = MULT_VALUE_3;
				break;
			case 'q':
				wait_for_number = 0;
				USART_Transmit_String("Przerwano wybor mnoznika.");
				USART_Transmit_EndOfLine();
			default:
				return;
		}
			wait_for_number = 0;
			USART_Transmit_String("Ustawiono wybor mnoznika diody na ");
			USART_Transmit(data);
			USART_Transmit_EndOfLine();
	}
	
	switch(data)
	{
		case 'l':
			itoa(loudness * 0.3922, str, 10); // 0,3922 = 100/255, mapping loudness to percent
			USART_Transmit_String("Aktualny poziom glosnosci: ");
			USART_Transmit_String(str);
			USART_Transmit('%');
			USART_Transmit_EndOfLine();
			break;
		case 'w':
			USART_Transmit_String("Wybierz mnoznik jasnosci diody (1, 2, 3, q zeby wyjsc)");
			USART_Transmit_EndOfLine();
			wait_for_number = 1;	
			break;
		case 's':
			USART_Transmit_String("Aktualne poziomy spektrum:");
			USART_Transmit_EndOfLine();
			for(int i = 0; i < 16; ++i)
			{
				USART_Transmit_String(columnHertz[i]);
				itoa(spectrum_history[i][spectrum_index] * 100 / 16, str, 10);
				USART_Transmit_String(str);
				USART_Transmit('%');
				USART_Transmit_EndOfLine();
			}
			USART_Transmit_EndOfLine();
			break;
		default:
			return;
	}
}