#include "stm32f1xx_hal.h"

#ifndef Comm_h
#define Comm_h

typedef struct 
{
	char 	Number;
	char 	Key;
	int		Value;
}Command_struct;

//Список команд
extern const uint8_t	Msg_List[];
extern const uint16_t	Msg_List_Len;
extern const uint8_t	Msg_Spacer;//=0x5F;	//"_" - Символ разделитель слов-команд для базы слов-команд

extern const uint8_t	Key_List[];
extern const uint16_t	Key_List_Len;

extern const uint8_t	Hex_List[];
extern const uint16_t	Hex_List_Len;

extern const uint8_t	chr_0A;
extern const uint8_t	chr_0D;

//Имена команд
enum m_
{
	m_null = 0,
	m_Q,
	m_RELAY,
	m_LED1,
	m_LED2,
	m_LED3,
	m_LED4,
	m_LED5,
	m_LED6,
	m_VBAT_SHOW,
	m_VSOLAR_SHOW,
	m_TIME_SHOW,
	m_TIME_SET
};


#define m_H			 1
//#define m_Q			 2
#define m_1WIRE_DETECT			 3
#define m_1WIRE_SHOW_COUNT			 4
#define m_1WIRE_SHOW_ID			 5
#define m_1WIRE_WORK			 6
#define m_BEAN_SEND			 7
#define m_BEAN_SHOW			 8
#define m_BEAN_TEST_SIGNAL			 99
#define m_CAN_FILTER_SET			 100
#define m_CAN_SEND			 11
#define m_CAN_SHOW			 12
#define m_CAN_TEST_SIGNAL			 13
#define m_CONNECT			 14
#define m_DS18B20_REQUEST			 15
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

//Имена ключей
#define k_0			 1
#define k_1			 2
#define k_DISABLE			 3
#define k_ENABLE			 4
#define k_OFF			 5
#define k_ON			 6

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
#define t_USART_Can_Show			1<<3
#define	t_CAN_Test_Signal			1<<5

#define t_BEAN_Test_Signal		1<<8

#define t_Vbat_Show						1<<9
#define t_Vsolar_Show					1<<10
#define t_Time_Show						1<<11

void Str_From_List(const uint8_t *Str, uint8_t *Len, const uint8_t *Comm_List, char Comm_N);
void Text_From_List( uint8_t *Text, uint8_t *Len, const uint8_t *Comm_Str, char Comm_N );
extern char Command_Write(char Number, char Key, int Value);
void Command_Exec(void);

void Send_Ansver_from_List(char Msg, char Key);

void Vbat_Show(uint16_t V);
void Vsolar_Show(uint16_t V);
#endif
