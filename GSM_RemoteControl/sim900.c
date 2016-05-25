#include "sim900.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>
#include <stdio.h>

#define UART_RX_BUFFER_SIZE 128
#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1)

//----------------------------------------------------------------------


static char buffer[BUFFER_LEN];
static char print_buf[32];

//--------------------------------------------------------------
static volatile unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_RxHead;
static volatile unsigned char UART_RxTail;

//-----------------------------------------------------------------------
void USART_Transmit(unsigned char data);
unsigned char USART_Receive(void);

//=======================================================================
void SIM900_Init(void)
{
  PORTD &= ~(1<<PD2);
  DDRD |= (1<<PD2);

  //---- UART init ------
      UBRR0H = 0;
      UBRR0L = 47; // 9600 bod
    //UBRR0L = 95;  // 4800 bod
   // UBRR0L = 23;

  //Enable receiver and transmitter
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  // Set frame format: 8data, 1stop bit
  UCSR0C = (3<<UCSZ00);

 // UCSR0B |= (1<<RXCIE0);

  UART_RxHead = UART_RxTail = 0;

}


//=========================================================================
void USART_Transmit(unsigned char data)
{
  // Wait for empty transmit buffer
	while( !( UCSR0A & (1<<UDRE0)) );
  // Put data into buffer, sends the data */
 UDR0 = data;
}

//-------------------------------------------------------------------------
unsigned char UART_Available(void)
{
  return (UCSR0A & (1<<RXC0));
}

//-------------------------------------------------------------------------
unsigned char USART_Receive(void)
{
  // Wait for data to be received
  // while ( !(UCSR0A & (1<<RXC0)) );
  // Get and return received data from buffer
 return UDR0;
}

//-------------------------------------------------------------------------
unsigned short int UART_GetCh(void)
{
  unsigned char tmptail;
  unsigned char data;

	if(UART_RxHead == UART_RxTail)
	{
		return 0;// UART_NO_DATA;
	}

	// calculate /store buffer index
	tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;
	UART_RxTail = tmptail;

	// get data from receive buffer
	data = UART_RxBuf[tmptail];

  return 0x0100 + data;

}

//-------------------------------------------------------------------------
//unsigned char UART_Available(void)
//{
//  return (UART_RX_BUFFER_SIZE + UART_RxHead - UART_RxTail) & UART_RX_BUFFER_MASK;
//}

//-------------------------------------------------------------------------
ISR(USART_RX_vect) // USART Rx Complete
{
  unsigned char tmphead;
  unsigned char data;

   //unsigned char  usr;
   //uint8_t lastRxError;

	// read UART status register and UART data register
	//usr  = UART1_STATUS;
	data = UDR0;


	//lastRxError = (usr & (_BV(FE0)|_BV(DOR0)) );
	// calculate buffer index
	tmphead = (UART_RxHead + 1) & UART_RX_BUFFER_MASK;

	if(tmphead == UART_RxTail)
	{
	  // error: receive buffer overflow
	  //	lastRxError = UART_BUFFER_OVERFLOW >> 8;
	}
	else
	{
	  //store new index
	  UART_RxHead = tmphead;
	  // store received data in buffer
	  UART_RxBuf[tmphead] = data;
	}
//UART1_LastRxError = lastRxError;
}



//-------------------------------------------------------------------------
ISR(USART_UDRE_vect) // USART, Data Register Empty
{
}

//-------------------------------------------------------------------------
ISR(USART_TX_vect) // USART Tx Complete
{
}

//------------------------------------------------------------------------
void SIM900_OnOff(void)
{
  PORTD |= (1<<PD2);
	_delay_ms(1000);
  PORTD &= ~(1<<PD2);

}

//-------------------------------------------------------------------------
void SIM900_Pool(void)
{
	USART_Transmit(USART_Receive());
}

void uart_print(char *s)
{
	while(*s)
	{
	  USART_Transmit(*s++);
	}
}

//--------------------------------------------------------------------------------------------------------
void SIM900_sendAtCommand(const char *command)
{
  /*  unsigned char flag = 0;// false;
char c;

	_delay_ms(200);


	while(USART_Avaliable())
	{
	  c = simSwSerial->read();

		if(!flag)
		{
			//Serial.print("  SKIPPED : ");
		   flag = 1; //true;
		}
		//Serial.print(c);
	}
*/

	while(*command)
	{
	  USART_Transmit(*command++);
	}
}

//---------------------------------------------------------------------------------------
unsigned char SIM900_readLine(char *response, unsigned int timeout_, unsigned char append)
{
  unsigned char idx=0;
  unsigned char  answer;
 // unsigned short int data;
  unsigned int timeout = 0;

  char c;

	if(append)
	{
	  idx = strlen(response);
	}
	else
	{
	  memset(response, '\0', BUFFER_LEN);    // Initialice the string
	}

	answer = 2;  //Timeout par défaut

	//unsigned long previous = millis();

	while(timeout < timeout_ )//(millis()-previous)<timeout)
	{
		if(UART_Available() != 0)
		{
		   c = UDR0; //simSwSerial->read();
		 //  data = UART_GetCh();

		//   if(data & 0xFF00)
		  // {

			//On vérifie que ça rentre dans le buffer
			if((idx + 1) >= BUFFER_LEN)
			{
			 // Serial.println("ERROR: Buffer overflow");
			  answer = 3;
			  break;
			}

			response[idx] = c;
			idx = idx + 1;

			if( c == 10 )
			{
			  //C'est la fin de ligne
			  //if(idx>0 && response[idx-1]==13)  response[idx--] = '\0';
			   answer = 0;
			   if(idx == 2) answer = 1; //uniquement les caractères CR(13) LF(10)
			  break;
			}
		 // }
		}

		++timeout;
	  _delay_us(100);
	}
  return answer;
}

//---------------------------------------------------------------------------------------
/* Attend et vérifie que la réponse contient la chaine attendue
 * Attend jusqu'au timeout.
 * Réponse :
 *    0:OK - response match
 *    1:Empty
 *    2:Timeout
 *    3:Overflow
 *    4:ERROR  */
unsigned char SIM900_checkOK(char *response, unsigned int timeout)
{
	//La réponse commence éventuellement par un "echo" de la requete, puis une ligne vide
	//On ignore cette première partie
	//char *sub;
	unsigned int idx;
	unsigned char answer;


	 answer = SIM900_readLine(response, timeout, 0);

	if(answer==2)
	{
		//Serial.println("TIMEOUT sur le première ligne");
		return answer;
	}

	do {
	  answer = SIM900_readLine(response, timeout, 0);
	}
	while(answer == 1); //Tant que c'est une ligne vide, on passe


	while(answer==0 || answer==1)
	{
		//On regarde si la réponse se termine par OK
		//sub=strstr(response,"\r\nOK");
	  idx = strlen(response) - 6;

		if(idx>=0 && strcmp(&response[idx],"\r\nOK\r\n") == 0)
		{
			//Reponse OK
			//On tronque le buffer pour supprimer "OK"
		  response[idx] = '\0';
		  break;
		}

		//On regarde si la réponse commence par OK
		if(strcmp("OK",response) < 0)
		{
		   response[0] = '\0';
		  break;
		}

		//On regarde si la réponse commence ou termine par "ERROR"
		idx = strlen(response) - 9;

		if((strcmp("ERROR", response) < 0 )|| ((idx >= 0) && (strcmp(&response[idx],"\r\nERROR\r\n") == 0)))
		{
		   answer = 4; //Reponse ERROR
	      break;
		}

		//On lit les lignes suivantes tant qu'on n'a pas le réponse escomptée
		answer = SIM900_readLine(response, timeout, 1); //true
	}

 return answer;
}

unsigned char SIM900_sendAtCommandAndCheckOK(const char *command, char *response, unsigned int timeout)
{
  SIM900_sendAtCommand(command);
	 return SIM900_checkOK(response, timeout);
}

char *SIM900_readSMS(unsigned char smsIdx, char *num_tel, unsigned int timeout)
{
  unsigned char answer;

  memset(print_buf, '\0', 32);
  sprintf(print_buf, "AT+CMGR=%d,0\r", smsIdx);

  answer = SIM900_sendAtCommandAndCheckOK(print_buf, buffer, timeout);

	if((answer == 0) && (strcmp("+CMGR:", buffer) < 0) )
	{
		char* tmp = strchr(&buffer[2],10);
		char* msg = &tmp[1];
		tmp[0] = '\0';

		strtok(buffer,"\"");               //1° token : +CMGR:
		//////char* stat = strtok(NULL,"\"");    //2° token : REC UNREAD ou REC READ
		strtok(NULL,"\"");                 //3° token : ","
		char* tel = strtok(NULL,"\"");     //4° token : n° de tel
		//Serial.print("stat:"); Serial.println(stat);
		//Serial.print("tel :"); Serial.println(tel);

		if(num_tel != NULL) strcpy(num_tel,tel);
		return msg;
	}
	else{
		//Serial.print("response:"); Serial.println(answer);
		return NULL;
	}
}

unsigned char SIM900_waitPrompt(unsigned int timeout)
{
  unsigned char c;//, idx = 0;
  unsigned char answer = 2;  //Timeout par défaut
  unsigned int tt = 0;

	//unsigned long previous = millis();

	while( tt < timeout )//(millis()-previous)<timeout){
	{
		if(UART_Available() != 0)
		{
		  c = UDR0;

			if(c==10 || c==13)
			{
			  //on ignore les caractères CR LF
			}
			else if(c=='>')
			{
			  //on a le prompt
			  answer = 0;
			  break;
			}
		}

	  ++tt;
	  _delay_us(100);
	}
  return answer;
}

unsigned char SIM900_sendSMS(char *phoneNumber, char *msg)
{
  unsigned char answer;
  char  *s;

   s = msg;

   sprintf(&buffer[0],"AT+CMGS=\"%s\"\r",phoneNumber);
//"AT+CMGS=\"0961150566\"");
	SIM900_sendAtCommand(&buffer[0]);
	answer = SIM900_waitPrompt(20000);

	if(answer == 0)
	{
		//Envoi du message
		//Serial.print(">"); Serial.println(msg);

		//simSwSerial->println(msg);

		while(*s)
		{
		  USART_Transmit(*s++);
		}

		//simSwSerial->write(0x1A);
		USART_Transmit(0x1A);

		//On attend la réponse
		answer = SIM900_checkOK(buffer,50000);
	}

  return answer;
}
