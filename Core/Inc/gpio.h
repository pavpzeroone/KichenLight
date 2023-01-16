/**
  ******************************************************************************
  * File Name          : gpio.h
  * Date               : 19/05/2014 15:59:00
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
	 
#define  	Power_Hold_ON()   	GPIO_WriteBit( GPIOB, GPIO_Pin_12 ,Bit_SET)
#define  	Power_Hold_OFF()		GPIO_WriteBit( GPIOB, GPIO_Pin_12 ,Bit_RESET)	 
	 
#define		Status_Led_ON()   	GPIO_WriteBit( GPIOB, GPIO_Pin_5 ,Bit_SET)
#define  	Status_Led_OFF()		GPIO_WriteBit( GPIOB, GPIO_Pin_5 ,Bit_RESET)	 

#define		Key_Press			( GPIOA->IDR & (1<<(12)))			// Результат 0 - нет нажатия / 1 - кнопка нажата
	 
#define		Key_Look_Period				5													//Частота опроса кнопки	(мс)
#define		Key_AntiNoise_Cnt			15		/ Key_Look_Period		//Кол-во срабатываний до гарантированого определения нажатия/отжатия (ввод в мс)
#define		Key_Press_Short_Cnt		200		/ Key_Look_Period		//Кол-во срабатываний до определения короткого нажатия (ввод в мс)
#define		Key_Press_Long_Cnt		1500	/ Key_Look_Period		//Кол-во срабатываний до определения короткого нажатия (ввод в мс)
#define		Key_Hold_Long_Cnt			2000	/ Key_Look_Period		//Кол-во срабатываний до определения короткого нажатия (ввод в мс)
#define		Key_Realise_Cnt				100		/ Key_Look_Period		//Кол-во срабатываний до определения отжатия кнопки (ввод в мс)	 
	 
#define		Led_Wait					10				//Частота работы со светодиодом
	 
void PORT_Init(void);
uint32_t	Key_Handler(void);
uint32_t 	Status_Led_Handler(void);
void Status_Led_Prog_Set(char Number, char Retry);

extern char Key_Event;

enum	//Названия событий кнопки
{	No_Press			= 0,
	Long_Press_1 	= 1,
	Long_Press_2	=	2,
	Long_Press_3	= 3,
	Long_Press_4	= 4,
	Long_Press_5	= 5,
	Long_Press_6	= 6,
	Long_Press_7	= 7,
	Short_Press		= 100
};

enum	//Названия программ Status_Led
{	Led_OFF		=	0,
	Led_ON		=	1,
	Led_Flash	= 2,
	Led_Alarm	= 3
};

enum	//Названия вариантов повтора
{ One_Shot	=	0,
	Retry			=	1
};

//Параметры программ					
#define Led_Progs_Count 	4				//Общее кол-во программ
#define Led_Progs_Step_Count	7		//Общее кол-во шагов всех программ	

//Структуры базы данных программ ------------------------------------
typedef struct
{
	char	Step_Start;	//Номер первого шага программы
	char	Step_Len;		//Кол-во шагов в программе
}Status_Led_Programm_struct;

typedef struct
{
	char					Bright;				//Целевая яркость к концу шага ( 0 - 1 )
	unsigned int	Delay_After;	//Длина шага в мс
}Status_Led_Programm_Step_struct;



#ifdef __cplusplus
}
#endif
#endif /*__ gpio_H */



#define 	Heaters_PORT			GPIOB

#define  	Heater_1_PIN			GPIO_Pin_2
#define  	Heater_2_PIN			GPIO_Pin_13
#define  	Heater_3_PIN			GPIO_Pin_14
#define  	Heater_4_PIN			GPIO_Pin_15

#define  	Heater_1_ON()   	GPIO_WriteBit( Heaters_PORT, Heater_1_PIN ,Bit_SET)
#define  	Heater_1_OFF()		GPIO_WriteBit( Heaters_PORT, Heater_1_PIN ,Bit_RESET)

#define  	Heater_2_ON()   	GPIO_WriteBit( Heaters_PORT, Heater_2_PIN ,Bit_SET)
#define  	Heater_2_OFF()		GPIO_WriteBit( Heaters_PORT, Heater_2_PIN ,Bit_RESET)

#define  	Heater_3_ON()   	GPIO_WriteBit( Heaters_PORT, Heater_3_PIN ,Bit_SET)
#define  	Heater_3_OFF()		GPIO_WriteBit( Heaters_PORT, Heater_3_PIN ,Bit_RESET)

#define  	Heater_4_ON()   	GPIO_WriteBit( Heaters_PORT, Heater_4_PIN ,Bit_SET)
#define  	Heater_4_OFF()		GPIO_WriteBit( Heaters_PORT, Heater_4_PIN ,Bit_RESET)


#define		Relay_PORT				GPIOA
#define		Relay_PIN					GPIO_Pin_8

#define  	Relay_ON()   			GPIO_WriteBit( Relay_PORT, Relay_PIN ,Bit_SET)
#define  	Relay_OFF()				GPIO_WriteBit( Relay_PORT, Relay_PIN ,Bit_RESET)
//===========================================================================


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
