#include "keyboard.h"

#define KEY_DOWN	1
#define KEY_REPEAT	2
#define KEY_PRESSED	4

#define ClearFlags(reg, flg) reg &= (~(flg))
#define SetFlags(reg, flg) reg |= (flg)
#define GetFlag(reg, flg) (reg & (flg))

//----------------------------------------------------------------------------------------------------------

static unsigned char key_flags;
static unsigned char /*n_key,*/ last_key;
static HandleActionKeyboard_F OnPressed_f;

//----------------------------------------------------------------------------------------------------------
static unsigned char NullKeyF(unsigned char key){ return 1; }

//----------------------------------------------------------------------------------------------------------
void Keyboard_Init(void)
{
   key_flags = 0;
   OnPressed_f = &NullKeyF;

  KEYBOARD_INITIALIZE_PORT;
  KEYBOARD_INITIALIZE_DIR;
}

//----------------------------------------------------------------------------------------------------------
void Keyboard_SetKeyPressFunction(HandleActionKeyboard_F onpressed)
{
   OnPressed_f = onpressed;
}

//----------------------------------------------------------------------------------------------------------
void Keyboard_Scan(void)
{
 unsigned char tmp;

 //if(GetFlag(key_flags, KEY_PRESSED) == 0)
 //{
   tmp = ~KEYBOARD_IN;// & (KEY_PLUS|KEY_MINUS|KEY_SELECT);

   //Якщо біт нажата встановлений і опередній стан кнопок == прочитаному
   if( (GetFlag(key_flags, KEY_DOWN) == KEY_DOWN) && (tmp == last_key))
   {
     if(GetFlag(key_flags, KEY_REPEAT) == 0)
     {
    	//SetFlags(key_flags, KEY_REPEAT | KEY_PRESSED);   // rep = 1, new = 1
    	if( (*OnPressed_f)(tmp) != 0 )
    	{
    	  SetFlags(key_flags, KEY_REPEAT);
    	}
      // n_key= tmp;
     }
   }
   else if(tmp != 0)  //16
   {
     last_key = tmp;
     SetFlags(key_flags, KEY_DOWN);
     ClearFlags(key_flags, KEY_REPEAT);
   }
   else
   {
     ClearFlags(key_flags, KEY_DOWN | KEY_REPEAT);
   }

}

//----------------------------------------------------------------------------------------------------------
