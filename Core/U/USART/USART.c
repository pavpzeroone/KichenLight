/*
 * sim900.c
 *
 *  Created on: 30.01.2014
 *      Author: Ptitsyn PS
 */

#include "USART.h"

//#include "ds18b20.h"
//#include "LCD.h"
#include "stdio.h"

//// Буфер для приема/передачи по 1-wire
//#define USART_Buf_Len sizeof DBG_Buf
//#define USART_Rx_Buf_Len sizeof USART_Rx_Buf
////#define Command_Str_Len sizeof Command_Str
//#define Answer_Str_Len sizeof Answer_Str
//#define Answer_Buf_Len sizeof Answer_Buf

//const char Command_Str[]="AT+CLIP=_AT+COPS?_AT+COPS=?_AT+GMR_AT+GMM_AT+CMGF=_AT+CSCS=_AT+CSCB=_AT+CMGS=_ATA_ATH0_AT_";
//const char Param_Str[]={'_','0','_','1','_','"','G','S','M','"','_'};
//const char Answer_Str[]="_+CFUN: _+CLIP: _+CME ERROR:_+CPIN: _> _BUSY_Call Ready_NO ANSWER_NO CARRIER_NO DIALTONE_NORMAL POWER DOWN_OK_RDY_RING_";

//const char Msg_List[]="_?_RESTART_";

//Параметры телефонного номера
//char Phone_Number[14]="+79139735344_";
//unsigned char Phone_Number_Type;
//unsigned char Phone_Number_Valid;

//Буфер номеров полученных ответов из списка
//char Answer_Buf[8]={0,0,0,0,0,0,0,0};
//char Answer_Buf_Pos=0;
//char Answer_Buf_Read_Pos=0;

char Answer_Wait_Pos=0xFF;
char Answer_Wait=0;
//Переменные поиска ответов из списка
char Answer_Str_Pos;
char Answer_Chr_Count;
//char Answer_Temp_Nmb;

char	Answer_Mode;
char	Answer_Param_Nmb;

char Answer_Param_Skob;

char DBG_Work_Mode=m_Off;
char DBG_Handler_Step=0;

//Обработчик команд и ответов от GSM
//char Command_Retry_Count=0;				//кол-во повторов команды
//char Answer_Wait_Count=0;					//кол-во раз ожидания ответа

//char USART_Tx_Buf[220];
//char USART_Rx_Buf[30];	//Кол-во кратное 2
//uint8_t	DBG_Rx_Counter=0,
//				DBG_Rx_Pos=0;
								
//char	Rx_Buf_Reg=0;	//Регистр для обработки 0d0a в посылках USART

//char USART_Rx_Buf_WritePos=0;
//char USART_Rx_Buf_Read_Pos=0;

//Переменные для вывода на LCD
// char	LCD_Line=0;
// char	LCD_Colon=0;

// char Test_Var=0;

USART_struct	USART;

//Переменные не относящиеся к непосредственной работе USART
//uint32_t	USART_Show_Reg;			//Регистр установки бит вывода информации по USART

//Буфер отдачи
// Buffer	Tx_Buf={{0},0,0};
// #define Tx_Buf_Len sizeof Tx_Buf.Text

// typedef struct Command_Buffer
// {	
// 	char Number[6];
// 	uint32_t Param[6];
// 	char WritePos;
// 	char ReadPos;
// }Command_Buffer;

// Command_Buffer 	USART_Command={{0,0,0,0,0,0},{0,0,0,0,0,0},0,0};
// #define 				USART_Command_Len sizeof USART_Command.Number

//Command Msg_Command={0,0,0};

//Private function-----------------------------------------------------------
uint8_t SearchChr_in_List(char *Chr, const char *List, int *List_Len, char *List_Msg_Number, char *ListPos, char *Found_Crh_Count);
void	USART_Command_NextWritePos(void);


void UART_Rx_Handler()
{
	HAL_UART_Receive_IT (&huart3, buf, 1); // запуск приема
}


//----------------------------------------------------------------------------
//Прерывания USART-ов
//----------------------------------------------------------------------------
void USART_IRQHandler(void)	//Тело прерывания общее для всех USART1-4
{
	//if ( USART_GetITStatus( USE_USART, USART_IT_RXNE) ) USART_ClearITPendingBit( USE_USART, USART_IT_RXNE);
	//если причина прерывания окончание приема
  //if((USART1->SR & USART_SR_RXNE)!=0)	
	
	if (USART_GetITStatus( USE_USART, USART_FLAG_ORE)== SET) 
	{
			//USART.RX_Buf.Write_Pos = 0;
		USART.RX_Buf.Text[ USART.RX_Buf.Write_Pos ] = USART_ReceiveData( USE_USART );
		if( ++USART.RX_Buf.Write_Pos == USART.RX_Buf.Len ) USART.RX_Buf.Write_Pos = 0;
	}
	
		
	USART.RX_Buf.Text[ USART.RX_Buf.Write_Pos ] = USART_ReceiveData( USE_USART );
	if( ++USART.RX_Buf.Write_Pos == USART.RX_Buf.Len ) USART.RX_Buf.Write_Pos = 0;

	//USART_ClearITPendingBit( USE_USART, USART_IT_RXNE | USART_FLAG_ORE);	
}

void USART1_IRQHandler(void)
{	
	#ifdef USE_USART1
		USART_IRQHandler();	
	#endif
}

void USART2_IRQHandler(void)
{
	#ifdef USE_USART2
		USART_IRQHandler();
	#endif
}

void USART3_IRQHandler(void)
{
	#ifdef USE_USART3
		USART_IRQHandler();
	#endif
}

void USART4_IRQHandler(void)
{
	#ifdef USE_USART4
		USART_IRQHandler();
	#endif
}

//-----------------------------------------------------------------------------
// инициализирует USART и DMA
//-----------------------------------------------------------------------------
void USART_Setup(void) 
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	#ifdef USE_USART1
	//if (USE_USART == USART1) 
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		// USART TX
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_AF_OD;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOB, &GPIO_InitStruct);

		// USART RX
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOB, &GPIO_InitStruct);	
		
		/*Configure peripheral I/O remapping */
		GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	}
	#endif

	#ifdef USE_USART2
	//if (USE_USART == USART2) 
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
		
		// USART TX
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;//GPIO_Mode_AF_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		// USART RX
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStruct);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	}
	#endif
	
	#ifdef USE_USART3
	//if (USE_USART == USART3) 
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

		// USART TX
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;//GPIO_Mode_AF_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOB, &GPIO_InitStruct);

		// USART RX
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStruct);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	}
	#endif

	/* Enable the USARTz Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	USART_InitStructure.USART_BaudRate = 1000000;//115200;//19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USE_USART, &USART_InitStructure);
	
	/* Enable the USARTz Receive Interrupt */
	USART_ITConfig(USE_USART, USART_IT_RXNE, ENABLE);	
	USART_Cmd(USE_USART, ENABLE);

		// старт цикла отправки
		USART_ClearFlag(USE_USART, USART_FLAG_RXNE /*| USART_FLAG_TC | USART_FLAG_TXE*/);
		//USART_DMACmd(USE_USART, USART_DMAReq_Rx, ENABLE);
		//DMA_Cmd(DBG_DMA_CH_RX, ENABLE);
		//DMA_Cmd(USART_DMA_CH_RX, ENABLE);
	
	// Здесь вставим разрешение работы USART в полудуплексном режиме 
  //USART_HalfDuplexCmd(USE_USART, ENABLE);	
	
	//Инициализация переменных USART
	USART.RX_Buf.Len = Text_RX_Buf_Len;
	USART.TX_Buf.Len = Text_TX_Buf_Len;
}

void AmoHeat_USART_Setup(void) 
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// USART TX
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// USART RX
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStruct);	


	/* Enable the USARTz Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	USART_InitStructure.USART_BaudRate = 1000000;//115200;//19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USE_USART, &USART_InitStructure);
	
	/* Enable the USARTz Receive Interrupt */
	USART_ITConfig(USE_USART, USART_IT_RXNE, ENABLE);	
	USART_Cmd(USE_USART, ENABLE);

		// старт цикла отправки
		USART_ClearFlag(USE_USART, USART_FLAG_RXNE /*| USART_FLAG_TC | USART_FLAG_TXE*/);
		//USART_DMACmd(USE_USART, USART_DMAReq_Rx, ENABLE);
		//DMA_Cmd(DBG_DMA_CH_RX, ENABLE);
		//DMA_Cmd(USART_DMA_CH_RX, ENABLE);
	
	// Здесь вставим разрешение работы USART в полудуплексном режиме 
  //USART_HalfDuplexCmd(USE_USART, ENABLE);	
	
	//Инициализация переменных USART
	USART.RX_Buf.Len = Text_RX_Buf_Len;
	USART.TX_Buf.Len = Text_TX_Buf_Len;
}

//-----------------------------------------------------------------------------
void USART_Send(char *data, uint8_t pos, uint8_t dLen) 
{
		DMA_InitTypeDef DMA_InitStructure;

		// DMA на запись
		DMA_DeInit(USART_DMA_CH_TX);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USE_USART->DR);
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &data[pos];
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_BufferSize = dLen;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(USART_DMA_CH_TX, &DMA_InitStructure);

		// старт цикла отправки
		USART_ClearFlag(USE_USART, /*USART_FLAG_RXNE |*/ USART_FLAG_TC | USART_FLAG_TXE);
		USART_DMACmd(USE_USART, USART_DMAReq_Tx /*| USART_DMAReq_Rx*/, ENABLE);
		//DMA_Cmd(USART_DMA_CH_RX, ENABLE);
		DMA_Cmd(USART_DMA_CH_TX, ENABLE);
}

// Обработчик принятых символов _.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.__
void USART_Buf_Rx_Handler(void)
{
	static char Detector_Msg = Find_None;
	static char Search_Mode;
	
	static char Msg_Number;			//Возможно нужен вынос за процедуру
	static char Msg_List_Pos;		//Возможно нужен вынос за процедуру
	
	static char Search_Result;
	
	//Проверям есть ли новый символ в буфере
	if( USART.RX_Buf.Write_Pos !=  USART.RX_Buf.Read_Pos )
	{	
		//Проверяем какой символ приняли
		switch( USART.RX_Buf.Text[ USART.RX_Buf.Read_Pos ] )
		{	
			case 0x0D:
			{
				if(Detector_Msg==Find_0D0A_Msg) Detector_Msg=Find_Msg_0D;
				else 														Detector_Msg=Find_0D;
				break;
			}
			case 0x0A:
			{
				if(Detector_Msg==Find_0D)			Detector_Msg=Find_0D0A;	//Ничего не делаем
				if(Detector_Msg==Find_Msg_0D) Detector_Msg=Find_0D0A;
				break;
			}
			default:
			{
				if(Detector_Msg==Find_0D0A)	
				{	Detector_Msg=Find_0D0A_Msg;	//Найдено начало сообщения
					
					//Инициализация переменных задействованных в поиске команд
					//Answer_Str_Pos=0;					
					//Answer_Temp_Nmb=0;
					Msg_List_Pos=0;
					Msg_Number=0;	//Номер команды из списка Msg_List, временный
					Search_Mode=0;//Режим поиска (0 - определение команды, 1-255 - определение параметров команды)
				}
				if((Detector_Msg==Find_Msg_0D)||(Detector_Msg==Find_0D))
				{
					Detector_Msg=Find_None;
					//перевод строки LCD экрана
				}
			
				//=====================================Парсинг===================================				
				if(Detector_Msg==Find_0D0A_Msg)
				switch(Search_Mode)
				{
					case p_Msg: //Поиск ответов по списку Msg_List---------------------------------
					{										
						Search_Result = SearchChr_in_List( &USART.RX_Buf.Text[ USART.RX_Buf.Read_Pos ], &Msg_List[0], &Msg_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						
						//Если найдено полное совпадение слова+++++++++++++++++++++++++
						if(Search_Result == r_Srch_Complete)
						{
							//Заносим номер сообщения в буфер и меняем режим поиска в зависимости от принятой команды
							Search_Mode = Command_Write( Msg_Number,0,0 );
/*							USART_Command.Number[USART_Command.WritePos] = Msg_List_Pos;
							//Увеличение USART_Command.WritePos (Увеличение USART_Command.ReadPos если WritePos залезло на ReadPos)
							USART_Command_NextWritePos();
*/								
								/* //В зависимости от полученного сообщения выбираем требуется ли парсинг дополнительных параметров сообщения
								switch(Msg_List_Pos)							//Выбор режима развора ответа в зависимости от определённого ответа
								{
									case m_Q:
									{//Ничего не требуется										
									break;}
// 										case m_CLIP:
// 										{
// 											Search_Mode=Msg_List_Pos;		//Режим поиска параметров
// 											Answer_Param_Nmb=0;
// 											Answer_Param_Skob=0;
// 											break;
// 										}
										case m_V:
										{
											Answer_Mode=Msg_List_Pos;		//Режим поиска параметров
										}
										default: {}
								}*/
								
							Msg_Number=0;
							Msg_List_Pos=0;	//Обнуляем номер найденого сообщения
							Answer_Chr_Count=0;
						}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						break;
					}//------------------------------------------------------------------------------
					
					case p_Key:	//Поиск ключа по списку Key_List-------------------------------------
					{
						if( USART.RX_Buf.Text[ USART.RX_Buf.Read_Pos ]==32)break;	//Делаем пропуск пробелов
						
						Search_Result = SearchChr_in_List( &USART.RX_Buf.Text[ USART.RX_Buf.Read_Pos ], &Key_List[0], &Key_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						
						//Если найдено полное совпадение слова+++++++++++++++++++++++++
						if(Search_Result == r_Srch_Complete)
						{
							Search_Mode = Command_Write( 0, Msg_Number, 0 );
						}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						
					break;}	//-----------------------------------------------------------------------
				
					case p_Value:	//Поиск ключа по списку Key_List-----------------------------------
					{
						
					break;}	//-----------------------------------------------------------------------		
					
					case p_HexValue:	//Поиск ключа по списку Hex_List-------------------------------
					{
						Search_Result = SearchChr_in_List( &USART.RX_Buf.Text[ USART.RX_Buf.Read_Pos ], &Hex_List[0], &Hex_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						if((Search_Result == r_Srch_ChrFound) || (Search_Result == r_Srch_Complete))	//Нашли символ из списка Hex_List
						{
							Search_Mode = Command_Write( 0, 0, Msg_Number);
							
							Msg_Number=0;
							Msg_List_Pos=0;	//Обнуляем номер найденого сообщения
							Answer_Chr_Count=0;
						}
					break;}	//-----------------------------------------------------------------------						

					case p_RawValue:	//Считывание сырых данных -------------------------------------
					{ Search_Mode = Command_Write( 0, 0, USART.RX_Buf.Text[ USART.RX_Buf.Read_Pos ]);						
					break;}	//-----------------------------------------------------------------------									
					/*
					case m_CLIP:							//Поиск параметров ответов
					{
						switch(Answer_Param_Nmb)
						{
							case 0:	//Ищем ковычки и телефонный номер
							{
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x22)	//Если находим """ - ковычки
									if(Answer_Param_Skob!=0x22)
									{
										Answer_Param_Skob=0x22;	//Кавычка открылась
										Phone_Number_Type=0;		//Используем как счётчик символов номера
										break;
									}
									else 
									{
										Answer_Param_Skob=0;		//Кавычка закрылась
										Answer_Param_Nmb++;			//переходим к слудующему параметру
										Phone_Number[Phone_Number_Type]=0x5F;	//"_" - конец ответа
										Phone_Number_Type=0;
										Phone_Number_Valid=0;
										break;
									}
									
								if(Answer_Param_Skob==0x22) Phone_Number[Phone_Number_Type++]=USART_Rx_Buf[USART_Rx_Buf_Read_Pos];
									
								break;
							}
							case 6: {}
							case 1:	//Ищем запятую
							{
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x2C)	Answer_Param_Nmb++;	//Если находим "," - запятую то переходим к слудующему параметру												//
								break;
							}
							case 2:	//Тип номера
							{
								if((USART_Rx_Buf[USART_Rx_Buf_Read_Pos]>0x2F)&&(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]<0x3A))	//Цифры от 0-9
								{
									Phone_Number_Type*=10;
									Phone_Number_Type+=(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]-0x30);
									
									//sprintf(DBG_Tx_Buf, "%i", Phone_Number_Type);LCDPutStr(DBG_Tx_Buf, 8, 1+3*6*Test_Var++, SMALL, BLACK, RED);
								}
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x2C)	Answer_Param_Nmb++;	//Если находим "," - запятую то переходим к слудующему параметру
								break;
							}
							case 3:	//Субадрес (игнорируем)
							{
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x22)	//Если находим """ - ковычки
									if(Answer_Param_Skob!=0x22) {Answer_Param_Skob=0x22;}	//Кавычка открылась									
									else 
									{
										Answer_Param_Skob=0;		//Кавычка закрылась
										Answer_Param_Nmb++;			//переходим к слудующему параметру																				
									}
								break;
							}
							case 4: //Тип субадресса (игнорируем)
							{
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x2C)	Answer_Param_Nmb++;	//Если находим "," - запятую то переходим к слудующему параметру
								break;
							}
							case 5:	//АльфаИД (игнорируем)
							{
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x22)	//Если находим """ - ковычки
									if(Answer_Param_Skob!=0x22) {Answer_Param_Skob=0x22;}	//Кавычка открылась									
									else 
									{
										Answer_Param_Skob=0;		//Кавычка закрылась
										Answer_Param_Nmb++;			//переходим к слудующему параметру																				
									}
								break;
							}
							
							case 7: //Правильность номера
							{
								if((USART_Rx_Buf[USART_Rx_Buf_Read_Pos]>0x2F)&&(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]<0x3A))	//Цифры от 0-9
								{
									Phone_Number_Valid*=10;
									Phone_Number_Valid+=(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]-0x30);
									
									//sprintf(DBG_Tx_Buf, "%i", Phone_Number_Valid);LCDPutStr(DBG_Tx_Buf, 8, 100+2*6*Test_Var++, SMALL, BLACK, RED);
								}
								if(USART_Rx_Buf[USART_Rx_Buf_Read_Pos]==0x2C)	Answer_Param_Nmb++;	//Если находим "," - запятую то переходим к слудующему параметру
								break;
							}
							default: {}
						}
						break;
					}
					case m_V:
					{
						//STM32vldiscovery_LEDToggle(LED4);
						break;
					}*/
				}
														//===========================================================================
								
				//Вывод на экран символа
//				LCDPutChar(USART_Rx_Buf[USART_Rx_Buf_Read_Pos], 80-(LCD_Line)*8, 1+6*LCD_Colon, SMALL, ORANGE, BLACK);
				//if(++LCD_Colon==22)	goto H_ln;
				
				break;
			}
		}
		if( ++USART.RX_Buf.Read_Pos == USART.RX_Buf.Len ) USART.RX_Buf.Read_Pos = 0;	//Переходим к следующему принятому символу
	}
}//_.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.__

//========================================================================================================
//======== Функция поиска совпадений принятых по USART символов со списком известных сообщений ===========
//========================================================================================================
//Chr 			- искомый символ
//List 			- список известных сообщений
//List_Len	- длинна списка List
//List_Msg_Number	- текущий порядковый номер сообщения в спике List
//ListPos		- позиция символа для сравнения в списке List
//Found_Crh_Count	- количество совпавших с Chr символов из List на данный момент
//========================================================================================================
uint8_t SearchChr_in_List(char *Chr, const char *List, int *List_Len, char *List_Msg_Number, char *ListPos, char *Found_Crh_Count)
{
	unsigned int i,k;
	char	Tmp;

Begin:	
	//Поиск совпадения с Chr первого символа из слова списка сообщений List---------------------------------
	if( *List_Msg_Number == 0 )	//Если ещё не было начато определние команды, приступаем
	{	
		Tmp = 0;											//Начальный номер слова списка сообщений List
		*Found_Crh_Count = 0;					//Обнуляем количество совпавших с Chr символов из List на данный момент
							
		for( i=0; i<*List_Len; i++)
		{
			if( List[i] == Msg_Spacer )		//Если находим "_"	
			{
				Tmp++;
				if( ++i < *List_Len )					//Если не упёрлись в конец списка ответов
					if( *Chr == List[i] )
					{									
						*List_Msg_Number = Tmp;
						*ListPos = i+1;							//Установливаем позицию следующего символа
						*Found_Crh_Count += 1;
						
						if (List[i+1] == Msg_Spacer) return r_Srch_Complete;	//Строка для определения команд в один символ
						else return r_Srch_ChrFound;
						//i=List_Len;		//Выход из for
					}
			}						
		}//Получаем Answer_Tmp_Nmb=0 - если не найден ответ на этот символ, либо номер ответа
		return r_Srch_Null;							
	}
	
	//Дальнейшее определение совпадений (2-й и последующие символы)------------------------------------------
	else
	{
		if( *Chr == List[*ListPos] )	//Если сиволы совпали
		{
			*ListPos += 1;								//Переходим на следующий символ
			*Found_Crh_Count += 1;				//++кол-во найденых символов
								
			if( List[*ListPos] == Msg_Spacer )	//Если находим "_" - конец слова
			{				
				return r_Srch_Complete;							//Считаем что ответ полностью совпал с образцом
			}
		}											
		//Текушее слово не совпало с принятыми символами, пробуем найти совпадения в следующих образцах слов
		else	
		{						
			for( i=*ListPos; i<*List_Len; i++ )
			{
				if( List[i] == Msg_Spacer )		//Если находим "_" 
				{	
					if( ++i < *List_Len )				//Если не упёрлись в конец списка ответов
					{
						*List_Msg_Number += 1;
						Tmp = 0;										//Обнуляем кол-во совпадений
						
						//Считаем совпадения символов	в ранее найденном слове и текущем						
						for( k=i; ((k<(i + *Found_Crh_Count))&&(k < *List_Len)); k++ )			
							if( List[*ListPos - *Found_Crh_Count + (k-i)] == List[k] ) Tmp++;
							
						if( Tmp == *Found_Crh_Count ) 	//Если все символы совпали
						{
							*ListPos = i+*Found_Crh_Count;
							goto Begin;
						}
					}								
				}
			}
			*List_Msg_Number = 0;	//Совпадений с образцами ответот не найдено - обнуляем номер сообщения
			return r_Srch_Null;
		}
		
	}
	return r_Srch_Null;
}//==================================================================================================

void USART_BEAN_Show(char *PRI, char *MSL, char *DST_ID, char *MES_ID, char *DATA )
{
char MSG_Len = 2+2*2+*MSL*3+2;	//0x0D 0x0A PRI MSL DST_ID MES_ID DATA 0x0D 0x0A
	
	if( (uint32_t) ( MSG_Len + USART.TX_Buf.Write_Pos) >= USART.TX_Buf.Len ) 	//Если длина сообщения выходит за границы буфера
	{	//Очищаем буфер
		USART.TX_Buf.Write_Pos = 0;
		USART.TX_Buf.Read_Pos = 0;
	}
	
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0D;
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0A;
	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%1X ", *PRI );		USART.TX_Buf.Write_Pos += 2;
	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%1X ", *MSL );		USART.TX_Buf.Write_Pos += 2;
	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DST_ID );	USART.TX_Buf.Write_Pos += 3;
	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *MES_ID );	USART.TX_Buf.Write_Pos += 3;
	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA );		USART.TX_Buf.Write_Pos += 3;
	
	switch( *MSL )
	{
		case 13:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case 12:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case 11:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case 10:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case  9:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case  8:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case  7:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case  6:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case  5:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		case  4:	{	DATA++;	sprintf( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], "%0*X ", 2, *DATA ); USART.TX_Buf.Write_Pos += 3; }
		default: 	{}		
	}
	
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0D;
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0A;		
	
	USART_Send( &USART.TX_Buf.Text[0], USART.TX_Buf.Read_Pos, MSG_Len );
	USART.TX_Buf.Read_Pos += MSG_Len;
}
//_________________________________________________________________________________
void USART_СAN_Show(char *IDE, uint32_t *ID, uint32_t *ExID, char *RTR, char *DLC, char *DATA )
{
char MSG_ID_Len 	= *IDE>>2 ? 9 : 4;			//Если расширенный код то 9 иначе 4
char MSG_Data_Len = *RTR>>1 ? 0 : *DLC*3;	//Если запрос данных то 0 иначе данные
char MSG_Len = 2 +MSG_ID_Len+2+1+MSG_Data_Len+ 2;	//0x0D 0x0A ID RTR DLC DATA 0x0D 0x0A
	
	if( (uint32_t) ( MSG_Len + USART.TX_Buf.Write_Pos) >= USART.TX_Buf.Len ) 	//Если длина сообщения выходит за границы буфера
	{	//Очищаем буфер
		USART.TX_Buf.Write_Pos = 0;
		USART.TX_Buf.Read_Pos = 0;
	}
	
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0D;
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0A;
	
	if( *IDE>>2 == 1 )	//Если расширенный код
	{
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>28 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>24 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>20 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>16 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>12 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>8 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>4 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ExID>>0 );
	}
	else								//Если стандартнный код
	{
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ID>>8 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ID>>4 );
		USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *ID>>0 );
	}	
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *RTR>>1 );
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DLC>>0 );	
		
	if( *RTR>>1 == 0 )
	switch( *DLC )
	{		
		case	8:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	7:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	6:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	5:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	4:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	3:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	2:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); DATA++; }
		case	1:	{ USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' '; USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>4 );	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = HByte_to_Hex ( *DATA>>0 ); }
		default: 	{}		
	}
	
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0D;
	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0A;		
	
	USART_Send( &USART.TX_Buf.Text[0], USART.TX_Buf.Read_Pos, MSG_Len );
	USART.TX_Buf.Read_Pos += MSG_Len;
}

//-=-=-=-=-Замена ф-ции sprintf
char HByte_to_Hex(uint8_t HalfByte)
	{
		HalfByte &= 0x0F;
		if( HalfByte >= 10) return('A' + HalfByte - 10); 
		else return('0' + HalfByte); 
	}

// uint8_t hexascii_to_halfbyte(uint8_t _ascii)
// {
// if((_ascii >= '0') && (_ascii <= '9')) return(_ascii — '0'); 
// if((_ascii >= 'a') && (_ascii <= 'f')) return(_ascii — 'a'); 
// if((_ascii >= 'A') && (_ascii <= 'F')) return(_ascii — 'A'); 
// return(0xFF);
// }


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//void USART_DS18B20_Show(void)
//{
//#include "onewire.h"
//#include "ds18b20.h"
//	
//char MSG_Len = 2+ (5+ 6 + 3 )* OW_Device.Count +2;
//char j;
//	
//	if( (uint32_t) ( MSG_Len + USART.TX_Buf.Write_Pos) >= USART.TX_Buf.Len ) 	//Если длина сообщения выходит за границы буфера
//	{	//Очищаем буфер
//		USART.TX_Buf.Write_Pos = 0;
//		USART.TX_Buf.Read_Pos = 0;
//	}
//	
//	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0D;
//	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0A;
//	j=1;
//		while( j++ <= OW_Device.Count )
//		{
//			USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 'T';
//			USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x30 +(j-1);
//			USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';
//			USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = '=';
//			USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';			
//			DS_Temp_to_Text( &USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos ], DS_Get_Temp(j-2) ); USART.TX_Buf.Write_Pos += 8;
//			//USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';			
//			//USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 'C';			
//			//USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';			
//			//USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';			
//			USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = ' ';			
//		}
//	
//	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0D;
//	USART.TX_Buf.Text[ USART.TX_Buf.Write_Pos++ ] = 0x0A;		
//		
//	USART_Send( &USART.TX_Buf.Text[0], USART.TX_Buf.Read_Pos, MSG_Len );
//	USART.TX_Buf.Read_Pos += MSG_Len;
//}

//void	USART_Command_NextWritePos(void)
//{
//// 	if(++USART_Command.WritePos == USART_Command_Len) USART_Command.WritePos = 0;
//// 	USART_Command.Number[USART_Command.WritePos]=0;
//// 	USART_Command.Param[USART_Command.WritePos]=0;
//// 	//Увеличение USART_Command.ReadPos если WritePos залезло на ReadPos
//// 	if(USART_Command.ReadPos == USART_Command.WritePos)
//// 		if(++USART_Command.ReadPos == USART_Command_Len) USART_Command.ReadPos = 0;		
//}

void DBG_Mode_Set(char Mode)
{
	DBG_Work_Mode=Mode;
	DBG_Handler_Step=0;	
}

void DBG_Wait_Answer(char Answer)
{
	//Answer_Wait_Pos=Answer_Buf_Pos;
//	Answer_Wait=Answer;
//	Answer_Wait_Count=0;					//кол-во раз ожидания ответа
}

char DBG_Check_Answer(void)
{
char i;
//	i=Answer_Buf_Read_Pos;
//	while(i!=Answer_Buf_Pos)
	{
//		if(Answer_Buf[i]==Answer_Wait) 
		{
//			if(++i==Answer_Buf_Len)i=0;
//			Answer_Buf_Read_Pos=i;			
			return 1;
		}
//		if(++i==Answer_Buf_Len)i=0;
	}
	
	return 0;	//Ответ не оправдал ожидания
}

char DBG_Answer_Wait_Try(void)
{
//	if(++Answer_Wait_Count==Answer_Wait_Count_Max)return 0;	//Попытки закончились
//	else return 1;																					//Попробовать ещё разок подождать
}

void DBG_Send_Command(char Command_Number, char Param_Number)
{
char i=0,j=0,k=0;
	
	//while(j<Command_Number)	if(Command_Str[i++]==0x5F)j++;	//Ищем позицию слова комманды по символу "_"
	
	j=0;
	//USART_Tx_Buf[j++]=0x0d;
	//USART_Tx_Buf[j++]=0x0a;
	//while(Command_Str[i]!=0x5F)	USART_Tx_Buf[j++]=Command_Str[i++];	//Запись комманды	

	i=0;
 	//while(k<Param_Number)	if(Param_Str[i++]==0x5F) k++;	//Ищем позицию слова параметра по символу "_"
	
 	//while(Param_Str[i]!=0x5F)	USART_Tx_Buf[j++]=Param_Str[i++];	//Запись параметра
	//USART_Tx_Buf[j++]=0x0d;
	//USART_Tx_Buf[j++]=0x0a;
	
	//USART_Send(&USART_Tx_Buf[0],0,j);
}

void DBG_Send_Command_SMS(char* Phone_Nmb)
{
char i=0,j=0;//k=0;
	
	//while(j<c_SMS_Send)	if(Command_Str[i++]==0x5F)j++;	//Ищем позицию слова комманды по символу "_"
	
	j=0;
	//USART_Tx_Buf[j++]=0x0d;
	//USART_Tx_Buf[j++]=0x0a;
	//while(Command_Str[i]!=0x5F)	USART_Tx_Buf[j++]=Command_Str[i++];	//Запись комманды	

	i=0;
 	//while(k<Param_Number)	if(Param_Str[i++]==0x5F) k++;	//Ищем позицию слова параметра по символу "_"
	//USART_Tx_Buf[j++]=0x22;
	
 	//while(Phone_Number[i]!=0x5F)	USART_Tx_Buf[j++]=Phone_Number[i++];	//Запись параметра
	//USART_Tx_Buf[j++]=0x22;
	//USART_Tx_Buf[j++]=0x0d;	//Перевод строки
		
	//USART_Send(&USART_Tx_Buf[0],0,j);	
}

void DBG_Send_SMS_Text(void)
{
char i,j=0,k=0;
char Text[7];
		
	//j=0;
	i=1;
	//while(i<=OW_Device_Count)
	{
		//USART_Tx_Buf[j++]='T';
		//USART_Tx_Buf[j++]=0x30+i;
		//USART_Tx_Buf[j++]='=';
		//DS_Text_Temp(Text,i++);
		k=0;
		//while(Text[k]!=0)USART_Tx_Buf[j++]=Text[k++];
		//USART_Tx_Buf[j++]=' ';
		//USART_Tx_Buf[j++]='C';
		//USART_Tx_Buf[j++]=0x0d;	//Перевод строки
	}
	
	//USART_Tx_Buf[j++]=0x1a;	//Конец текста СМС	
	
	//USART_Send(&USART_Tx_Buf[0],0,j);	
}

//Обработчик работы GSM
unsigned int DBG_Handler(void)
{
	//Буфер номеров ответов
// 	LCDPutChar((Answer_Buf[0]+0x30), 96, 1, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[1]+0x30), 96, 1+8, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[2]+0x30), 96, 1+16, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[3]+0x30), 96, 1+24, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[4]+0x30), 96, 1+32, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[5]+0x30), 96, 1+40, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[6]+0x30), 96, 1+48, SMALL, BLACK, WHITE);
// 	LCDPutChar((Answer_Buf[7]+0x30), 96, 1+56, SMALL, BLACK, WHITE);
	
	//LCDPutStr(Phone_Number, 88, 1, SMALL, RED, BLACK);
//	sprintf(DBG_Tx_Buf, "%i", Phone_Number_Type);LCDPutStr(DBG_Tx_Buf, 88, 1+84, SMALL, BLACK, RED);
//	LCDPutChar((Phone_Number_Valid+0x30), 88, 1+124, SMALL, BLACK, RED);
	
	switch(DBG_Work_Mode)
	{
		case m_Off:			//-------------------------------------------------------------
		{
			break;
		}
		
		case m_Prepare:	//-------------------------------------------------------------
		{
			switch(DBG_Handler_Step)
			{
				case 0:
				{
					//SIM_POWER_HI();
					DBG_Handler_Step++;
					return 800;	//800mc
				}
				case 1:
				{
					//SIM_POWER_LO();
					//DBG_Handler_Step++;
					DBG_Handler_Step=4;
					return 1;	//1mc
				}
// 				case 2:
// 				{
// 					DBG_Send_Command(c_AT,p_Null);
// 					DBG_Wait_Answer(a_OK);
// 					DBG_Handler_Step++;					
// 					break;
// 				}
// 				case 3:
// 				{
// 					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
// 					else 
// 						if(DBG_Answer_Wait_Try()){}
// 						else {DBG_Work_Mode=m_Error;}	//Error					
// 					break;
// 				}
				case 4:
				{
//					LCDPutStr("Begin", 88, 1, SMALL, YELLOW, BLACK);
					DBG_Wait_Answer(m_RDY);
//					Answer_Wait_Count=Answer_Wait_Count_Max-100;	//даём 15сек. (100*0,15 сек) на обнаружение команды "RDY"					
					DBG_Handler_Step++;
					break;
				}
				case 5:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Попытки кончились - Error					
					break;
				}		
				case 6:
				{
					DBG_Wait_Answer(m_CFUN);					
					DBG_Handler_Step++;
					break;
				}
				case 7:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Попытки кончились - Error					
					break;
				}
				case 8:
				{
					DBG_Wait_Answer(m_CPIN);					
					DBG_Handler_Step++;
					break;
				}
				case 9:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Попытки кончились - Error					
					break;
				}				
				case 10:
				{
					DBG_Wait_Answer(m_Call_Ready);
//					Answer_Wait_Count=Answer_Wait_Count_Max-75;	//даём 10сек. (75*0,15 сек) на обнаружение команды "Call_Ready"					
					DBG_Handler_Step++;
					break;
				}				
				case 11:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Попытки кончились - Error					
					break;
				}				
				case 12:
				{
					DBG_Send_Command(c_AOH,p_On);
					DBG_Wait_Answer(m_OK);
					DBG_Handler_Step++;					
					break;
				}
				case 13:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Error					
					break;
				}				
				case 14:
				{
					DBG_Send_Command(c_SMS_Format,p_On);
					DBG_Wait_Answer(m_OK);
					DBG_Handler_Step++;					
					break;
				}
				case 15:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Error					
					break;
				}
				case 16:
				{
					DBG_Send_Command(c_SMS_Coding,p_GSM);
					DBG_Wait_Answer(m_OK);
					DBG_Handler_Step++;					
					break;
				}
				case 17:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Error					
					break;
				}
				case 18:
				{
					DBG_Send_Command(c_SMS_Special,p_0);
					DBG_Wait_Answer(m_OK);
					DBG_Handler_Step++;					
					break;
				}
				case 19:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Error					
					break;
				}				
				case 20:
				{
//					LCDPutStr("Ready to work", 88, 1, SMALL, YELLOW, BLACK);
					DBG_Work_Mode=m_Listen;
					DBG_Handler_Step=0;					
					break;
				}	
			}
			break;
		}
		
		case m_Send_SMS:	//-------------------------------------------------------------
		{
			switch(DBG_Handler_Step)
			{
				case 0:
				{
//					LCDPutStr("Send SMS", 88, 1, SMALL, YELLOW, BLACK);
//					DBG_Send_Command_SMS(&Phone_Number[0]);
					DBG_Wait_Answer(m_V);
					DBG_Handler_Step++;
					break;
				}
				case 1:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Попытки кончились - Error					
					break;
				}		
				case 2:
				{
					DBG_Send_SMS_Text();	
					DBG_Wait_Answer(m_OK);
//					Answer_Wait_Count=Answer_Wait_Count_Max-75;	//даём 10сек. (75*0,15 сек) на обнаружение команды "OK"					
					DBG_Handler_Step++;
					break;
				}
				case 3:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else {DBG_Work_Mode=m_Error;}	//Попытки кончились - Error					
					break;
				}
				case 4:
				{
//					LCDPutStr("SMS send", 88, 1, SMALL, YELLOW, BLACK);
					DBG_Work_Mode=m_Listen;
					DBG_Handler_Step=0;					
					break;
				}					
			}
			break;
		}
		
		case m_Listen:	//-------------------------------------------------------------
		{
			switch(DBG_Handler_Step)
			{
				case 0:	//Wait RING
				{
					DBG_Wait_Answer(m_RING);								
					DBG_Handler_Step++;
					break;
				}
				case 1:
				{
					if(DBG_Check_Answer())DBG_Handler_Step=10;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else DBG_Mode_Set(m_Listen);	//Попытки кончились - Начианем заново					
					break;
				}
				
				case 10:	//RING
				{
					DBG_Wait_Answer(m_CLIP);								
					DBG_Handler_Step++;
					break;
				}
				case 11:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else DBG_Mode_Set(m_Listen);	//Попытки кончились - Начианем заново					
					break;
				}
				case 12:
				{
					DBG_Send_Command(c_Tube_Down,p_Null);
					DBG_Wait_Answer(m_OK);
					DBG_Handler_Step++;
					break;
				}
				case 13:
				{
					if(DBG_Check_Answer())DBG_Handler_Step++;	//Ожидаемый ответ получен
					else 
						if(DBG_Answer_Wait_Try()){}
						else DBG_Mode_Set(m_Error);	//Попытки кончились - Error					
					break;
				}	
				case 14:
				{
					DBG_Mode_Set(m_Listen);
					break;
				}
			}
			break;
		}		
		
		case m_Error:		//-------------------------------------------------------------
		{
//			LCDPutStr("Error Wait Answer", 88, 1, SMALL, RED, BLACK);
			DBG_Mode_Set(m_Off);
			break;
		}
		
	}
	
	return Timer_Wait_DBG_Handler;
}
