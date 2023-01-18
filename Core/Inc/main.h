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
#define Led_Ch_Cnt	6								//Количество каналов управления
#define Mov_Sens_Cnt 2							//Количество датчиков движения

#define Bright_Day			500					//Яркость освещения для дня
#define Bright_Night		100					//Яркость освещения для ночи

#define Relay_ON_Delay		400				//Задержка до изменения яркости на время включения питания 220В

#define	MovSens_Status_ON_Timeout		30000	//Задержка до сброса статуса датчика движения (30 сек)
#define	MovSens_Status_OFF_Timeout		500	//Задержка до сброса статуса датчика движения (0,5 сек)

#define Delay_Normal_Power_OFF			60000	//Задержка до выключения сетевого питания в обычном режиме

#define Charge_Time									30000 //5400000//Время работы зарядки батареи ( 1,5 часа = 90 мин *60 сек = 5400 сек )

#define vBat_Low			2978					//Напряжение батареи ниже которого включается режим зарядки
#define vBat_Norm			3300
#define vBat_High			3690

#define vLightSens_Night						2000	//Напряжение на датчике освещения ниже которого наступет ночь
#define vLightSens_Day							2300	//Напряжение на датчике освещения выше которого наступет день

#define Lux_Data_Period							600000	//Время между занесениями в массив данных значения освещенности (600 000 = 10 мин)

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
	volatile int			Bright;					//Текущая яркость	(0 - 1000)
	volatile int			Target_Bright;	//Целевая яркость	(0 - 1000)
	volatile uint32_t	Delay;					//Задержка до обработки яркости канала
	
	volatile int			Step;						//Nнкремент(декремент) яркости за шаг
//	volatile char			Step_Direction;	//Направление повышение/понижение яркости (1-?+ , 0-?-)
	volatile uint32_t	Step_Delay;			//Задержка до следуюшего шага изменения яркости
	
	volatile uint16_t	Day_Bright;			//Яркость по умолчанию для дневного времени
	volatile char			Day_Mode;				//Режим работы  для дневного времени

	volatile uint16_t	Night_Bright;		//Яркость по умолчанию для ночьного времени
	volatile char			Night_Mode;			//Режим работы  для ночьного времени	
	
}Channel_struct;

typedef struct
{
	Channel_struct	Channel[Led_Ch_Cnt];
}Led_struct;

typedef struct
{
	volatile char		Detect;					//Срабатывание (0 - 1)
	volatile uint32_t	LifeTime;			//Время жизни статуса срабатывания датчика
	
}MovSensChannel_struct;

typedef struct
{
	MovSensChannel_struct 		Channel[Mov_Sens_Cnt];
}MovSens_struct;

typedef struct
{
	volatile char		Fartuk;					//Статус работы освещения фартука (0 - выкл., 1 - вкл.)
	volatile char		Floor;
	volatile char		Cabinet;
}Light_Status_struct;

typedef struct
{
	volatile char			Status;				//Статус питания от сети (0 - выкл., 1 - вкл.)
	volatile char			RelayState;		//Статус реле питания от сети (0 - выкл., 1 - вкл.)
	volatile char			Change;				//Флаг изменения статуса
	volatile uint32_t	Consumers;		//Битовые статусы подключенных к питания потребителей (0 - выкл., 1 - вкл. для каждого бита)
	volatile uint32_t	ChangeDelay;	//Время задержки до разрешения на изменения статуса
}Power_struct;

//Биты статусов Consumers 
#define pc_Fartuk							1<<0
#define pc_Floor							1<<1
#define	pc_Cabinet						1<<2
#define pc_Battery						1<<7


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
void LightHandler(uint16_t Lux);
void Clock_Handler(void);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
