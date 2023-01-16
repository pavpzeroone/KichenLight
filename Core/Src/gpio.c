/**
  ******************************************************************************
  * File Name          : gpio.c
  * Date               : 19/05/2014 15:58:59
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

Status_Led_Programm_Step_struct Led_Progs_Step[Led_Progs_Step_Count] = {
			{0,100},		//Просто выключение
			{1,100},		//Просто включение
			{1,400},		//Моргание
			{0,100},
			{1,100},		//Аварийное быстрое моргание
			{0,100},
			{0,1}};

Status_Led_Programm_struct	Led_Progs[Led_Progs_Count] = {
			{0,1},			//Выключение
			{1,1},			//Включение
			{2,2},			//Моргание
			{4,2},			//Аварийное быстрое моргание
			};
						
char	Status_Led_Prog_Use	= 0;	//Метка означающая работу программы Status_Led
char	Status_Led_Prog = 0;
char	Status_Led_Prog_Step = 0;
char	Status_Led_Prog_Rty = 0;			
			
char Key_Event = No_Press;			//Обнуляем событие кнопки	
			
/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
void PORT_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
  */

  /*Enable or disable APB2 peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);

  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA */	//Кнопка
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

/*Configure GPIO pin : PB */	//Статусный светодиод кнопки
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /** CAN GPIO Configuration  
//  PB8   ------> CAN_RX
//  PB9   ------> CAN_TX
//  */

//  /*Enable or disable APB2 peripheral clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);

//  /*Configure GPIO pin : PB */
//  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /*Configure peripheral I/O remapping */
//  GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

  /** TIM1 GPIO Configuration  
  PA8   ------> TIM1_CH1
  PA9   ------> TIM1_CH2
  PA10   ------> TIM1_CH3
  PA11   ------> TIM1_CH4
  */

  /*Enable or disable APB2 peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

//  /** TIM2 GPIO Configuration  
//  PA0-WKUP   ------> TIM2_CH1_ETR
//  PA1   ------> TIM2_CH2
//  PB10   ------> TIM2_CH3
//  PB11   ------> TIM2_CH4
//  */

//  /*Enable or disable APB2 peripheral clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);

//  /*Configure GPIO pin : PA */
//  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
//  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOA, &GPIO_InitStruct);

//  /*Configure GPIO pin : PB */
//  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
//  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /*Configure peripheral I/O remapping */
//  GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);

  /** TIM3 GPIO Configuration  
  PA6   ------> TIM3_CH1
  PA7   ------> TIM3_CH2
  PB0   ------> TIM3_CH3
  PB1   ------> TIM3_CH4
  */

  /*Enable or disable APB2 peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /** USART1 GPIO Configuration  
//  PB6   ------> USART1_TX
//  PB7   ------> USART1_RX
//  */

//  /*Enable or disable APB2 peripheral clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);

//  /*Configure GPIO pin : PB */
//  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /*Configure peripheral I/O remapping */
//  GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

//  /** USART2 GPIO Configuration  
//  PA2   ------> USART2_TX
//  PA3   ------> USART2_RX
//  */

//  /*Enable or disable APB2 peripheral clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//  /*Configure GPIO pin : PA */
//  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOA, &GPIO_InitStruct);
}

uint32_t	Key_Handler(void)
{
	static	char	State = 0;
	static	char	State_Last;					//Последнее зафиксированное состояние кнопки
	static	char	State_Wait;					//Ожидаемое состояние кнопки
	static	char	State_Cnt;					//Кол-во перидов нахождения кнопки в одном состоянии (антидребезг)
	static	unsigned int	Stable_Cnt;	//Кол-во перидов нахождения кнопки в одном стабильном состоянии
	static	char	Count_Press_Long;		//Номер удерживаемой кнопкой команды ( 1 - n )
	
	static 	char	Prog = 0;				//Программа кнопки				
	
	if ( Key_Press ^ State_Last ) //Изменение статуса нажатия/отжатия кнопки
	{	
		State_Last ^= 1;						//Меняем на противоположное
		State_Wait	= State_Last;		//Ожидаем новое состояние кнопки
		State_Cnt = 0;
	}
		
	if ( State_Wait != State )	
		if ( State_Cnt++ == Key_AntiNoise_Cnt ) 
		{
			State = State_Wait;
			Stable_Cnt = Key_AntiNoise_Cnt;
		}
	
	switch( Prog )	// _.-=|=-._.-=|=-._.-=|=-._.-=|=-._.-=|=-._
	{
		case 0: //Ищем короткое нажатие кнопки
		{ 
			if ( State == 1 )
				if ( Stable_Cnt >= Key_Press_Short_Cnt ) { Prog = 1; }	//Короткое нажатие достигнуто			
		break; }
		
		case 1: //Ищем длинное нажатие кнопки, либо подтверждение короткого нажатия
		{ 
			if ( State == 1 ) 	
			{	if ( Stable_Cnt >= Key_Hold_Long_Cnt ) 			//Длинное удержание достигнуто
				{ Prog = 2; 
					Count_Press_Long = 1;
					Status_Led_Prog_Set( Led_OFF, One_Shot );//Status_Led_OFF(); 
				} 
			}			
			else 
				if ( Stable_Cnt >= Key_Realise_Cnt ) 		//Короткое нажатие подтверждено + + + + + + + + +
				{ Key_Event = Short_Press;
					Status_Led_Prog_Set( Led_ON, One_Shot ); } 	
		break; }
		
		case 2: //Считаем длину нажатия кнопки, либо подтверждение длинного нажатия
		{ 
			if ( State == 1 ) 
			{	
				if ( Stable_Cnt >= Key_Hold_Long_Cnt + Key_Press_Long_Cnt * Count_Press_Long ) 
				{ Count_Press_Long++; Status_Led_Prog_Set( Led_Flash, One_Shot ); } 	//Длинное нажатие достигнуто					
			}		
			else 
				if ( Stable_Cnt >= Key_Realise_Cnt )		//Длинное нажатие подтверждено + + + + + + + + +
				{ Key_Event = Count_Press_Long; } 	
		break; }		
	}							// _.-=|=-._.-=|=-._.-=|=-._.-=|=-._.-=|=-._
	
	Stable_Cnt++;	
	
	return	Key_Look_Period;
}

void Status_Led_Prog_Set(char Number, char Retry)
{
	Status_Led_Prog_Use = 1;
	Status_Led_Prog = Number;
	Status_Led_Prog_Step = 0;
	Status_Led_Prog_Rty = Retry;
}

uint32_t Status_Led_Handler(void)
{
	if( Status_Led_Prog_Use )
	{ 
		char i = Led_Progs_Step[ Led_Progs[ Status_Led_Prog ].Step_Start + Status_Led_Prog_Step ].Bright;	//Определяем яркость (0 или 1)
		if( i == 1 ) 	Status_Led_ON();
		else					Status_Led_OFF();		
		
		if( ++Status_Led_Prog_Step == Led_Progs [ Status_Led_Prog ].Step_Len ) 
		{
			if( Status_Led_Prog_Rty == One_Shot ) Status_Led_Prog_Use = 0;
			if( Status_Led_Prog_Rty == Retry ) 		Status_Led_Prog_Step = 0;
		}
		i = Led_Progs_Step[ Led_Progs[ Status_Led_Prog ].Step_Start + Status_Led_Prog_Step ].Delay_After;	//Определяем задержку до след.шага
		return i;
	}
	return Led_Wait;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
