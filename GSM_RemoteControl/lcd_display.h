#ifndef LCD_DISPLAY_H_
#define LCD_DISPLAY_H_

#include <avr/io.h>


//#define LCD_RESET PORTA.3
#define LCD_DATA (1<<PC2)
#define LCD_SCK (1<<PC1)
#define LCD_DC (1<<PC3)
#define LCD_CS (1<<PC4)

#define LCD_PORT PORTC
#define LCD_DDR DDRC

//=====================================================================================

void LCD_Init(void);
void LCD_Char(char ch);
void LCD_SetPos(unsigned char x, unsigned char y);
void LCD_Print(char *s);
void LCD_Clear(void);
void LCD_PrintInt(unsigned int data);

void LCD_Draw_BatImage(unsigned char x, unsigned char y, unsigned char inum);
//=====================================================================================

#endif
