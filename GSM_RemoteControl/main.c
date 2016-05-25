#include <avr/io.h>
#include <util/delay.h>
#include "lcd_display.h"
#include "Sys_timer.h"
#include "keyboard.h"
#include "sim900.h"
#include "ow_termo.h"
#include <string.h>
#include <stdio.h>

const unsigned char rom_id1[] = {0x28, 0xFF, 0xF9, 0x99, 0x4E, 0x04, 0x00, 0x91};
const unsigned char rom_id2[] = {0x28, 0xFF, 0x21, 0xA6, 0x4C, 0x04, 0x00, 0x6C};
const unsigned char rom_id3[] = {0x3A, 0xA1, 0x74, 0x0E, 0x00, 0x00, 0x00, 0x93};  //switch
//-------------------------------------------------------------------------------

unsigned int cnt = 0;

char buffer[BUFFER_LEN];
char *str;
unsigned char resp;

unsigned char termo_state = 0;
unsigned char sw_st = 0;
TemperatureData T1, T2;

//----------------------------------------------------------------------------------
void print_Rom(void);
void Form_result_for_sending(char *s);

//----------------------------------------------------------------------------------
unsigned short int Measure_Bat_U(void)
{
  unsigned int tmp;
  unsigned char i;

   //ADCSRA = (1<<ADEN)|(1<<ADSC)|(3<<ADPS0);
   // 128 prescaler = 62.5KHz @ 8MHz
   ADCSRA = (1<<ADEN)|(1<<ADSC) | (ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    while(ADCSRA & (1<<ADSC));

   tmp = ADCW;
   tmp = 0;

   for(i=0; i < 4; i++)
   {
     ADCSRA |= (1<<ADSC);
      while(ADCSRA & (1<<ADSC));
    tmp += ADCW;
   }

    ADCSRA = 0; // off adc

    tmp >>= 1;
    tmp <<= 3;  // tmp *= 8;

 //--------------------------------------
    if(tmp > 3860)
    {
	  LCD_Draw_BatImage(67, 0, 3);
    }
    else if(tmp > 3700)
    {
	  LCD_Draw_BatImage(67, 0, 2);
    }
    else if(tmp > 3600)
    {
	  LCD_Draw_BatImage(67, 0, 1);
    }
    else if(tmp < 3580)
    {
	  LCD_Draw_BatImage(67, 0, 0);
    }

  return tmp;
}

//-----------------------------------------------------------------------------------------
void parse_sms_and_run(const char *buf)
{
 char *ts;
 unsigned char sd = 0;

	LCD_Clear();

	ts = strstr(buf, "switch_st");

	if(ts != NULL)
	{
	 sd = *(ts + 10) - '0';

	 LCD_SetPos(0, 3);
	 LCD_Print("Switch_st=");
	 LCD_Char(sd+'0');
	 sw_st=sd;
	  OW_SetSwitchState(rom_id3, 3 & (3^sw_st));
	}

	if(NULL != strstr(buf, "get_st"))
	{
	 // send sms
	 Form_result_for_sending(buffer);
	  resp = SIM900_sendSMS("+380961150566",buffer);
	  //_delay_ms(50);

	  if(resp == 0)
	  {
		LCD_SetPos(0, 2);
		LCD_Print("Send state ok!");
		_delay_ms(20);
		SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 20000);
	  }
	  else
	  {
		  // auto recall
	  }
	}
}

//-----------------------------------------------------------------------------------------
void Form_result_for_sending(char *s)
{
  sprintf(s, "Test board rev 1.1\nt1=%d.%d C\nt2=%d.%d C \nSW1=%d, SW2=%d",
		   T2.T, T2.T_tenths,T1.T, T1.T_tenths, sw_st&1, (sw_st>>1) & 1);
}

//---------------------------------------------------------------------------------
void print_byte_hex(unsigned char b)
{
  unsigned char tb;

   tb = b >> 4;

    if(tb >= 10)
	{
	  tb -= 10;
	  LCD_Char('A'+tb);
	}
	else
	{
	  LCD_Char('0'+tb);
	}

	tb = b & 15;

	 if(tb >= 10)
	 {
	  tb -= 10;
      LCD_Char('A'+tb);
	 }
	 else
	 {
	   LCD_Char('0'+tb);
	 }
}

//--------------------------------------------------------------------------------
void print_Rom(void)
{
  unsigned char mas[8];
  unsigned char i;


   if(OW_ReadROM(mas) == 0)
   {
	 LCD_SetPos(0, 0);
	 LCD_Print("id: ");
	 print_byte_hex(mas[0]);

	 LCD_SetPos(0, 1);
	 for(i=1; i<8;i++)
	 {
		print_byte_hex(mas[i]);
	 }
   }
   else
   {
	 LCD_SetPos(0, 0);
	 LCD_Print("rom error!");
   }
}

//---------------------------------------------------------------------------------
void print_temp(unsigned char n, TemperatureData *td)
{
  unsigned char ch1;
  unsigned char x;

	ch1 = 0;
	x = td->T;

	while(x >= 10)
	{
	  x -=10;
	  ch1++;
	}

   if(n == 1)
   {
     LCD_SetPos(0, 4);
     LCD_Print("t1 = ");
   }
   else
   {
     LCD_SetPos(0, 5);
     LCD_Print("t2 = ");
   }

  LCD_Char('0'+ch1);
  LCD_Char('0'+x);
  LCD_Char('.');
  LCD_Char('0'+td->T_tenths);
  LCD_Char('C');
}

//---------------------------------------------------------------------------------
void Termo_POOL(void)
{
  unsigned char res;

   switch(termo_state)
   {
    case 0:
    	res = OW_Start_Tem_Convert(rom_id1);
    	res +=OW_Start_Tem_Convert(rom_id2);
    	 if(res == 0)
    	 {
    	   termo_state = 1;
    	 }
    	 else
    	 {
    	   termo_state = 10;
    	 }

	   break;

    case 1:
    	res = OW_Get_Temperature(rom_id1, &T1);
      	 if(res == 0)
    		print_temp(2, &T1);

     	res = OW_Get_Temperature(rom_id2, &T2);
       	 if(res == 0)
     		print_temp(1, &T2);

    	termo_state = 2;
	   break;

    case 2:
    	termo_state = 10;
	   break;

    case 3:
 	   break;

    default:
    	termo_state = 0;
   }
}

//-----------------------------------------------------------------------------------------
unsigned char onKeyPressed(unsigned char key)
{

  if(key & KEY_S4)
  {

	LCD_Clear();

	str = SIM900_readSMS(cnt,"",30000);
	if(str)
	{
		 LCD_Print((char*)str);

		// parse_sms_and_run(str);
	}

   }
   else if(key & KEY_S1)
   {
		//LCD_SetPos(1, 0);
		//LCD_PrintInt((cnt>0)? --cnt: 0);
		//LCD_Clear();
		//print_Rom();

	 ++sw_st;
 	  if(sw_st > 3){ sw_st=0; }

	  OW_SetSwitchState(rom_id3, 3 & (3^sw_st));
	  LCD_Clear();
	  LCD_SetPos(0, 1);
	  LCD_Print(" ");
	  LCD_SetPos(0, 2);
	  LCD_Print("SW1:");
	  LCD_Char('0'+(sw_st&1));
	  LCD_SetPos(0, 3);
	  LCD_Print("SW2:");
	  LCD_Char('0'+((sw_st>>1)&1));
	}
	else if(key & KEY_S3)
	{
	 LCD_SetPos(1, 0);
	 cnt =(cnt<10) ? cnt + 1:  0;
	 LCD_PrintInt(cnt);
	}
	else if(key & KEY_S4)
	{
	  PORTC ^= (1<<PC0);
	}
	else if(key & KEY_S0)
	{
	  SIM900_OnOff();

	  _delay_ms(10000);

	  if(SIM900_sendAtCommandAndCheckOK("AT\r", buffer, 50000)==0)
	  {
		 _delay_ms(50);
		SIM900_sendAtCommandAndCheckOK("ATV1\r", buffer, 20000);
		 _delay_ms(50);
		SIM900_sendAtCommandAndCheckOK("AT+CMGF=1\r", buffer, 20000);
		 _delay_ms(50);
		SIM900_sendAtCommandAndCheckOK("AT+CSCS=\"GSM\"\r", buffer, 20000);
		 _delay_ms(50);
		SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 30000);

		 LCD_Clear();
		 LCD_SetPos(0, 1);
    	 LCD_Print("Sim900 is run.");
	  }
	}
	else if(key & KEY_S2)
	{
		Form_result_for_sending(buffer);
		resp = SIM900_sendSMS("+380961150566",buffer);
		LCD_Clear();
		if(resp == 0)
		{
		 LCD_SetPos(0, 1);
		 LCD_Print("send ok!");
		}
		else
		{
		LCD_SetPos(0, 1);
		LCD_Print("error!");
		}
	}

 return 1;
}

//##################################################################################
int main(void)
{

 // unsigned int x = 0;
  unsigned char mi, ch;
  char mbuf[64];

  //char fl = 0;
  //unsigned char res_av_cnt = 0;

  unsigned char blinker_del=0, scansms_del=0;


  DDRC |= 1<<PC0;
 // PORTC |= 1<<PC0;
  //----------------------------------
  ACSR = 0x80;
  // 1.1V internal ref, ADC channel 0
  ADMUX = (1<<REFS0) | (1<<REFS1) | 7; //adc7



  Keyboard_Init();
  SysTimer_Init();
  LCD_Init();
  SIM900_Init();
  OW_Init();

  Keyboard_SetKeyPressFunction(&onKeyPressed);

    __asm__ __volatile__ ("sei" ::);

  LCD_SetPos(0, 1);
  LCD_Print("GSM_SIM900 :)");

 // LCD_SetPos(30, 3);
 // LCD_Print("Hello! :)");

 // LCD_SetPos(25, 4);
 // LCD_Print("Andriy! :)");



 // SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 20000);



  //SIM900_sendAtCommandAndCheckOK("AT+CMGD=4",buffer,30000);
  //LCD_SetPos(0, 5);
//	 LCD_Print((char*)buffer);

   mi = 0;

	//============================================================
	for(;;)
	{

    	if(st_update_cnt == 0)
		{
		  st_update_cnt = 50;  // 1 sek
		   Termo_POOL();

		   if(++scansms_del > 10)
		   {
			   scansms_del=0;

			  (void) Measure_Bat_U();
			 //  LCD_SetPos(40, 1);
			 //  LCD_PrintInt( Measure_Bat_U());
		   }

		}

		if(st_scan_key_cnt == 0)
		{
		  st_scan_key_cnt = 2;
		  Keyboard_Scan();

		  ++blinker_del;

		  if(blinker_del > 25 )
		  {
			blinker_del = 0;
			PORTC |= 1<<PC0;
		  }
		  else if(blinker_del == 2)
		  {
			PORTC &= ~(1<<PC0);
		  }
		}

		//-------------------------------- is recive sms
		if(UART_Available())
		{
			ch = USART_Receive();
			//LCD_Print(ch);

/*
			if(ch == '\n')
			{
			 mbuf[mi]=0;

			  if(fl)
			  {
			   fl=0;
				LCD_Clear();
				LCD_Print((char*)mbuf);

				//------------------
				str = SIM900_readSMS(1,"",30000);
				if(str)
				{
				  LCD_Print((char*)str);

					 parse_sms_and_run(str);
				}
				else
				{
				  str = SIM900_readSMS(1,"",30000);
				   if(str)
				   {
					  LCD_Print((char*)str);
					  parse_sms_and_run(str);
				   }
				}

			   _delay_ms(20);

			     SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 30000);
			     _delay_ms(60);
			     SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 5000);

			  }
			  else
			  {
				if(strstr(mbuf, "+CMTI:") != NULL)
				{
				  fl=1;
				}
			  }

			  mi=0;
			}
			else
			{
			  if(ch != '\n')
			  {
			    mbuf[mi++] = ch;
			  }

			 if(mi > 63) mi =0;
           }
*/


			if(ch == '\n')
			{
			  mbuf[mi]=0;
			  mi=0;

			  if(strstr(mbuf, "+CMTI:") != NULL)
			  {
				LCD_Clear();
				LCD_Print((char*)mbuf);

				//------------------
				str = SIM900_readSMS(1,"",30000);
				if(str)
				{
				  LCD_Print((char*)str);

					 parse_sms_and_run(str);
				}
				else
				{
				  str = SIM900_readSMS(1,"",30000);
				   if(str)
				   {
					  LCD_Print((char*)str);
					  parse_sms_and_run(str);
				   }
				}

			   _delay_ms(20);

			     SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 30000);
			     _delay_ms(60);
			     SIM900_sendAtCommandAndCheckOK("AT+CMGDA=\"DEL ALL\"\r", buffer, 5000);
			  }

			}
			else
			{
			  if(ch != '\r')
			  {
			    mbuf[mi++] = ch;
			  }

			 if(mi > 63) mi =0;
           }



		}

		//Keyboard_Scan();
	  _delay_ms(1);
	}

//	SIM900_Pool();
  return 0;
}


/*

  tmp = curU;
  if(tmp > 3810)
  {
	Draw_BatImage(112, 0, 3);
  }
  else if(tmp > 3600)
  {
    Draw_BatImage(112, 0, 2);
  }
  else if(tmp > 3400)
  {
  	Draw_BatImage(112, 0, 1);
  }
  else if(tmp < 3280)
  {
	 Draw_BatImage(112, 0, 0);
  }
 */
