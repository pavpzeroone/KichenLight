#include "stm32f1xx_hal.h"

#ifndef Comm_h
#define Comm_h

typedef struct 
{ char 	Number;
	char 	Key;			//Ключь на исполнение команды ( 1 / 0 )
	int		Value;
}Command_struct;

typedef struct 
{ char 	Led_Nbr;	//Номер канала света
	int		Value;		//Значение яркости (0 - 1000)
}ManualLed_struct;

typedef struct 
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

extern const uint8_t	chr_0A;
extern const uint8_t	chr_0D;

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


#define m_H			 1
//#define m_Q			 2
#define m_1WIRE_DETECT			 33
#define m_1WIRE_SHOW_COUNT			 44
#define m_1WIRE_SHOW_ID			 55
#define m_1WIRE_WORK			 66
#define m_BEAN_SEND			 77
#define m_BEAN_SHOW			 88
#define m_BEAN_TEST_SIGNAL			 99
#define m_CAN_FILTER_SET			 100
#define m_CAN_SEND			 11
#define m_CAN_SHOW			 112
#define m_CAN_TEST_SIGNAL			 130
#define m_CONNECT			 14
#define m_DS18B20_REQUEST			 115
#define m_DS18B20_SHOW_TEMP			 16
#define m_HEATER1			 17
#define m_HEATER2			 18
#define m_HEATER3			 19
#define m_HEATER4			 20
#define m_LC_DRL_DEMO			 21
#define m_LC_DRL_LED			 22
#define m_LCD_TEMP_SHOW			 23
//#define m_RELAY			 24
#define m_RESET			 25

//Имена ключей команд
#define k_0			 			1
#define k_1			 			2
#define k_DISABLE			3
#define k_ENABLE			4
#define k_OFF			 		5
#define k_ON			 		6
//#define k_GO					254
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
#define t_USART_Can_Show			(uint32_t) 1<<20
#define	t_CAN_Test_Signal			(uint32_t) 1<<29

#define t_BEAN_Test_Signal		(uint32_t) 1<<28

#define t_Vbat_Show						0b10000000	//(uint32_t) 1<<3
#define t_Vsolar_Show					0b01000000	//(uint32_t) 1<<4
#define t_Time_Show						0b00100000	//(uint32_t) 1<<5
#define t_Time_Set						0b00010000	//(uint32_t) 1<<5
#define t_LuxData_Show				0b00001000	//(uint32_t) 1<<6
#define t_Debug								0b00000001

volatile uint8_t const *get_StrFromList( uint8_t const* List, char N );
uint8_t get_LenListStr( volatile uint8_t const* Str );
//void Str_From_List(const uint8_t *Str, uint8_t *Len, const uint8_t *Comm_List, char Comm_N);
void Text_From_List( uint8_t *Text, uint8_t *Len, const uint8_t *Comm_Str, char Comm_N );
extern uint8_t Command_Write(uint8_t Number, uint8_t Key, uint16_t Value);
void Command_Exec(void);

uint8_t Send_uint16(uint16_t Digit);
uint8_t Send_BitsByte(uint8_t Digit);
void Send_Answer_from_List(uint8_t Msg, uint8_t Key);

void Vbat_Show(uint16_t V);
void Vsolar_Show(uint16_t V);
void Time_Show(int16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second);
uint8_t LuxData_Show(uint16_t* Lux, uint16_t len, uint16_t pos);
void Debug_Show(uint8_t Mode, uint8_t MSensL, uint8_t MSensR, uint8_t Consumers);
#endif
