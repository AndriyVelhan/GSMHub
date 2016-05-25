#ifndef SIM900_H_
#define SIM900_H_
#define BUFFER_LEN 256



void SIM900_Init(void);
void SIM900_OnOff(void);
void SIM900_Pool(void);
//unsigned char SIM900_Check(unsigned char *m);


unsigned char UART_Available(void);
unsigned char USART_Receive(void);


unsigned char SIM900_sendAtCommandAndCheckOK(const char *command, char *response, unsigned int timeout);
char *SIM900_readSMS(unsigned char smsIdx, char *num_tel, unsigned int timeout);
unsigned char SIM900_sendSMS(char *phoneNumber, char *msg);

#endif
