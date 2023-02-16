#include "command.h"
#include "uart.h"
#include "main.h"

//Список команд (обязательна сортировка)
const uint8_t	Msg_List[]="_?_DEBUG_LED1_LED2_LED3_LED4_LED5_LED6_LUXDATA SHOW_RELAY_RESET_TIME SET_TIME SHOW_VBAT SHOW_VSOLAR SHOW_";
const uint16_t	Msg_List_Len = sizeof Msg_List;

//Список ключей (обязательна сортировка)
const uint8_t	Key_List[]="_0_1_DISABLE_ENABLE_OFF_ON_";
const uint16_t	Key_List_Len = sizeof Key_List;

//Список Hex зачений (обязательна сортировка)
const uint8_t	Hex_List[]="_ _._:_0_1_2_3_4_5_6_7_8_9_A_B_C_D_E_F_";
const uint16_t	Hex_List_Len = sizeof Hex_List;
const uint16_t	Dec_List_Len = 23+4;
const uint8_t NhexChar_spc = 1;
const uint8_t	NhexChar_dot = 3;
const uint8_t	NhexChar_2dot = 5;
const uint8_t	NhexChar_0 = 7;
const uint8_t	NhexList_spc = 1;
const uint8_t	NhexList_dot = 2;
const uint8_t	NhexList_2dot = 3;
const uint8_t	NhexList_0 = 4;

//Список ответов (обязательна сортировка)
const uint8_t	Answer_List[]="_DISABLE_ENABLE_ERROR_OFF_OK_ON_";
const uint16_t	Answer_List_Len = sizeof Answer_List;

const uint8_t	Msg_Spacer=0x5F;	//"_" - Символ разделитель слов-команд для базы слов-команд
const uint8_t	chr_0A = 0x0A;
const uint8_t	chr_0D = 0x0D;
const uint8_t	chr_m = 0x6D;
const uint8_t	chr_L = 0x4C;
const uint8_t	chr_R = 0x52;
const uint8_t	chr_C = 0x43;

volatile Command_struct Command;	//Команда с параметрами

unsigned int	Comm_Task;				  //Набор битов-флагов исполнения задачь
ManualLed_struct ManualLedSw;
ManualTime_struct ManualTime;

//Запись в структуру Msg_Cmd ОДНОГО из значений==========================================
uint8_t Command_Write(uint8_t Cmd, uint8_t Key, uint16_t Value)
{
	//Запись значений----------------------------
	if(Cmd) 
	{	Command.Number=Cmd;
		Command.Key=Key;			//Фактически стирание
		Command.Value=Value;	//Фактически стирание
	}
	if(Key)		Command.Key=Key;
	if(Value)	Command.Value=Value;//-------------
	
	switch (Command.Number)		//В зависимости от принятой команды, ключа, величины определяем что делать дальше
	{
		case m_Q:
		case m_RESET:
		{ Command.Key=1;
		break;}
		
		case m_RELAY:
		{ if(Cmd) return p_Key;		//Если был передан только № сообщения, ищем ключь
			if(Value) return p_Msg;
		break;}
		
		case m_LED1:
		case m_LED2:
		case m_LED3:
		case m_LED4:
		case m_LED5:
		case m_LED6:
		{ 
			static char step=0;
			
			if(Cmd) step=0;
			switch(step)
			{
				case 0: step++; return p_HexValue; break;
				case 1:
				{	//Найден " " из списка HexValue
					if(Value == NhexList_spc) { step++; ManualLedSw.Value = 0; return p_Value; }	
					else return p_Msg;
					break;
				}
				case 2:
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) )) 
					{
						ManualLedSw.Value = ManualLedSw.Value * 10;
						ManualLedSw.Value += Value - NhexList_0; 
						return p_Value;
					}
					else return p_Msg;
					break;
				}
			}
		break;}
		
		case m_TIME_SHOW:
		case m_LUXDATA_SHOW:
		{	//Запуск команды произойдет по окончанию сообщения (0x0D 0x0A)
		break;}
				
		//Для сообщений содержащих ключь
		case m_DEBUG:
		case m_VBAT_SHOW:
		case m_VSOLAR_SHOW:
		{	if(Cmd) return p_Key;	//Если был передан только № сообщения, ищем ключь
		break;}			

		case m_TIME_SET:						//Прием даты и времени в формате 2023.12.11 10:32:57
		{
			static char step=0;
			
			if(Cmd) step=0;
			switch(step)
			{
				case 0: step++; return p_HexValue; break;
				case 1:
				{	//Найден " " из списка HexValue
					if(Value == NhexList_spc) { step++; ManualTime.Year = 0; return p_Value; }	
					else return p_Msg;
				break;}
				case 2:	//Ищем Год
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) )) 
					{ ManualTime.Year = ManualTime.Year * 10;
						ManualTime.Year += Value - NhexList_0; 
						return p_Value;
					}
					else //Найден "." из списка HexValue
						if( Value == NhexList_dot ) { step++; ManualTime.Month = 0; return p_Value; }
						else return p_Msg;
				break;}
				case 3:	//Ищем Месяц
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) )) 
					{ ManualTime.Month = ManualTime.Month * 10;
						ManualTime.Month += Value - NhexList_0; 
						return p_Value;
					}
					else //Найден "." из списка HexValue
						if( Value == NhexList_dot ) { step++; ManualTime.Day = 0; return p_Value; }
						else return p_Msg;
				break;}
				case 4:	//Ищем День
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) ))  
					{ ManualTime.Day = ManualTime.Day * 10;
						ManualTime.Day += Value - NhexList_0; 
						return p_Value;
					}
					else //Найден " " из списка HexValue
						if(Value == NhexList_spc) { step++; ManualTime.Hour = 0; return p_Value; }
						else return p_Msg;
				break;}
				case 5:	//Ищем Час
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) )) 
					{ ManualTime.Hour = ManualTime.Hour * 10;
						ManualTime.Hour += Value - NhexList_0; 
						return p_Value;
					}
					else //Найден ":" из списка HexValue
						if( Value == NhexList_2dot ) { step++; ManualTime.Minute = 0; return p_Value; }
						else return p_Msg;
				break;}	
				case 6:	//Ищем Минуты
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) )) 
					{ ManualTime.Minute = ManualTime.Minute * 10;
						ManualTime.Minute += Value - NhexList_0; 
						return p_Value;
					}
					else //Найден ":" из списка HexValue
						if( Value == NhexList_2dot ) { step++; ManualTime.Second = 0; return p_Value; }
						else return p_Msg;
				break;}			
				case 7:	//Ищем Секунды
				{ //Найдены "0-9" из списка HexValue
					if(( Value >= NhexList_0 ) && ( Value < (NhexList_0+10) ))  
					{ ManualTime.Second = ManualTime.Second * 10;
						ManualTime.Second += Value - NhexList_0; 
						return p_Value;
					}
					else return p_Msg;
				break;}					
			}			
		break;}

		default: { break; }
	}	
	
	return p_Msg;
}//======================================================================================

//=======================================================================================
//= Непосредсвенное выполнение команд																										=
//=======================================================================================
void Command_Exec(void)
{	
	if( Command.Key )	//Если есть ключ команды (!=0) начинаем её обработку
	{
		switch( Command.Number )
		{
			case m_Q:	//Команда подсказки по именам команд--------------------------
			{	for(uint16_t i=0; i<Msg_List_Len; i++)
					if(Msg_List[i] == Msg_Spacer) UART_Send_Chr( &chr_0D );	//Перенос строки												
					else UART_Send_Chr( &Msg_List[i] );				
			break;}//---------------------------------------------------------------
			
			case m_RESET:	//Команда сброса контроллера------------------------------
			{	HAL_NVIC_SystemReset();	break; }
			
			case m_RELAY:	//Команда включения / выключения реле питания-------------
			{ if((Command.Key==k_0)&&(Command.Key==k_OFF))	{ Relay_OFF; Send_Answer_from_List(m_RELAY, a_OFF); }
				else 																					{ Relay_ON;	 Send_Answer_from_List(m_RELAY, a_ON);	}
			break;}//---------------------------------------------------------------			
			
			case m_DEBUG:	//Команда вывода переменных для Debug
			{	switch( Command.Key )
				{	case k_ON:
					case k_ENABLE:
					{	Comm_Task |= t_Debug;												//Включаем режим вывода Debug info
						Send_Answer_from_List(m_DEBUG, a_ON);				//Формирование ответа
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ Comm_Task &= (uint32_t) ~t_Debug;						//Выключаем режим вывода Debug info
						Send_Answer_from_List(m_DEBUG, a_OFF);			//Формирование ответа
					break;}
				}											
				break;
			}
			
			case m_TIME_SHOW:	//Команда вывода времени
			{ Comm_Task |= t_Time_Show;													//Включаем разовый показ времени
				break;
			}
			
			case m_TIME_SET:	//Команда установки даты и времени YYYY.MM.DD hh:mm:ss
			{ Comm_Task |= t_Time_Set;													//Инициализируем установку даты и времени
				break;
			}
			
			case m_VBAT_SHOW:	//Команда включения / выключения вывода Vbat ----
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{	Comm_Task |= t_Vbat_Show;												//Включаем режим вывода Vbat
						Send_Answer_from_List(m_VBAT_SHOW, a_ON);				//Формирование ответа
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ Comm_Task &= (uint32_t) ~t_Vbat_Show;						//Выключаем режим вывода Vbat
						Send_Answer_from_List(m_VBAT_SHOW, a_OFF);			//Формирование ответа
					break;}
				}
			break;}
			
			case m_VSOLAR_SHOW:	//Команда включения / выключения вывода Vsolar ----
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{	Comm_Task |= t_Vsolar_Show;												//Включаем режим вывода Vbat
						Send_Answer_from_List(m_VSOLAR_SHOW, a_ON);				//Формирование ответа
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ Comm_Task &= (uint32_t) ~t_Vsolar_Show;						//Выключаем режим вывода Vbat
						Send_Answer_from_List(m_VSOLAR_SHOW, a_OFF);			//Формирование ответа
					break;}
				}
			break;}		

			case m_LUXDATA_SHOW:	//Команда вывода накопленного массива данных
			{ Comm_Task |= t_LuxData_Show;													//Включаем разовый вывода накопленного массива данных освещенности
				break;
			}			
			
			case m_LED1:							
			{ if( ManualLedSw.Value <= 1000 ) 
					{ ManualLedSw.Led_Nbr = 1; 
						Send_Answer_from_List(m_LED1, a_OK); }	//Формирование ответа
				else Send_Answer_from_List(m_LED1, a_ERROR);	
			break; }
			
			case m_LED2:							
			{ if( ManualLedSw.Value <= 1000 ) 
					{ ManualLedSw.Led_Nbr = 2; 
						Send_Answer_from_List(m_LED2, a_OK); }	//Формирование ответа
				else Send_Answer_from_List(m_LED2, a_ERROR);	
			break; }
			
			case m_LED3:							
			{ if( ManualLedSw.Value <= 1000 ) 
					{ ManualLedSw.Led_Nbr = 3; 
						Send_Answer_from_List(m_LED3, a_OK); }	//Формирование ответа
				else Send_Answer_from_List(m_LED3, a_ERROR);	
			break; }
			
			case m_LED4:							
			{ if( ManualLedSw.Value <= 1000 ) 
					{ ManualLedSw.Led_Nbr = 4; 
						Send_Answer_from_List(m_LED4, a_OK); }	//Формирование ответа
				else Send_Answer_from_List(m_LED4, a_ERROR);	
			break; }
			
			case m_LED5:							
			{ if( ManualLedSw.Value <= 1000 ) 
					{ ManualLedSw.Led_Nbr = 5; 
						Send_Answer_from_List(m_LED5, a_OK); }	//Формирование ответа
				else Send_Answer_from_List(m_LED5, a_ERROR);	
			break; }
			
			case m_LED6:							
			{ if( ManualLedSw.Value <= 1000 ) 
					{ ManualLedSw.Led_Nbr = 6; 
						Send_Answer_from_List(m_LED6, a_OK); }	//Формирование ответа
				else Send_Answer_from_List(m_LED6, a_ERROR);	
			break; }
		}
		
		if( Command.Key == k_END_OF_MSG ) Command.Number = 0;
		Command.Key = 0;	//Очистка команды
	}
}

//Функция возврата указателей *Str на начало строки (номер элемента N) списка List
volatile uint8_t const *get_StrFromList( uint8_t const* List, char N )
{	char i = 0;	
	while ( i < N ) //Ищем начало элемента
		if ( *(List++) == Msg_Spacer ) i++;	
	return List;     //Даем указателю адрес начала элемента
}

//Функция поиска длины строки элемента списка
uint8_t get_LenListStr( volatile uint8_t const* Str )
{	char i = 0;
 	while ( *Str != Msg_Spacer ) { Str++; i++; }
  	return i;
}

//Отправка в UART 5х-значного числа uint16_t (Возврщает 1 при переполнении буфера)
uint8_t Send_uint16(uint16_t Digit)
{	uint8_t d0 = (uint16_t) Digit/10000; Digit -= d0*10000;
	uint8_t d1 = (uint16_t) Digit/1000;	Digit -= d1*1000;
	uint8_t d2 = (uint16_t) Digit/100;	Digit -= d2*100;
	uint8_t d3 = (uint16_t) Digit/10;	Digit -= d3*10;
	
	if(d0)							{	if( UART_Send_Chr(&Hex_List[NhexChar_0+2*d0]) )return 1;	}			//1-й разряд
	if(d0||d1) 					{	if( UART_Send_Chr(&Hex_List[NhexChar_0+2*d1]) )return 1;	}			//2-й
	if(d0||d1||d2) 			{	if( UART_Send_Chr(&Hex_List[NhexChar_0+2*d2]) )return 1;	}			//3-й
	if(d0||d1||d2||d3) 	{	if( UART_Send_Chr(&Hex_List[NhexChar_0+2*d3]) )return 1;	}			//4-й
												if( UART_Send_Chr(&Hex_List[NhexChar_0+2*Digit]) )return 1;		//5-й последний
	return 0;
}

//Отправка в UART 5х-значного числа uint16_t (Возврщает 1 при переполнении буфера)
uint8_t Send_BitsByte(uint8_t Digit)
{	uint8_t d = 0b10000000;
	while ( d )
	{ if( Digit & d ) { if( UART_Send_Chr(&Hex_List[ NhexChar_0+2 ]) )return 1;	}		//1
		else						{ if( UART_Send_Chr(&Hex_List[ NhexChar_0 ]) )return 1;	}			//0
		d >>= 1;
	}
	return 0;
}

void Send_Answer_from_List(uint8_t Msg, uint8_t Key)
{	volatile uint8_t const* S;   
	//Формирование ответа	
	//UART_Send_Chr( &chr_0D );UART_Send_Chr( &chr_0A );	
  S = get_StrFromList( Msg_List, Msg );
	UART_Send_Str( S, get_LenListStr( S ) );												
	UART_Send_Chr( &Hex_List[1] );//Передача указателя на " "
	S = get_StrFromList( Answer_List, Key ); 
	UART_Send_Str( S, get_LenListStr( S ) );												
	UART_Send_Chr( &chr_0D );UART_Send_Chr( &chr_0A );
}

void Vbat_Show(uint16_t V)
{	volatile uint8_t const* S; 	
	//Формирование ответа
	//UART_Send_Chr( &chr_0D );UART_Send_Chr( &chr_0A );	
	S = get_StrFromList( Msg_List, m_VBAT_SHOW );
	UART_Send_Str( S, 5 );
	Send_uint16(V);
	UART_Send_Chr( &chr_0D );UART_Send_Chr( &chr_0A );	
}

void Vsolar_Show(uint16_t V)
{ volatile uint8_t const* S; 	
	//Формирование ответа
	//UART_Send_Chr( &chr_0D );UART_Send_Chr( &chr_0A );	
	S = get_StrFromList( Msg_List, m_VSOLAR_SHOW );
	UART_Send_Str( S, 7 );
	Send_uint16(V);
	UART_Send_Chr( &chr_0D );UART_Send_Chr( &chr_0A );
}

void Time_Show(int16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second)
{
	//Формирование ответа в формате YYYY.MM.DD HH:MM:SS
	//UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
	
	Send_uint16(Year);
	UART_Send_Chr(&Hex_List[NhexChar_dot]);									//.
	if( Month < 10 ) UART_Send_Chr(&Hex_List[NhexChar_0]); 	//Ноль
	Send_uint16(Month);
	UART_Send_Chr(&Hex_List[NhexChar_dot]);									//.
	if( Day < 10 ) UART_Send_Chr(&Hex_List[NhexChar_0]); 		//Ноль
	Send_uint16(Day);
	UART_Send_Chr(&Hex_List[NhexChar_spc]);									//" "
	if( Hour < 10 ) UART_Send_Chr(&Hex_List[NhexChar_0]); 		//Ноль
	Send_uint16(Hour);
	UART_Send_Chr(&Hex_List[NhexChar_2dot]);									//:
	if( Minute < 10 ) UART_Send_Chr(&Hex_List[NhexChar_0]); 	//Ноль
	Send_uint16(Minute);
	UART_Send_Chr(&Hex_List[NhexChar_2dot]);									//:
	if( Second < 10 ) UART_Send_Chr(&Hex_List[NhexChar_0]); 	//Ноль
	Send_uint16(Second);
	
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
}

uint8_t LuxData_Show(uint16_t* Lux, uint16_t len, uint16_t pos)
{ static uint16_t i = UINT16_MAX;				//Текущий индекс в массиве для отправки
	//Запуск индекса
	if( i == UINT16_MAX ) i = pos;
	
	if( Uart.TX_Busy ) return 1;
	
	while(( Lux[i] > 0 )&&( i != UINT16_MAX ))
	{
		if( Send_uint16( Lux[i] ) == 1 ) return 1;
		if( UART_Send_Chr(&Hex_List[NhexChar_spc]) == 1 )return 1;
		if( i == 0 ) i = len-1;
		if( i == pos ) i = UINT16_MAX;
	}
	//Реинициализация индекса, финализация вывода
	i = UINT16_MAX; return 0;
}

void Debug_Show(uint8_t Mode, uint8_t MSensL, uint8_t MSensR, uint8_t Consumers)
{
	UART_Send_Chr(&chr_m); 								//m
	Send_uint16( Mode );
	UART_Send_Chr(&Hex_List[NhexChar_spc]);	//" "
	UART_Send_Chr(&chr_L); 								//L
	Send_uint16( MSensL );
	UART_Send_Chr(&chr_R); 								//R
	Send_uint16( MSensR );
	UART_Send_Chr(&Hex_List[NhexChar_spc]);	//" "
	UART_Send_Chr(&chr_C); 								//L
	Send_BitsByte( Consumers );
	
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);	
}