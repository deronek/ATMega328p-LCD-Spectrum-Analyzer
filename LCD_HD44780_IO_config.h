/* -------------------------------------------------------------------------------
	USER SETTINGS FOR CONFIGURATION OF IOs used to drive LCd with HD44780 driver
	---------------------------------------------------------------------------- */
#define LCD_RS_DDR		DDRB
#define LCD_RS_PORT 	PORTB
#define LCD_RS_BIT		(1 << PB5)

#define LCD_E_DDR		DDRB
#define LCD_E_PORT		PORTB
#define LCD_E_BIT		(1 << PB4)

#define LCD_D4_DDR		DDRB
#define LCD_D4_PORT		PORTB
#define LCD_D4_BIT		(1 << PB3)

#define LCD_D5_DDR		DDRB
#define LCD_D5_PORT		PORTB
#define LCD_D5_BIT		(1 << PB2)

#define LCD_D6_DDR		DDRB
#define LCD_D6_PORT		PORTB
#define LCD_D6_BIT		(1 << PC1)

#define LCD_D7_DDR		DDRB
#define LCD_D7_PORT		PORTB
#define LCD_D7_BIT		(1 << PD0)