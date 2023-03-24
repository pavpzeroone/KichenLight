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

#include "stm32f1xx_hal.h"

void UART_Rx_Start(UART_HandleTypeDef *huart);
void UART_Tx_Handler(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);			

#define RX_Buf_Len 	196
typedef struct 
{	
	uint8_t Text[RX_Buf_Len];
	volatile uint8_t Write_Pos;
	volatile uint8_t Read_Pos;
}Text_RX_Buf_struct;

#define TX_Buf_Len 	194
typedef struct 
{	
	uint8_t Text[TX_Buf_Len];
	volatile uint8_t Write_Pos;
	volatile uint8_t Read_Pos;
}Text_TX_Buf_struct;

typedef struct
{
	Text_RX_Buf_struct	RX_Buf;
	Text_TX_Buf_struct	TX_Buf;
	uint8_t							TX_Busy;
} Uart_struct;

extern Uart_struct	Uart;

typedef enum
{
	Find_None,
	Find_0D,
	Find_0D0A,
	Find_0D0A_Msg,
	Find_Msg_0D
} Find_type;

#define r_Srch_Null						0
#define r_Srch_ChrFound				1
#define r_Srch_Complete				2

void UART_Rx_Handler(void);
void UART_Tx_Handler(UART_HandleTypeDef *huart);
uint8_t UART_TXbuf_Space_Check();
uint8_t UART_Send_Chr(uint8_t const* Chr);
uint8_t	UART_Send_Str(uint8_t const *Str, uint8_t Size);
