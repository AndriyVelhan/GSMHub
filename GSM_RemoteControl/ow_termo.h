#ifndef OW_TERMO_H_
#define OW_TERMO_H_

typedef struct
{
   unsigned char T;
   unsigned char T_tenths;

} TemperatureData;


void OW_Init(void);
unsigned char OW_Reset(void);
void OW_WriteByte(unsigned char bb);
unsigned char OW_ReadByte(void);
void OW_MatchROM(const unsigned char *rom);
unsigned char OW_ReadROM(unsigned char *rom_id);

unsigned char OW_Start_Tem_Convert(const unsigned char *rom);
unsigned char OW_Get_Temperature(const unsigned char *rom, TemperatureData *td);

unsigned char OW_SetSwitchState(const unsigned char *rom, unsigned char state);

#endif
