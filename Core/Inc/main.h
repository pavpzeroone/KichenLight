/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//#define DEBUG_MODE									//Режим Debug для назначения особоых параметров переменных

#define Led_Ch_Cnt		6							//Количество каналов управления
#define Mov_Sens_Cnt 	2							//Количество датчиков движения

#define Relay_ON_Delay							400		//Задержка до изменения яркости на время включения питания 220В

#define	MovSens_Status_ON_Timeout		120000	//Задержка до сброса статуса датчика движения после начала однаружения движения (120 сек)
#define	MovSens_Status_OFF_Timeout	15000		//Задержка до сброса статуса датчика движения после окончания детекции движения (10 сек)

#ifdef DEBUG_MODE
#define Delay_Normal_Power_OFF			30000	//Задержка до выключения сетевого питания в обычном режиме

#define Charge_Time									20000 //Время работы зарядки батареи ( 1,5 часа = 90 мин *60 сек = 5400 сек )
#else
#define Delay_Normal_Power_OFF			60000	//Задержка до выключения сетевого питания в обычном режиме

#define Charge_Time									5400000//Время работы зарядки батареи ( 1,5 часа = 90 мин *60 сек = 5400 сек )
#endif

#define vBat_Low			2835					//(3.5V)Напряжение батареи ниже которого включается режим зарядки
#define vBat_Norm			3030					//3.75V = 3032
#define vBat_High			3320					//(4.1V)

#define Bright_Day			650					//Яркость освещения для дня
#define Bright_Night		150					//Яркость освещения для ночи

#define vLightSens_Night						2000	//Напряжение на датчике освещения ниже которого наступет ночь
#define vLightSens_Day							2300	//Напряжение на датчике освещения выше которого наступет день
#define vLightSens_Sun							3500	//Напряжение на датчике освещения выше которого наступет солнечный день (нет необходмости в освещении)

#define Lux_Data_Period							600000	//Время между занесениями в массив данных значения освещенности (600 000 = 10 мин)
#define Lux_Data_Len								500			//Количество элементов массива сбора освещенности

#define Time_correction							-6			//Корректировка времени в милисекундах на секунду (-5,635ms)
//extern Uart_struct	Uart;

typedef struct
{
	volatile uint16_t Year;
	volatile uint16_t Month;
	volatile uint16_t Day;
	volatile uint16_t Hour;
	volatile uint16_t Minute;
	volatile uint16_t Second;
	volatile uint16_t Delay;				//Задержка до срабатывания секундного тика
	volatile char			Tik;					//Флаг срабатывания секундного тика
}Time_struct;
	
typedef struct
{
	uint16_t 	Data[Lux_Data_Len];
	uint16_t	Pos;									//Позиция актуального значения освещенности
}LuxData_struct;

typedef struct
{
	volatile int			Bright;					//Текущая яркость	(0 - 1000)
	volatile int			Target_Bright;	//Целевая яркость	(0 - 1000)
	volatile uint32_t	Delay;					//Задержка до обработки яркости канала
	
	volatile int			Step;						//Nнкремент(декремент) яркости за шаг
	volatile uint32_t	Step_Delay;			//Задержка до следуюшего шага изменения яркости
	
	volatile uint16_t	Day_Bright;			//Яркость по умолчанию для дневного времени

	volatile uint16_t	Night_Bright;		//Яркость по умолчанию для ночьного времени	
}Channel_struct;

typedef struct
{
	Channel_struct	Channel[Led_Ch_Cnt];
}Led_struct;

typedef struct
{
	volatile char			Detect;				//Срабатывание (0 - 1)
	volatile uint32_t	LifeTime;			//Время жизни статуса срабатывания датчика	
}MovSensChannel_struct;

typedef struct
{
	MovSensChannel_struct 		Channel[Mov_Sens_Cnt];
}MovSens_struct;

typedef struct
{
	volatile char			Status;				//Статус питания от сети (0 - выкл., 1 - вкл.)
	volatile char			RelayState;		//Статус реле питания от сети (0 - выкл., 1 - вкл.)
	volatile char			ChangeFlag;		//Флаг изменения статуса
	volatile uint8_t	Consumers;		//Битовые статусы подключенных к питания потребителей (0 - выкл., 1 - вкл. для каждого бита)
	volatile uint32_t	ChangeDelay;	//Время задержки до разрешения на изменения статуса
}Power_struct;

//Биты статусов Consumers 
#define pc_Fartuk							0b00000001	//1<<0
#define pc_Floor							0b00000010	//1<<1
#define	pc_Cabinet						0b00000100	//1<<2
#define pc_ChargeBattery			0b10000000	//1<<7

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
//Датчик движения левый
#define		SensL_PORT				GPIOA
#define		SensL_PIN					GPIO_PIN_8
#define		SensL							(HAL_GPIO_ReadPin(SensL_PORT, SensL_PIN) == GPIO_PIN_SET) ? 1 : 0;
//Датчик движения правый
#define		SensR_PORT				GPIOB
#define		SensR_PIN					GPIO_PIN_15
#define		SensR							(HAL_GPIO_ReadPin(SensR_PORT, SensR_PIN) == GPIO_PIN_SET) ? 1 : 0;
//Твердотельное реле вкл/выкл
#define		Relay_PORT				GPIOB
#define		Relay_PIN					GPIO_PIN_14
#define  	Relay_ON	   			HAL_GPIO_WritePin(Relay_PORT, Relay_PIN, GPIO_PIN_SET);//GPIO_WriteBit( Relay_PORT, Relay_PIN ,Bit_SET)
#define  	Relay_OFF					HAL_GPIO_WritePin(Relay_PORT, Relay_PIN, GPIO_PIN_RESET);//GPIO_WriteBit( Relay_PORT, Relay_PIN ,Bit_RESET)
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
uint32_t Led_Prog_Exec(char i);
uint32_t GetDelayAndPowerON(void);
void Set_Led_Bright(char Led_Number, int bright);
void StatusLedRefresh(int bright);
void LuxHandler(uint16_t Lux);
void Clock_Handler(void);
int16_t getNumberOfDayInMonth(uint16_t month, uint16_t year);
char isLeapYear(uint16_t year);
void Time_Set (uint16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second );
void TimingDelay_Decrement(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Led5_Pin GPIO_PIN_0
#define Led5_GPIO_Port GPIOA
#define Led6_Pin GPIO_PIN_1
#define Led6_GPIO_Port GPIOA
#define Led1_Pin GPIO_PIN_6
#define Led1_GPIO_Port GPIOB
#define Led2_Pin GPIO_PIN_7
#define Led2_GPIO_Port GPIOB
#define Led3_Pin GPIO_PIN_8
#define Led3_GPIO_Port GPIOB
#define Led4_Pin GPIO_PIN_9
#define Led4_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
