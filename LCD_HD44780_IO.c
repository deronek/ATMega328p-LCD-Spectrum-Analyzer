#include "LCD_HD44780_IO.h"

/* -------------------------------------
	USER FUNTIONS
	----------------------------------- */
//forms data ready to send
void LCDsendChar(uint8_t data)		                        
{
	LCD_RS_PORT |= LCD_RS_BIT;
	sendByteLCD(data);
	_delay_us(delayCommand);
}
//forms data ready to send
void LCDsendCommand(uint8_t data)	                        
{
	LCD_RS_PORT &= ~LCD_RS_BIT;
	sendByteLCD(data);
	_delay_us(delayCommand);
}
//Initializes LCD
void LCDinit(void)			                            
{
	unsigned char itr=0;
	// setting IO as outputs
	LCD_RS_DDR |= LCD_RS_BIT;
	LCD_E_DDR  |= LCD_E_BIT;
	LCD_D4_DDR |= LCD_D4_BIT;
	LCD_D5_DDR |= LCD_D5_BIT;
	LCD_D6_DDR |= LCD_D6_BIT;
	LCD_D7_DDR |= LCD_D7_BIT;
	_delay_ms(15);
	LCD_RS_PORT &= ~LCD_RS_BIT;
	LCD_E_PORT	&= ~LCD_E_BIT;
	
	for(itr=0;itr<3;itr++){
		LCD_E_PORT	&= ~LCD_E_BIT;
		setHalfByteLCD(0x03);
		LCD_E_PORT	|= LCD_E_BIT;
		_delay_ms(6);
	}
	LCD_E_PORT	&= ~LCD_E_BIT;
	setHalfByteLCD(0x02);
	LCD_E_PORT	|= LCD_E_BIT;
	_delay_ms(1);
	LCDsendCommand(HD44780_FUNCTION_SET | HD44780_FONT5x10 | HD44780_TWO_LINE | HD44780_4_BIT); // 5x10 font, two line, 4bit interface
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF); // turn off display
	LCDsendCommand(HD44780_CLEAR); // clear DDRAM
	_delay_ms(500);
	LCDsendCommand(HD44780_ENTRY_MODE | HD44780_EM_SHIFT_CURSOR | HD44780_EM_INCREMENT);// inkrement addres, move coursore mode
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK); // turn on LCD, cursor on, blink on
}

void LCDprogramChars()
{
	// Set CGRAM address at the beginning
	LCDsendCommand(0x40);
	
	// Put chars into CGRAM -  0x00 is 1 bar high, 0x07 is 8 bar high, 0x20 is 0 bar high
	for(int i = 0; i < 8; ++i)
	{
		for(int k = 0; k < (7-i); ++k)
		{
			LCDsendChar(0x00);
		}
		for(int k = 0; k < (1+i); ++k)
		{
			LCDsendChar(0x1F);
		}
	}
	LCDsendCommand(0x80);
	/*
	LCDsendChar(0x20);
	for(int i = 0; i < 8; ++i)
	{
		LCDsendChar(i);
	}
		LCDGotoXY(0,1);
		for(int i = 0; i < 8; ++i)
		{
			LCDsendChar(i);
		}
*/
}

//Clears LCD
void LCDclr(void)
{
	LCDsendCommand(HD44780_CLEAR);
}		                        
//LCD cursor home
void LCDhome(void)
{
	LCDsendCommand(HD44780_HOME);
	_delay_ms(10);
}			                            
//Outputs string to LCD
void LCDstring(uint8_t* data, uint8_t nOfBytes){
	uint8_t i;
	
	if (!data) return; // check the pointer

	for(i=0; i<nOfBytes; i++)
		LCDsendChar(data[i]);
}
//Cursor to X Y position
void LCDGotoXY(uint8_t x, uint8_t y){
	LCDsendCommand(HD44780_DDRAM_SET | (x + (0x40 * y)));
}
//shift display by n characters Right
void LCDshiftRight(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_DISPLAY | HD44780_SHIFT_RIGHT);
}	
//shift display by n characters Left                        
void LCDshiftLeft(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_DISPLAY | HD44780_SHIFT_LEFT);
}	
//Underline cursor ON                            
void LCDcursorOn(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_CURSOR_NOBLINK);
}	
//Underline blinking cursor ON	                            
void LCDcursorOnBlink(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_CURSOR_BLINK);
}
//Cursor OFF
void LCDcursorOFF(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK);
}	
//LCD blank but not cleared	                        
void LCDblank(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK);
}
//LCD visible
void LCDvisible(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK);
}
//Shift cursor left by n
void LCDcursorLeft(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_CURSOR | HD44780_SHIFT_LEFT);
}
//Shift cursor right by n	                        
void LCDcursorRight(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_CURSOR | HD44780_SHIFT_RIGHT);
}	                

/* -------------------------------------
	BYTE SEND FUNTIONS
	----------------------------------- */
// send byte to LCD/HD44780 via IOs
void sendByteLCD(char data){
	//upper part
	LCD_E_PORT |= LCD_E_BIT;
	setHalfByteLCD(data>>4);
	LCD_E_PORT &= ~LCD_E_BIT;
	_delay_us(delayHalfByte);
	//lower part
	LCD_E_PORT |= LCD_E_BIT;
	setHalfByteLCD(data & 0xF);
	LCD_E_PORT &= ~LCD_E_BIT;
	_delay_us(delayHalfByte);
}

// set half to LCD/HD44780 via IOs (DB4 to DB7)
void setHalfByteLCD(char data){
	if(data & 0x01)
		LCD_D4_PORT |= LCD_D4_BIT;
	else
		LCD_D4_PORT &= ~LCD_D4_BIT;
		
	if(data & 0x02)
		LCD_D5_PORT |= LCD_D5_BIT;
	else
		LCD_D5_PORT &= ~LCD_D5_BIT;
	
	if(data & 0x04)
		LCD_D6_PORT |= LCD_D6_BIT;
	else
		LCD_D6_PORT &= ~LCD_D6_BIT;
	
	if(data & 0x08)
		LCD_D7_PORT |= LCD_D7_BIT;
	else
		LCD_D7_PORT &= ~LCD_D7_BIT;
}

