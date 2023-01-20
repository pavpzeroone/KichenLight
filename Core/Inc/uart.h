/*
 * sim900.h
 *
 *  Version 1.0.0
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

// для разных процессоров потребуется проверить функцию DBG_Init
// на предмет расположения ножек USART
//#include "stm32f10x.h"
//#include "stm32f10x_rcc.h"
//#include "stm32f10x_gpio.h"
//#include "stm32f10x_usart.h"
//#include "stm32f10x_dma.h"
//#include "misc.h"

#include "stm32f1xx_hal.h"
//#include "comm.h"		//Командный файл

void UART_Rx_Start(UART_HandleTypeDef *huart);
void UART_Tx_Handler(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);			

//extern UART_HandleTypeDef *const Used_uart = &huart3;		//Выбор используемого UARTa

// выбираем, на каком USART для вывода отладочной информации
#define USE_USART3
//#define USE_USART2
//#define USE_USART3
//#define USE_USART4

#ifdef USE_USART1

#undef USE_USART2
#undef USE_USART3
#undef USE_USART4

#define USE_USART 			USART1
#define USART_DMA_CH_RX 	DMA1_Channel5
#define USART_DMA_CH_TX 	DMA1_Channel4
#define USART_DMA_FLAG		DMA1_FLAG_TC4
#define USART_IRQn				USART1_IRQn

#endif


#ifdef USE_USART2

#undef USE_USART1
#undef USE_USART3
#undef USE_USART4

#define USE_USART 			USART2
#define DBG_DMA_CH_RX 	DMA1_Channel6
#define DBG_DMA_CH_TX 	DMA1_Channel7
#define DBG_DMA_FLAG		DMA1_FLAG_TC6
#define DBG_IRQn				USART2_IRQn

#endif


#ifdef USE_USART3

#undef USE_USART1
#undef USE_USART2
#undef USE_USART4

#define USE_USART 			USART3
//#define OW_DMA_CH_RX 	DMA1_Channel6
//#define OW_DMA_CH_TX 	DMA1_Channel7
//#define OW_DMA_FLAG		DMA1_FLAG_TC6
#define DBG_IRQn				USART3_IRQn

#endif

//extern char USART_Tx_Buf[220];
typedef enum
{
	Find_None,
	Find_0D,
	Find_0D0A,
	Find_0D0A_Msg,
	Find_Msg_0D
} Find_type;

#define Timer_Wait_DBG_Handler	150	//0,15 сек
#define Command_Retry_Max				3		//кол-во повторов команды
#define Answer_Wait_Count_Max		3		//кол-во циклов ожидания ответа

//Мнемоники режима работы GSM
#define m_Off								0
#define m_Switch_On					1
#define m_Prepare						2
#define m_Listen						3
#define m_Send_SMS					4
#define m_Switch_Off				5
#define m_Error							127

//Мнемоники команд
#define c_AOH								0
#define c_Operator_Info			1
#define c_Operators_Info		2
#define c_DBG_Revision  		3
#define c_DBG_Ident     		4
#define	c_SMS_Format				5
#define	c_SMS_Coding				6
#define	c_SMS_Special				7
#define	c_SMS_Send					8
#define	c_Tube_Up						9
#define	c_Tube_Down					10
#define	c_AT								11

//Мнемоники параметров
#define p_Null							0
#define p_Off								1
#define p_On								2
#define p_0									1
#define p_1									2
#define p_GSM								3

//Мнемоники ответов
//#define m_Q			 								1
//#define m_CME_ERROR			 				2
#define m_CPIN			 						3
#define m_V			 								4	
//#define m_BUSY			 						5
#define m_Call_Ready			 			6
//#define m_NO_ANSWER			 				7
//#define m_NO_CARRIER			 			8
//#define m_NO_DIALTONE			 			9
//#define m_NORMAL_POWER_DOWN			10
#define m_OK			 11
#define m_RDY			 12
//#define m_RESET			 13
#define m_RING			 14
#define m_CLIP			 15
#define m_CFUN			 16


#define	a_No_Answer					127

#define r_Srch_Null						0
#define r_Srch_ChrFound				1
#define r_Srch_Complete				2

//extern uint32_t	USART_Show_Reg;			//Регистр установки бит вывода информации по USART
//#define s_BEAN	1
//#define s_CAN		2

#define Text_RX_Buf_Len 	196
typedef struct 
{	
	uint8_t Text[Text_RX_Buf_Len];
	volatile uint8_t Write_Pos;
	volatile uint8_t Read_Pos;
	uint8_t Len;
}Text_RX_Buf_struct;

#define Text_TX_Buf_Len 	194
typedef struct 
{	
	uint8_t Text[Text_TX_Buf_Len];
	volatile uint8_t Write_Pos;
	volatile uint8_t Read_Pos;
	uint8_t Len;
}Text_TX_Buf_struct;

//Буфер приёма
//Text_Buffer		Rx_Buf={{0}, 0, 0, Text_Buf_Len};
//#define 			Rx_Buf_Len sizeof Rx_Buf.Text

typedef struct
{
	Text_RX_Buf_struct	RX_Buf;
	Text_TX_Buf_struct	TX_Buf;
	uint8_t							TX_Busy;
} Uart_struct;

extern Uart_struct	Uart;

char HByte_to_Hex(uint8_t HalfByte);

void USART_BEAN_Show(char *PRI, char *MSL, char *DST_ID, char *MES_ID, char *DATA );
void USART_СAN_Show(char *IDE, uint32_t *ID, uint32_t *ExID, char *RTR, char *DLC, char *DATA );
//void USART_DS18B20_Show(void);

extern char DBG_Work_Mode;
extern char DBG_Handler_Step;

void USART_Setup(void);
void USART_Send(char *data, uint8_t pos, uint8_t dLen);
void DBG_Mode_Set(char Mode);
void DBG_Wait_Answer(char Answer);

void USART_Buf_Rx_Handler(void);
void UART_Tx_Handler(UART_HandleTypeDef *huart);
uint8_t UART_Send_Chr(const uint8_t *Chr);
uint8_t UART_Send_Str(const uint8_t *Chr, uint8_t Size);
void UART_Send_uint16(uint16_t Digit);
void UART_Send_Time(int16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second);
//unsigned int DBG_Handler(void);
//void DBG_Send_Command(char Command_Number, char Param_Number);
//void DBG_Send_Command_SMS(char* Phone_Number);
//void DBG_Send_SMS_Text(void);

// extern Command Msg_Command;

//Внешние переменные main
//extern uint32_t DelayTime_USART;

//Переменные OneWire
extern uint8_t OW_Device_Count;

//Внешние данные ds18b20
extern void DS_Text_Temp(char *Text, char Sens_Nmb);
