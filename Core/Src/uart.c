#include "uart.h"
#include "command.h"

//Private function-----------------------------------------------------------
uint8_t SearchChr_in_List(uint8_t *Chr, const uint8_t *List, const uint16_t *List_Len, char *List_Msg_Number, char *ListPos, char *Found_Crh_Count);

Uart_struct	Uart;

//Переменные поиска ответов из списка
char Answer_Str_Pos;

//Запуск приема UART с прерыванием по окончанию приёма
void UART_Rx_Start(UART_HandleTypeDef *huart)
{ HAL_UART_Receive_IT ( huart, &Uart.RX_Buf.Text[Uart.RX_Buf.Write_Pos], 1); // запуск приема UART
}

//Обработка окончания приёма UART (Включаем прием в следующую ячейку буфера)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{ if( ++Uart.RX_Buf.Write_Pos == RX_Buf_Len ) Uart.RX_Buf.Write_Pos = 0;
	HAL_UART_Receive_IT ( huart, &Uart.RX_Buf.Text[Uart.RX_Buf.Write_Pos], 1); // запуск приема UART
}

//Обработка окончания отправки данных в Uart через прерывания или DMA
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{// завершена передача всех данных
   Uart.TX_Busy = 0;
}

//Отправка кольцевого буфера в Uart
void UART_Tx_Handler(UART_HandleTypeDef *huart)
{
	if(( Uart.TX_Buf.Read_Pos != Uart.TX_Buf.Write_Pos ) && ( Uart.TX_Busy == 0 ))
	{
		if( Uart.TX_Buf.Read_Pos < Uart.TX_Buf.Write_Pos )
		{
			//HAL_UART_Transmit_DMA( huart, (uint8_t*) &Uart.TX_Buf.Text[Uart.TX_Buf.Read_Pos], (Uart.TX_Buf.Write_Pos - Uart.TX_Buf.Read_Pos) );	
			HAL_UART_Transmit_IT( huart, (uint8_t*) &Uart.TX_Buf.Text[Uart.TX_Buf.Read_Pos], (Uart.TX_Buf.Write_Pos - Uart.TX_Buf.Read_Pos) );	
			Uart.TX_Buf.Read_Pos = Uart.TX_Buf.Write_Pos;
		}
		else
		{
			//HAL_UART_Transmit_DMA( huart, (uint8_t*) &Uart.TX_Buf.Text[Uart.TX_Buf.Read_Pos], (Uart.TX_Buf.Len - Uart.TX_Buf.Read_Pos) );	
			HAL_UART_Transmit_IT( huart, (uint8_t*) &Uart.TX_Buf.Text[Uart.TX_Buf.Read_Pos], (TX_Buf_Len - Uart.TX_Buf.Read_Pos) );	
			Uart.TX_Buf.Read_Pos = 0;
		}
		Uart.TX_Busy = 1;
	}
}

//Проверяет и возвращает количество доступных для записи ячеек буфера отправки
uint8_t UART_TXbuf_Space_Check()
{	if( Uart.TX_Buf.Write_Pos == Uart.TX_Buf.Read_Pos) return TX_Buf_Len;
	if( Uart.TX_Buf.Write_Pos < Uart.TX_Buf.Read_Pos) return ( Uart.TX_Buf.Read_Pos - Uart.TX_Buf.Write_Pos );
	if( Uart.TX_Buf.Write_Pos > Uart.TX_Buf.Read_Pos) return ( TX_Buf_Len - Uart.TX_Buf.Write_Pos + Uart.TX_Buf.Read_Pos );
return 0;}

//Отправка одного символа в буфер передачи UART (Возврщает 1 при переполнении буфера)
uint8_t UART_Send_Chr(const uint8_t *Chr)
{ Uart.TX_Buf.Text[Uart.TX_Buf.Write_Pos++] = *Chr;
	if( Uart.TX_Buf.Write_Pos == Uart.TX_Buf.Read_Pos ) { Uart.TX_Buf.Write_Pos--; return 1; }	//Возвращаем флаг переполнения буфера
	if( Uart.TX_Buf.Write_Pos == TX_Buf_Len ) Uart.TX_Buf.Write_Pos = 0;
	return 0;																																		//Возвращаем флаг нормального завершения
}

//Отправка строки из символов, длинною в Size в буфер передачи UART (Возврщает 1 при переполнении буфера)
uint8_t	UART_Send_Str(uint8_t const *Str, uint8_t Size)
{	for(uint8_t i=1; i <= Size; i++ )
		if(UART_Send_Chr(Str++)==1)return 1;	//Возвращаем флаг переполнения буфера
	return 0;
}

// Обработчик принятых символов _.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.___.-=-.__
void UART_Rx_Handler(void)
{ static Find_type Detect_Msg = Find_None;
	static char Search_Mode;
	
	static char Answer_Chr_Count;
	static char Msg_Number;			//Возможно нужен вынос за процедуру
	static char Msg_List_Pos;		//Возможно нужен вынос за процедуру
	
	static char Search_Result;
	
	//Проверям есть ли новый символ в буфере
	if( Uart.RX_Buf.Write_Pos !=  Uart.RX_Buf.Read_Pos )
	{	
		//Проверяем какой символ приняли
		switch( Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ] )
		{	
			case 0x0D:
			{
				if( Detect_Msg == Find_0D0A_Msg )	Detect_Msg = Find_Msg_0D;
				else 															Detect_Msg = Find_0D;
				break;
			}
			case 0x0A:
			{
				if( Detect_Msg == Find_0D )			Detect_Msg = Find_0D0A;	//Ничего не делаем
				if( Detect_Msg == Find_Msg_0D ) { Detect_Msg = Find_0D0A; Command_Write( 0, k_END_OF_MSG, 0 ); } //Найден конец сообщения
				break;
			}
			default:
			{
				if( Detect_Msg == Find_0D0A )	
				{	Detect_Msg = Find_0D0A_Msg;	//Найдено начало сообщения
					
					//Инициализация переменных задействованных в поиске команд
					Msg_List_Pos = 0;
					Msg_Number = 0;	//Номер команды из списка Msg_List, временный
					Search_Mode = 0;//Режим поиска (0 - определение команды, 1-255 - определение параметров команды)
				}
				if(( Detect_Msg == Find_Msg_0D ) || ( Detect_Msg == Find_0D )) Detect_Msg=Find_None;					
			
				//=====================================Парсинг===================================				
				if( Detect_Msg == Find_0D0A_Msg )
				switch(Search_Mode)
				{
					case p_Msg: //Поиск ответов по списку Msg_List---------------------------------
					{										
						Search_Result = SearchChr_in_List( &Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ], &Msg_List[0], &Msg_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						
						//Если найдено полное совпадение слова+++++++++++++++++++++++++
						if(Search_Result == r_Srch_Complete)
						{
							//Заносим номер сообщения в буфер и меняем режим поиска в зависимости от принятой команды
							Search_Mode = Command_Write( Msg_Number,0,0 );
							Msg_Number=0;
							Msg_List_Pos=0;	//Обнуляем номер найденого сообщения
							Answer_Chr_Count=0;
						}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						break;
					}//------------------------------------------------------------------------------
					
					case p_Key:	//Поиск ключа по списку Key_List-------------------------------------
					{
						if( Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ] == 32 )	break;	//Делаем пропуск пробелов
						
						Search_Result = SearchChr_in_List( &Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ], &Key_List[0], &Key_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						
						//Если найдено полное совпадение слова+++++++++++++++++++++++++
						if(Search_Result == r_Srch_Complete)
						{
							Search_Mode = Command_Write( 0, Msg_Number, 0 );
						}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						
					break;}	//-----------------------------------------------------------------------
				
					case p_Value:	//Поиск ключа по списку Hex_List-----------------------------------
					{
						Search_Result = SearchChr_in_List( &Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ], &Hex_List[0], &Dec_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						if((Search_Result == r_Srch_ChrFound) || (Search_Result == r_Srch_Complete))	//Нашли символ из списка Hex_List
						{
							Search_Mode = Command_Write( 0, 0, Msg_Number);
							
							Msg_Number=0;
							Msg_List_Pos=0;	//Обнуляем номер найденого сообщения
							Answer_Chr_Count=0;
						}
						//else Search_Mode = Command_Write( 0, 0, UINT16_MAX); //Если символ не найден отправляем 0
					break;}	//-----------------------------------------------------------------------		
					
					case p_HexValue:	//Поиск ключа по списку Hex_List-------------------------------
					{
						Search_Result = SearchChr_in_List( &Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ], &Hex_List[0], &Hex_List_Len, &Msg_Number, &Msg_List_Pos, &Answer_Chr_Count);
						if((Search_Result == r_Srch_ChrFound) || (Search_Result == r_Srch_Complete))	//Нашли символ из списка Hex_List
						{
							Search_Mode = Command_Write( 0, 0, Msg_Number);
							
							Msg_Number=0;
							Msg_List_Pos=0;	//Обнуляем номер найденого сообщения
							Answer_Chr_Count=0;
						}
					break;}	//-----------------------------------------------------------------------						

					case p_RawValue:	//Считывание сырых данных -------------------------------------
					{ Search_Mode = Command_Write( 0, 0, Uart.RX_Buf.Text[ Uart.RX_Buf.Read_Pos ]);						
					break;}	//-----------------------------------------------------------------------									

				}
				//===========================================================================				
				break;
			}
		}
		if( ++Uart.RX_Buf.Read_Pos == RX_Buf_Len ) Uart.RX_Buf.Read_Pos = 0;	//Переходим к следующему принятому символу
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
uint8_t SearchChr_in_List(uint8_t *Chr, const uint8_t *List, const uint16_t *List_Len, char *List_Msg_Number, char *ListPos, char *Found_Crh_Count)
{
	unsigned int i,k;
	char	Tmp;

Begin:	
	//Поиск совпадения с Chr первого символа из слова списка сообщений List---------------------------------
	if( *List_Msg_Number == 0 )	//Если ещё не было начато определние сообщения, приступаем
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

