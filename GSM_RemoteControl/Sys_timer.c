#include "Sys_timer.h"
#include <avr/interrupt.h>

volatile unsigned char Interrupt_counter = 50;
volatile unsigned long int sys_time;

//--------------------------------------------------
// user variables
volatile unsigned char st_update_cnt;
volatile unsigned char st_scan_key_cnt;


//----------------------------------------------------------------------------------------------
void SysTimer_Init(void)
{
 ASSR = 0;
  sys_time = 0;

   //st_scan_key_cnt = 0;
   st_update_cnt = 0;

   TCNT0 = (256 - 144);
   TCCR0A = 0x00;
   TCCR0B = (1<<CS02)|(0<<CS01)|(1<<CS00);
   TIMSK0 |= (1<<TOIE0);

/*
  OCR0A = 215;
  TCCR0A = (1<<WGM01);
  TCCR0B = (1<<CS02)|(0<<CS01)|(1<<CS00);
  TIMSK0 |= (1<<OCIE0A);*/
  // time = 20ms
};

//----------------------------------------------------------------------------------------------
/* Interrupt -Timer/Counter0 Overflow */
ISR(TIMER0_OVF_vect)  // 20ms
{
 TCNT0 = (256 - 144);

   if(Interrupt_counter == 0) //виклик кожну секунду
   {
	 Interrupt_counter = 50;

	 sys_time++;
   };

  Interrupt_counter--;

  if(st_update_cnt !=0)
  {
	st_update_cnt--;
  }

  if(st_scan_key_cnt !=0)
  {
	st_scan_key_cnt--;
  }
};

//---------------------------------------------------------------------------------------------
/* Interrupt -Timer/Counter0 CTC mode */
ISR(TIMER0_COMPA_vect)  // 20ms
{
 /*  if(Interrupt_counter == 0) //виклик кожну секунду
   {
	 Interrupt_counter = 50;

	 sys_time++;
   };

  Interrupt_counter--;

  if(st_update_cnt !=0)
  {
	st_update_cnt--;
  }

  if(st_scan_key_cnt !=0)
  {
	st_scan_key_cnt--;
  }*/
};

//---------------------------------------------------------------------------------------------
//ISR(TIMER2_OVF_vect){ ; }  // zaglushka

//---------------------------------------------------------------------------------------------------------
unsigned long int ST_GetCurrentTime(void)
{
 unsigned long int time;

	BEGIN_CRITICAL_SECTION;
  	 time = sys_time;
    END_CRITICAL_SECTION;

 return time;
}

//---------------------------------------------------------------------------------------------
void ST_SetSysTime(unsigned long int st)
{
  BEGIN_CRITICAL_SECTION;
    sys_time = st;
  END_CRITICAL_SECTION;
}
//---------------------------------------------------------------------------------------------
