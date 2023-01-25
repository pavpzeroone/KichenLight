#include "command.h"
#include "uart.h"
#include "main.h"
//#include "gpio.h"
//#include "modulation.h"
//#include "CAN.h"
//#include "BEAN.h"

//Список команд
//const uint8_t	Msg_List[]="_#_?_1WIRE DETECT_1WIRE SHOW COUNT_1WIRE SHOW ID_1WIRE WORK_BEAN SEND_BEAN SHOW_BEAN TEST SIGNAL_CAN FILTER SET_CAN SEND_CAN SHOW_CAN TEST SIGNAL_CONNECT_DS18B20 REQUEST_DS18B20 SHOW TEMP_HEATER1_HEATER2_HEATER3_HEATER4_LC DRL DEMO_LC DRL LED_LCD TEMP SHOW_RELAY_RESET_";
const uint8_t	Msg_List[]="_?_RELAY_LED1_LED2_LED3_LED4_LED5_LED6_VBAT SHOW_VSOLAR SHOW_TIME SHOW_TIME SET_LUXDATA SHOW_";
const uint16_t	Msg_List_Len = sizeof Msg_List;

//Список ключей
const uint8_t	Key_List[]="_0_1_DISABLE_ENABLE_OFF_ON_";
const uint16_t	Key_List_Len = sizeof Key_List;

//Список Hex зачений
const uint8_t	Hex_List[]="_ _0_1_2_3_4_5_6_7_8_9_A_B_C_D_E_F_._:_";
const uint16_t	Hex_List_Len = sizeof Hex_List;

//Список ответов
const uint8_t	Answer_List[]="_DISABLE_ENABLE_ERROR_OFF_OK_ON_";
const uint16_t	Answer_List_Len = sizeof Answer_List;

const uint8_t		Msg_Spacer=0x5F;	//"_" - Символ разделитель слов-команд для базы слов-команд

const uint8_t	chr_0A = 0x0A;
const uint8_t	chr_0D = 0x0D;

volatile Command_struct Command;	//Команда с параметрами

unsigned int	Comm_Task;				  //Набор битов-флагов исполнения задачь

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

//Запись в структуру Msg_Cmd ОДНОГО из значений==========================================
char Command_Write(char Number, char Key, int Value)
{
// 	char Num_Key_Val;
// 	#define Num 0
// 	#define Key 1
// 	#define Val 2
	
	//Запись значений----------------------------
	if(Number) 
	{	
		Command.Number=Number;
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
		
		case m_HEATER1:			
		case m_HEATER2:
		case m_HEATER3:
		case m_HEATER4:
		case m_RELAY:
		{
			if(Number) return p_Key;	//Если был передан только № сообщения, ищем ключь
// 			if(Key)
// 			{
// 				switch(Key)
// 				{
// 					case k_0:
// 					{
// 						//Relay_OFF();
// 					break;}
// 					
// 					case k_1:
// 					{
// 						//Relay_ON();
// 					break;}					
// 				}
// 			}
			if(Value) return p_Msg;
		break;}
		
		case m_TIME_SHOW:
		case m_LUXDATA_SHOW:
		case m_DS18B20_SHOW_TEMP:
		{
			Command.Key = 1;
		break;}
				
		//Для сообщений содержащих ключь
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
		{	if(Number) return p_Key;	//Если был передан только № сообщения, ищем ключь
		break;}			

		case m_BEAN_SEND:
		{
			static char step=0, spacer=0;
			
			if(Number) { step=0; spacer=0; return p_HexValue;}
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
			
			if(Number) { step=0; spacer=0; return p_HexValue;}
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
			{	for(unsigned int i=0; i<Msg_List_Len; i++)
					if(Msg_List[i] == Msg_Spacer) UART_Send_Chr( &chr_0D );	//Перенос строки												
					else UART_Send_Chr( &Msg_List[i] );				
			break;}//---------------------------------------------------------------
			
			case m_HEATER1:	//Команда включения / выключения нагревателя 1----------
			{
//				if((Command.Key==k_0)&&(Command.Key==k_OFF))	Heater_1_OFF();
//				else 																					Heater_1_ON();
			break;}//---------------------------------------------------------------
			
			case m_HEATER2:	//Команда включения / выключения нагревателя 2----------
			{
//				if((Command.Key==k_0)&&(Command.Key==k_OFF))	Heater_2_OFF();
//				else 																					Heater_2_ON();
			break;}//---------------------------------------------------------------

			case m_HEATER3:	//Команда включения / выключения нагревателя 3----------
			{
//				if((Command.Key==k_0)&&(Command.Key==k_OFF))	Heater_3_OFF();
//				else 																					Heater_3_ON();
			break;}//---------------------------------------------------------------

			case m_HEATER4:	//Команда включения / выключения нагревателя 4----------
			{
//				if((Command.Key==k_0)&&(Command.Key==k_OFF))	Heater_4_OFF();
//				else 																					Heater_4_ON();
			break;}//---------------------------------------------------------------	

			case m_RELAY:	//Команда включения / выключения реле питания-------------
			{
				if((Command.Key==k_0)&&(Command.Key==k_OFF))	{ Relay_OFF; Send_Ansver_from_List(m_RELAY, a_OFF); }
				else 																					{ Relay_ON;	 Send_Ansver_from_List(m_RELAY, a_ON);	}
			break;}//---------------------------------------------------------------			
			
			case m_TIME_SHOW:	//Команда вывода времени
			{
				Comm_Task |= t_Time_Show;													//Включаем разовый показ времени
				break;
			}
			
			case m_VBAT_SHOW:	//Команда включения / выключения вывода Vbat ----
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{	Comm_Task |= t_Vbat_Show;												//Включаем режим вывода Vbat
						Send_Ansver_from_List(m_VBAT_SHOW, a_ON);				//Формирование ответа
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ Comm_Task &= (uint32_t) ~t_Vbat_Show;						//Выключаем режим вывода Vbat
						Send_Ansver_from_List(m_VBAT_SHOW, a_OFF);			//Формирование ответа
					break;}
				}
			break;}
			
			case m_VSOLAR_SHOW:	//Команда включения / выключения вывода Vsolar ----
			{ switch( Command.Key )
				{
					case k_ON:
					case k_ENABLE:
					{	Comm_Task |= t_Vsolar_Show;												//Включаем режим вывода Vbat
						Send_Ansver_from_List(m_VSOLAR_SHOW, a_ON);				//Формирование ответа
					break;}
					
					case k_OFF:
					case k_DISABLE:
					{ Comm_Task &= (uint32_t) ~t_Vsolar_Show;						//Выключаем режим вывода Vbat
						Send_Ansver_from_List(m_VSOLAR_SHOW, a_OFF);			//Формирование ответа
					break;}
				}
			break;}		

			case m_LUXDATA_SHOW:	//Команда вывода накопленного массива данных
			{
				Comm_Task |= t_LuxData_Show;													//Включаем разовый вывода накопленного массива данных освещенности
				break;
			}			
			
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
						
//						USART_Send_Ansver_from_List(m_LC_DRL_LED, a_ON);
					break;}
					case k_OFF:
					{
//						DRL.Preset.Number = 0;
//						DRL.Preset.Step 	= 0;
//						DRL.Preset.Delay 	= 0;
						
//						USART_Send_Ansver_from_List(m_LC_DRL_LED, a_OFF);
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
						
//						USART_Send_Ansver_from_List(m_LC_DRL_DEMO, a_ON);
					break;}
					case k_OFF:
					{
//						DRL.Preset.Number = 0;
//						DRL.Preset.Step 	= 0;
//						DRL.Preset.Delay 	= 0;
						
//						USART_Send_Ansver_from_List(m_LC_DRL_DEMO, a_OFF);
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
		
		Command.Key = 0;	//Очистка команды
	}
}

//Функция возврата указателей *Str, длинною *Len, на начало строки (номер Comm_N) списка Comm_List
void Str_From_List(const uint8_t *Str, uint8_t *Len, const uint8_t *Comm_List, char Comm_N)
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
}

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
	//*Text = ' ';
	//Text++; *Len++;	
}

void Send_Ansver_from_List(char Msg, char Key)
{	const uint8_t* S = 0; uint8_t Len;
	
	//Формирование ответа
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
	Str_From_List(S, &Len, &Msg_List[0], Msg); UART_Send_Str(S, Len);
	UART_Send_Chr(&Hex_List[1]);//Передача указателя на " "
	Str_From_List(S, &Len, &Answer_List[0], Key); UART_Send_Str(S, Len);												
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
}

void Vbat_Show(uint16_t V)
{ const uint8_t* S = 0; uint8_t Len;
	
	//Формирование ответа
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);

	Str_From_List(S, &Len, &Msg_List[0], m_VBAT_SHOW); UART_Send_Str(S, 5);	
	UART_Send_uint16(V);

	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);	
}

void Vsolar_Show(uint16_t V)
{ const uint8_t* S = 0; uint8_t Len;
	
	//Формирование ответа
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
	
	Str_From_List(S, &Len, &Msg_List[0], m_VSOLAR_SHOW); UART_Send_Str(S, 7);
	UART_Send_uint16(V);
	
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);	
}

void Time_Show(int16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second)
{
	//Формирование ответа в формате YYYY.MM.DD HH:MM:SS
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
	
	UART_Send_uint16(Year);
	UART_Send_Chr(&Hex_List[35]);									//.
	if( Month < 10 ) UART_Send_Chr(&Hex_List[3]); //Ноль
	UART_Send_uint16(Month);
	UART_Send_Chr(&Hex_List[35]);									//.
	if( Day < 10 ) UART_Send_Chr(&Hex_List[3]); 	//Ноль
	UART_Send_uint16(Day);
	UART_Send_Chr(&Hex_List[1]);									//" "
	if( Hour < 10 ) UART_Send_Chr(&Hex_List[3]); 	//Ноль
	UART_Send_uint16(Hour);
	UART_Send_Chr(&Hex_List[37]);									//:
	if( Minute < 10 ) UART_Send_Chr(&Hex_List[3]); 	//Ноль
	UART_Send_uint16(Minute);
	UART_Send_Chr(&Hex_List[37]);									//:
	if( Second < 10 ) UART_Send_Chr(&Hex_List[3]); 	//Ноль
	UART_Send_uint16(Second);
	
	UART_Send_Chr(&chr_0D);UART_Send_Chr(&chr_0A);
}

uint8_t LuxData_Show(uint16_t* Lux, uint16_t len, uint16_t pos)
{ static uint16_t i = UINT16_MAX;				//Текущий индекс в массиве для отправки
	//Запуск индекса
	if( i == UINT16_MAX ) i = pos;
	
	if( Uart.TX_Busy ) return 1;
	
	while(( Lux[i] > 0 )&&( i != UINT16_MAX ))
	{
		if( UART_Send_uint16( Lux[i] ) == 1 ) return 1;
		if( UART_Send_Chr(&Hex_List[1]) == 1 )return 1;
		if( i == 0 ) i = len-1;
		if( i == pos ) i = UINT16_MAX;
	}
	//Реинициализация индекса, финализация вывода
	i = UINT16_MAX; return 0;
}