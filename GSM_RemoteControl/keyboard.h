#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <avr/io.h>

//------------------------------------------------
#define KEYBOARD_IN ((PINB & 0x07)|(PIND & 0x60))
#define KEYBOARD_INITIALIZE_DIR DDRB &= ~0x07; DDRD &= ~0x60
#define KEYBOARD_INITIALIZE_PORT PORTB &= ~0x07; PORTD &= ~0x60

#define KEY_S0   0x20
#define KEY_S1   0x40
#define KEY_S2   0x01
#define KEY_S3   0x02
#define KEY_S4   0x04

//-------------------------------------------------------------------
typedef unsigned char (*HandleActionKeyboard_F)(unsigned char);

//-------------------------------------------------------------------
void Keyboard_Init(void);
void Keyboard_Scan(void);
void Keyboard_SetKeyPressFunction(HandleActionKeyboard_F onpressed);
#endif
