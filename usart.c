#include <avr/io.h>

void USART_Init()
{
	// Set baud rate = 115200 and double transmission speed
	UBRR0H = 0;
	UBRR0L = 16;
	UCSR0A |= _BV(U2X0);
	
	// Enable receiver and transmitter, enable receiver interrupt
	UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
	//UCSR0B = _BV(TXEN0);
	
	// Set format: 8 data, 1 stop bit
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
		
}

void USART_Transmit(unsigned char data)
{
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void USART_Transmit_String(unsigned char* str)
{
	while(*str)
	{
		USART_Transmit(*str);
		++str;
	}
}

void USART_Transmit_EndOfLine()
{
	USART_Transmit('\n');
	USART_Transmit('\r');
}

void USART_Transmit_Tab()
{
	USART_Transmit('\t');
}