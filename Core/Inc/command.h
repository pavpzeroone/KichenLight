#include "stm32f1xx_hal.h"

#ifndef Comm_h
#define Comm_h

typedef struct 
{ char 	Number;		//Номер команды
	char 	Key;			//Ключь на исполнения команды ( k_0, k_1, ... )
	int		Value;		//Переданое значение (цифра)
}Command_struct;

typedef struct 
{ char 	Led_Nbr;	//Номер канала света
	int		Value;		//Значение яркости (0 - 1000)
}ManualLed_struct;

typedef struct 		//Для установки времени
{ volatile uint16_t Year;
	volatile uint16_t Month;
	volatile uint16_t Day;
	volatile uint16_t Hour;
	volatile uint16_t Minute;
	volatile uint16_t Second;
}ManualTime_struct;

//Список команд
extern const uint8_t	Msg_List[];
extern const uint16_t	Msg_List_Len;
extern const uint8_t	Msg_Spacer;//=0x5F;	//"_" - Символ разделитель слов-команд для базы слов-команд

extern const uint8_t	Key_List[];
extern const uint16_t	Key_List_Len;

extern const uint8_t	Hex_List[];
extern const uint16_t	Hex_List_Len;
extern const uint16_t	Dec_List_Len;

extern const uint8_t 	NhexChar_spc;
extern const uint8_t	NhexChar_dot;
extern const uint8_t	NhexChar_2dot;
extern const uint8_t	NhexChar_0;
extern const uint8_t	NhexList_spc;
extern const uint8_t	NhexList_dot;
extern const uint8_t	NhexList_2dot;
extern const uint8_t	NhexList_0;

//extern const uint8_t	chr_0A;
//extern const uint8_t	chr_0D;

//Имена команд
enum m_
{
	m_null = 0,
	m_Q,
	m_DEBUG,	
	m_LED1,
	m_LED2,
	m_LED3,
	m_LED4,
	m_LED5,
	m_LED6,
	m_LUXDATA_SHOW,
	m_RELAY,
	m_RESET,
	m_TIME_SET,
	m_TIME_SHOW,
	m_VBAT_SHOW,
	m_VSOLAR_SHOW	
};

//Имена ключей команд
#define k_0			 			1
#define k_1			 			2
#define k_DISABLE			3
#define k_ENABLE			4
#define k_OFF			 		5
#define k_ON			 		6
#define k_END_OF_MSG	255

//Имена ответов
#define a_DISABLE			 1
#define a_ENABLE			 2
#define a_ERROR			 3
#define a_OFF			 4
#define a_OK			 5
#define a_ON			 6

//Режимы парсинга
#define p_Msg				0
#define p_Key				1
#define p_Value			2
#define p_HexValue 	3
#define p_RawValue 	4

extern unsigned int	Comm_Task;			//Переменная содержащая биты разрешения работы задачь
extern ManualLed_struct ManualLedSw;
extern ManualTime_struct ManualTime;

#define t_Vbat_Show						0b10000000	//(uint32_t) 1<<3
#define t_Vsolar_Show					0b01000000	//(uint32_t) 1<<4
#define t_Time_Show						0b00100000	//(uint32_t) 1<<5
#define t_Time_Set						0b00010000	//(uint32_t) 1<<5
#define t_LuxData_Show				0b00001000	//(uint32_t) 1<<6
#define t_Debug								0b00000001

extern uint8_t Command_Write(uint8_t Number, uint8_t Key, uint16_t Value);
void Command_Exec(void);
volatile uint8_t const *get_StrFromList( uint8_t const* List, char N );
uint8_t get_LenListStr( volatile uint8_t const* Str );

uint8_t Send_uint16(uint16_t Digit);
uint8_t Send_BitsByte(uint8_t Digit);
void Send_Answer_from_List(uint8_t Msg, uint8_t Key);

void Vbat_Show(uint16_t V);
void Vsolar_Show(uint16_t V);
void Time_Show(int16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second);
uint8_t LuxData_Show(uint16_t* Lux, uint16_t len, uint16_t pos);
void Debug_Show(uint8_t Mode, uint8_t MSensL, uint8_t MSensR, uint8_t Consumers);
#endif
