#include "ow_termo.h"
#include <avr/io.h>
#include <util/delay.h>

#define OW_PIN 7

#define ow_line_hi() DDRD &= ~(1<<OW_PIN);
#define ow_line_lo() DDRD |= (1<<OW_PIN);
#define ow_line_get_state() (PIND & (1<<OW_PIN))


unsigned char ow_buff[8], ow_crc;

//----------------------------------------------------------------------------------
void OW_Init(void)
{
   PORTD &= (1<<OW_PIN);
   ow_line_hi();
}

//----------------------------------------------------------------------------------
unsigned char OW_Reset(void)
{
  unsigned char t = 1;

  __asm__ __volatile__ ("cli" ::);

  ow_line_lo(); //[0]
    _delay_us(480);
  ow_line_hi(); //[1]
   _delay_us(70);

 if(ow_line_get_state() == 0) t = 0; // presence ok!
  else t = 1;                   	 // not present
    _delay_us(410);
  if(ow_line_get_state() == 0) t = 2; // error!

  __asm__ __volatile__ ("sei" ::);

 return t;
}

//--------------------------------------------------------------------------------
/*unsigned char OW_ReadBit(void)
{
  unsigned char t;

 t=0;

  ow_line_lo(); //[0]
   _delay_us(2); //6
  ow_line_hi(); //[1]
   _delay_us(7); //9
 if(ow_line_get_state() != 0) t = 1;
  _delay_us(50);  //55

 return t;
}

//--------------------------------------------------------------------------------
void OW_WriteBit(unsigned char b)
{
 ow_line_lo(); //[0]

  if(b){ _delay_us(6);}
    else { _delay_us(60);}

 ow_line_hi(); //[1]

  if(b){ _delay_us(64);}
    else { _delay_us(10);}
}
*/
//--------------------------------------------------------------------------------
unsigned char OW_ReadByte(void)
{
 unsigned char i, t;

 __asm__ __volatile__ ("cli" ::);//  cli
  t=0;

  for(i=0;i<8;i++)
  {
    ow_line_lo(); //[0]
     _delay_us(6);
    ow_line_hi(); //[1]
     _delay_us(9);
    t >>=1;
    if(ow_line_get_state()){ t |= 0x80; }

   _delay_us(55);
  }
  __asm__ __volatile__ ("sei" ::);
 return t;
}

//--------------------------------------------------------------------------------
void OW_WriteByte(unsigned char bb)
{
  unsigned char i,t;

  __asm__ __volatile__ ("cli" ::);

 for(i=1; i; i<<=1)
 {
   t = bb & i;

   ow_line_lo(); //[0]

   if(t){ _delay_us(6);}
     else { _delay_us(60);}

  ow_line_hi(); //[1]

   if(t){ _delay_us(64);}
     else { _delay_us(10);}
  }

 __asm__ __volatile__ ("sei" ::);
}

//--------------------------------------------------------------------------------
unsigned char OW_ReadROM(unsigned char *rom_id)
{
 unsigned char i;

  i=OW_Reset();

   if(i==0)
   {
	   OW_WriteByte(0x33);

	   for(i=0;i<8;i++)
	   {
		  rom_id[i]=OW_ReadByte();
	   }
 // CRC=OW_ReadByte();
 } else  return(i);
 return 0;
}

//--------------------------------------------------------------------------------
void UpdateCRC(unsigned char dd)
{
 unsigned char i, t, d;

 d = dd;

 for(i=0; i<8; i++)
 {

  if(d & 1){
   if(ow_crc & 1) t = 0; else t = 1; }
  else{
    if(ow_crc & 1) t = 1; else t = 0; }

  if(t)
  {
	ow_crc ^= 0x18;
	ow_crc >>= 1;
	ow_crc |= 0x80;
  }
  else
  {
	ow_crc >>= 1;
	ow_crc &= 0x7F;
  }

  d >>=1;
 }
}

//--------------------------------------------------------------------------------
void OW_MatchROM(const unsigned char *rom)
{
  unsigned char i;

  OW_WriteByte(0x55);

   for(i=0; i<8; i++)
   {
     OW_WriteByte(rom[i]);
   }
}

//--------------------------------------------------------------------------------
unsigned char OW_Start_Tem_Convert(const unsigned char *rom)
{
  unsigned char i;

   i = OW_Reset();

	if(i == 0)
	{
	 //  OW_WriteByte(0xCC); // DALLAS_SKIP_ROM
	  OW_MatchROM(rom);
	   OW_WriteByte(0x44);
	} // CONVERT_TEMP

 return i;
}

//--------------------------------------------------------------------------------
unsigned char OW_Get_Temperature(const unsigned char *rom, TemperatureData *td)
{
 unsigned char i, d, f;

    union Word_var_U
    {
	   unsigned short int word;
	   unsigned char byte[2];
	} Word_var;

  i= OW_Reset();

  if(i==0)
  {

   // OW_WriteByte(0xCC); // DALLAS_SKIP_ROM
    OW_MatchROM(rom);

    OW_WriteByte(0xBE); //  read scratchpad

    ow_crc = 0; f = 1;

    Word_var.byte[0] = OW_ReadByte();  //lo
	Word_var.byte[1] = OW_ReadByte();  //hi
      UpdateCRC(Word_var.byte[0]);
      UpdateCRC(Word_var.byte[1]);

    td->T =(unsigned char)(Word_var.word >> 4 & 0x7F);
     i = Word_var.byte[0] & 0x0F;
    td->T_tenths =  ((i << 1) + (i << 3)) >> 4;

    for(i=0;i<6;i++)
    {
     d= OW_ReadByte();
      UpdateCRC(d);
    }

    d = OW_ReadByte();

     if((d == ow_crc) && (f != 0)) return 0;
       else return 3;
   } else return i;
}


//----------------------------------------------------------------------------------
unsigned char OW_GetSwitchState(const unsigned char *rom, unsigned char *state)
{
 unsigned char _state, t;

  t = OW_Reset();

    if(!t)
	{
	  // OW_WriteByte(0xCC);
       OW_MatchROM(rom);
	   OW_WriteByte(0xF5); //cmd: write latch

	   _state = OW_ReadByte(); //Get the register results
	    t = (!_state & 0x0F) == (_state >> 4);    // Compare nibbles

	    if(!t) return 1;

	    *state =  _state & 0x0F;
	    //OW_Reset();

	}
	else
	{
	  return 1;
	}

 return 0;
}

//----------------------------------------------------------------------------------
unsigned char OW_SetSwitchState(const unsigned char *rom, unsigned char state)
{
 unsigned char t,k;

   t = OW_Reset();

    if(!t)
	{
	  // OW_WriteByte(0xCC);
       OW_MatchROM(rom);
	   OW_WriteByte(0x5A); //cmd: write latch

	   state |= 0xFC;

	   OW_WriteByte(state);
	   OW_WriteByte(~state);

	    t= OW_ReadByte();
	    k= OW_ReadByte();

	   if(t != 0xAA)
	   {
		   return 1;
	   }
	}
    else
    {
	  //  LCD_String("1-Wire ERROR: $",0,2,clRed,clBlack);
	    //LCD_Print_hex(t);
	   // LCD_String("    ",0xFF,0xFF,clRed,clBlack);
      return 1;
	}

  return 0;
}
