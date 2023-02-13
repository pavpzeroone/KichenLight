#include "command.h"
#include "uart.h"
#include "main.h"
//#include "gpio.h"
//#include "modulation.h"
//#include "CAN.h"
//#include "BEAN.h"

//Список команд
//const uint8_t	Msg_List[]="_#_?_1WIRE DETECT_1WIRE SHOW COUNT_1WIRE SHOW ID_1WIRE WORK_BEAN SEND_BEAN SHOW_BEAN TEST SIGNAL_CAN FILTER SET_CAN SEND_CAN SHOW_CAN TEST SIGNAL_CONNECT_DS18B20 REQUEST_DS18B20 SHOW TEMP_HEATER1_HEATER2_HEATER3_HEATER4_LC DRL DEMO_LC DRL LED_LCD TEMP SHOW_RELAY_RESET_";
const uint8_t	Msg_List[]="_?_DEBUG_LED1_LED2_LED3_LED4_LED5_LED6_LUXDATA SHOW_RELAY_RESET_TIME SET_TIME SHOW_VBAT SHOW_VSOLAR SHOW_";
const uint16_t	Msg_List_Len = sizeof Msg_List;

//Список ключей
const uint8_t	Key_List[]="_0_1_DISABLE_ENABLE_OFF_ON_";
const uint16_t	Key_List_Len = sizeof Key_List;

//Список Hex зачений
//const uint8_t	Hex_List[]="_ _0_1_2_3_4_5_6_7_8_9_A_B_C_D_E_F_._:_";
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

//Список ответов
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

//Переменные используемые в main
char BEAN_Code[15];	//Переменная для отправки кода BEAN

typedef struct
{
	char Number;				//Номер фильтра
	char Ctrl_Reg;			//Содержит 0b00000001 - Scale32b, 0b00000010 - List
	unsigned int ID;
	unsigned int Mask;
} CAN_Filter_struct;
CAN_Filter_struct CAN_Filter_Bank;	//Переменная для установки фильтра

//uint32_t Digit;				//Число собранное из данных UART

//Запись в структуру Msg_Cmd ОДНОГО из значений==========================================
uint8_t Command_Write(uint8_t Cmd, uint8_t Key, uint16_t Value)
{
	//Запись значений----------------------------
	if(Cmd) 
	{	
		Command.Number=Cmd;
		Command.Key=Key;			//Фактически стирание
		Command.Value=Value;	//Фактически стирание
	}
	if(Key)		Command.Key=Key;
	if(Value)	Command.Value=Value;//-------------
	
	switch (Command.Number)		//В зависимости от принятой команды, ключа, величины определяем что делать дальше
	{
		case m_Q:
		{
			Command.Key=1;
		break;}
		
		case m_RELAY:
		{
			if(Cmd) return p_Key;		//Если был передан только № сообщения, ищем ключь
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
		case m_DS18B20_SHOW_TEMP:
			{	//Запуск команды произойдет по окончанию сообщения (0x0D 0x0A)
			//Command.Key = 1;
		break;}
				
		//Для сообщений содержащих ключь
		case m_DEBUG:
		case m_VBAT_SHOW:
		case m_VSOLAR_SHOW:
		case m_1WIRE_WORK:
		case m_BEAN_SHOW:	
		case m_BEAN_TEST_SIGNAL:			
		case m_CAN_SHOW:
		case m_CAN_TEST_SIGNAL:
		case m_DS18B20_REQUEST:
		case m_LCD_TEMP_SHOW:
		case m_LC_DRL_LED:
		case m_LC_DRL_DEMO:
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
		
		case m_BEAN_SEND:
		{
			static char step=0, spacer=0;
			
			if(Cmd) { step=0; spacer=0; return p_HexValue;}
			if(Value)
			{
				switch(Value)
				{
					case 1:		//найден " "
					{
						if(spacer==0) { step++; spacer=1; BEAN_Code[step-1]=0; }
						break;
					}
					case 18:	//найдена "."
					{
						Command.Key=1;	//Разрешаем исполнение команды
						break;
					}
					default:
					{
						spacer = 0;
						BEAN_Code[step-1]<<=4;
						BEAN_Code[step-1]|=Value-2;
						break;
					}
				}
			}	
			return p_HexValue;
			break;
		}
		
		case m_CAN_FILTER_SET:
		{
			static char step=0, spacer=0;
			
			if(Cmd) { step=0; spacer=0; return p_HexValue;}
			if(Value)
			{
				switch(Value)
				{
					case 1:		//найден " "
					{
						if(spacer==0) { step++; spacer=1; }
						break;
					}
					case 18:	//найдена "." - Конец сообщения
					{
						Command.Key=1;	//Разрешаем исполнение команды
						break;
					}
					default:
					{
						spacer = 0;
						switch(step)
						{
							case 1: { CAN_Filter_Bank.Number = Value-2; break; }
							case 2: { CAN_Filter_Bank.Ctrl_Reg = Value-2; ;break; }
							case 3: { CAN_Filter_Bank.ID <<= 4; CAN_Filter_Bank.ID |= (Value-2); break; }
							case 4: { CAN_Filter_Bank.Mask <<= 4; CAN_Filter_Bank.Mask |= (Value-2); break; }
							default: {}
						}
						break;
					}
				}
			}	
			return p_HexValue;
			break;
		}		
		
//		case m_H:
//		{
//			static char step=0;
//			
//			if(Number) { step=0; return p_RawValue; }
//			switch(step)
//			{
//				case 0:	//найден PRI и ML
//				{
//					BEAN_Code[0] = Value >> 4;
//					BEAN_Code[1] = Value & 0x0F;
//					step++;
//					break;
//				}
//				default:
//				{						
//					BEAN_Code[step+1]=Value;
//					if(step++ == BEAN_Code[1]) 
//						Command.Key=1;	//Разрешаем исполнение команды
//					break;
//					}
//			}
//				
//			return p_RawValue;
//		break;}
						
		
		default: {}
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
			
			case m_RELAY:	//Команда включения / выключения реле питания-------------
			{
				if((Command.Key==k_0)&&(Command.Key==k_OFF))	{ Relay_OFF; Send_Answer_from_List(m_RELAY, a_OFF); }
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
			{
				Comm_Task |= t_Time_Show;													//Включаем разовый показ времени
				break;
			}
			
			case m_TIME_SET:	//Команда установки даты и времени YYYY.MM.DD hh:mm:ss
			{
				Comm_Task |= t_Time_Set;													//Инициализируем установку даты и времени
				break;
			}
			
			case m_VBAT_SHOW:	//Команда включения / выключения вывода Vbat ----
			{
				switch( Command.Key )
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
			{
				switch( Command.Key )
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
			{
				Comm_Task |= t_LuxData_Show;													//Включаем разовый вывода накопленного массива данных освещенности
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
			
			case m_CAN_SHOW:	//Команда включения / выключения вывода кодов CAN ----
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{	uint8_t Len = 0;
						Comm_Task |= t_USART_Can_Show;				//Включаем режим вывода CAN
						
						//Формирование ответа
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_CAN_SHOW );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_ENABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ uint8_t Len = 0;
						Comm_Task &= (uint32_t) ~t_USART_Can_Show;	//Выключаем режим вывода CAN
						
						//Формирование ответа
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Msg_List[0], m_CAN_SHOW );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_DISABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );
					break;}
				}
			break;}		

			case m_CAN_TEST_SIGNAL:	
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{ uint8_t Len = 0;
						Comm_Task |= (uint32_t) t_CAN_Test_Signal;	//Включаем режим отправки тестовых сообщений CAN
						
						//Формирование ответа						
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_CAN_TEST_SIGNAL );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_ENABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );					
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{	uint8_t Len = 0;
					Comm_Task &= (uint32_t) ~t_CAN_Test_Signal;	//Выключаем режим отправки тестовых сообщений CAN
						
						//Формирование ответа						
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_CAN_TEST_SIGNAL );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_DISABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );							
					break;}
				}
			break;}			
			
			case m_CAN_FILTER_SET:		//Установка фильтра CAN
			{	uint8_t Len = 0;
				//CAN_Filter_Set(CAN_Filter_Bank.Number, CAN_Filter_Bank.Ctrl_Reg, CAN_Filter_Bank.ID, CAN_Filter_Bank.Mask);
				
				//Формирование ответа
				Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
				Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_OK );
				Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
				
				//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );	
			}
			
//			case m_CAN_SEND:				//Отправка сообщения CAN
//			{	uint8_t Len = 0;
//				//CAN_Filter_Set(CAN_Filter_Bank.Number, CAN_Filter_Bank.Ctrl_Reg, CAN_Filter_Bank.ID, CAN_Filter_Bank.Mask);
//				
//				//Формирование ответа
//				Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
//				Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_OK );
//				Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
//				
//				//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );	
//			}
			
			case m_BEAN_SHOW:	//Команда включения / выключения вывода кодов BEAN ---
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{	uint8_t Len = 0;
						Comm_Task |= (uint32_t) t_BEAN_Test_Signal;				//Включаем режим вывода BEAN
						
						//Формирование ответа
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_BEAN_SHOW );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_ENABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ uint8_t Len = 0;
						Comm_Task &= (uint32_t) ~t_BEAN_Test_Signal;	//Выключаем режим вывода BEAN
						
						//Формирование ответа
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Msg_List[0], m_BEAN_SHOW );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_DISABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );
					break;}
				}
			break;}
			
			case m_BEAN_TEST_SIGNAL:	
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{ uint8_t Len = 0;
//						RTE_Device_Work |= (uint32_t) e_BEAN_Test_Signal;	//Включаем режим вывода BEAN
						
						//Формирование ответа						
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_BEAN_TEST_SIGNAL );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_ENABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );					
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{	uint8_t Len = 0;
//						RTE_Device_Work ^= (uint32_t) e_BEAN_Test_Signal;	//Выключаем режим вывода BEAN
						
						//Формирование ответа						
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_BEAN_TEST_SIGNAL );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_DISABLE );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );							
					break;}
				}
			break;}
			
//			case m_H:
			case m_BEAN_SEND:
			{
//				BEAN_Send(BEAN_Code[0], BEAN_Code[1] , BEAN_Code[2], BEAN_Code[3] , &BEAN_Code[4] , &BEAN_Two_TX_MSG);
			break;}
				
			case m_LC_DRL_LED:	//Управление ДХО
			{ switch( Command.Key )
				{
					case k_ON:
					{
//						DRL.Preset.Number = 1;
//						DRL.Preset.Step 	= 0;
//						DRL.Preset.Delay 	= 0;
						
//						USART_Send_Answer_from_List(m_LC_DRL_LED, a_ON);
					break;}
					case k_OFF:
					{
//						DRL.Preset.Number = 0;
//						DRL.Preset.Step 	= 0;
//						DRL.Preset.Delay 	= 0;
						
//						USART_Send_Answer_from_List(m_LC_DRL_LED, a_OFF);
					break;}					
				}
			break;}
			
			case m_LC_DRL_DEMO:	//Управление демо режимом ДХО 
			{ switch( Command.Key )
				{
					case k_ON:
					{
//						DRL.Preset.Number = 2;
//						DRL.Preset.Step 	= 0;
//						DRL.Preset.Delay 	= 0;
						
//						USART_Send_Answer_from_List(m_LC_DRL_DEMO, a_ON);
					break;}
					case k_OFF:
					{
//						DRL.Preset.Number = 0;
//						DRL.Preset.Step 	= 0;
//						DRL.Preset.Delay 	= 0;
						
//						USART_Send_Answer_from_List(m_LC_DRL_DEMO, a_OFF);
					break;}					
				}
			break;}			
			
					case m_DS18B20_SHOW_TEMP:	//Команда вывода показаний датчиков температуры ----
			{ //USART_DS18B20_Show();
			break;}
			
			case m_DS18B20_REQUEST:		//Команда включения / выключения запроса температуры ---
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE: {
//					{ RTE_Device_Work |= (uint32_t) e_Sens_Temp_Request;	//Включаем режим запроса температуры
					break;}
					
					case k_OFF:
					case k_DISABLE: {
//					{ RTE_Device_Work ^= (uint32_t) e_Sens_Temp_Request;	//Выключаем режим запроса температуры
					break;}
				}
			break;}
			
			case m_1WIRE_WORK:	//Команда включения / выключения вывода температуры на экран ---
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{ uint8_t Len = 0;
//						RTE_Device_Work |= (uint32_t) e_1Wire;		//Включаем 
						
						//Формирование ответа						
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_1WIRE_WORK );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_ON );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );	
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ uint8_t Len = 0;
//						RTE_Device_Work ^= (uint32_t) e_1Wire;		//Выключаем 
						
						//Формирование ответа						
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						Text_From_List( &Uart.TX_Buf.Text[ Len] , &Len, &Msg_List[0], m_1WIRE_WORK );
						Uart.TX_Buf.Text[ Len++ ] = ' ';
						Text_From_List( &Uart.TX_Buf.Text[ Len ] , &Len, &Answer_List[0], a_OFF );
						Uart.TX_Buf.Text[ Len++ ] = 0x0D;	Uart.TX_Buf.Text[ Len++ ] = 0x0A;
						
						//USART_Send( &Uart.TX_Buf.Text[ 0 ], 0, Len );	
					break;}
				}
			break;}					
			
			case m_LCD_TEMP_SHOW:	//Команда включения / выключения вывода температуры на экран ---
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{ //RTE_Device_Work |= (uint32_t) e_LCD_Show_Temp;		//Включаем режим вывода на экран
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ //RTE_Device_Work ^= (uint32_t) e_LCD_Show_Temp;		//Выключаем режим вывода на экран
					break;}
				}
			break;}			
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

//На удаление всвязи с заменой на get_StrFromList и get_LenListStr
//Функция возврата указателей *Str, длинною *Len, на начало строки (номер Comm_N) списка Comm_List
/*void Str_From_List(const uint8_t *Str, uint8_t *Len, const uint8_t *Comm_List, char Comm_N)
{	uint8_t k = 0;	
	while ( k < Comm_N )
	{	
		if ( *Comm_List == Msg_Spacer ) k++;
		Comm_List++;		
	}
	Str = Comm_List;
	*Len = 0;
	while ( k < Comm_N + 1 )	
	{
		*Len = *Len + 1;
		Comm_List++;
		if ( *Comm_List == Msg_Spacer ) k++;
	}	
}*/

void Text_From_List( uint8_t *Text, uint8_t *Len, const uint8_t *Comm_Str, char Comm_N )
{	char k = 0;
	
	while ( k < Comm_N )
	{	
		if ( *Comm_Str == Msg_Spacer ) k++;
		Comm_Str++;		
	}
	while ( k < Comm_N + 1 )	
	{
		*Text = *Comm_Str;
		Text++; *Len = *Len + 1;
		Comm_Str++;
		if ( *Comm_Str == Msg_Spacer ) k++;
	}
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