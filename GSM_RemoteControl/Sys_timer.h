#ifndef SYS_TIMER_H_
#define SYS_TIMER_H_

#include <avr/io.h>

//#define BEGIN_CRITICAL_SECTION TIMSK0 &= ~(1<<OCIE0A)
//#define END_CRITICAL_SECTION TIMSK0 |= (1<<OCIE0A)
#define BEGIN_CRITICAL_SECTION TIMSK0 &= ~(1<<TOIE0)
#define END_CRITICAL_SECTION TIMSK0 |= (1<<TOIE0)

extern volatile unsigned char st_update_cnt;
extern volatile unsigned char st_scan_key_cnt;

//-------------------------------------------------------------------------
void SysTimer_Init(void);
unsigned char ST_SynchronizeTime(void);
unsigned long int ST_GetCurrentTime(void);
void ST_SetSysTime(unsigned long int st);

#endif
