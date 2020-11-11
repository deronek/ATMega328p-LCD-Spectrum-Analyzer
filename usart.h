/*
 * usart.h
 *
 * Created: 15.09.2020 19:54:13
 *  Author: kunek
 */ 


#ifndef USART_H_
#define USART_H_

void USART_Init();
void USART_Transmit(unsigned char);
void USART_Transmit_String(char*);
void USART_Transmit_EndOfLine();
void USART_Transmit_Tab();
void USART_Flush();

#endif /* USART_H_ */