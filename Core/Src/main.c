/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart.h"
#include "command.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
Time_struct						Time;
Led_struct 						Led;
MovSens_struct				MovSens;
//Light_Status_struct		Light_Status;
LuxData_struct				LuxData;

Power_struct Power;			//Статусы питания 220В

char ADC_compl_flag = 0;				

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CRC_HandleTypeDef hcrc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
UART_HandleTypeDef *const Used_uart = &huart3;		//Выбор используемого UARTa

volatile uint32_t		ADC_Delay = 1000,
										Charger_Delay,
										LuxIntegry_Period = Lux_Data_Period;
uint16_t	Vbat[4] = { vBat_Norm, vBat_Norm, vBat_Norm, 0 },
					Vlsens[3];
					
//char firstByteWait=1; // признак ожидание первого байта		
//uint8_t buf=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_CRC_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	int i;
	char 	Mode = 0,
				Mode_step = 0;

	char	ADC_Step = 0;
	char	MoveSensL = 0, MoveSensL_ = 0,	//Данные с датчиков текущие и предыдущие
				MoveSensR = 0, MoveSensR_ = 0;
	
	Time.Year = 2023;
	Time.Month = 1;
	Time.Day = 18;
	Time.Hour = 19;
	
	LuxData.Pos = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM4_Init();
  MX_CRC_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADCEx_Calibration_Start(&hadc1);
	
	__HAL_TIM_ENABLE(&htim4);
	TIM_CCxChannelCmd(TIM4,TIM_CHANNEL_1,TIM_CCx_ENABLE);
	TIM_CCxChannelCmd(TIM4,TIM_CHANNEL_2,TIM_CCx_ENABLE);
	TIM_CCxChannelCmd(TIM4,TIM_CHANNEL_3,TIM_CCx_ENABLE);
	TIM_CCxChannelCmd(TIM4,TIM_CHANNEL_4,TIM_CCx_ENABLE);

	__HAL_TIM_ENABLE(&htim2);
	TIM_CCxChannelCmd(TIM2,TIM_CHANNEL_1,TIM_CCx_ENABLE);
	TIM_CCxChannelCmd(TIM2,TIM_CHANNEL_2,TIM_CCx_ENABLE);
	
	for(i=0;i<Led_Ch_Cnt;i++)	//Nнициализация начальных значений яркостей Led
	{ 
		Led.Channel[i].Bright = 0;
		Led.Channel[i].Target_Bright = 0;
		Led.Channel[i].Step = 10;
		Led.Channel[i].Step_Delay = 20;
		Led.Channel[i].Day_Bright = Bright_Day;
		Led.Channel[i].Night_Mode = Bright_Night;
		Led.Channel[i].Delay = 0;
		Set_Led_Bright(i, Led.Channel[i].Bright);
	}	
	
	Power.Status = 0;
	Power.ChangeFlag = 0;
	Power.Consumers = 0;
	Power.ChangeDelay = 0;
	//Light_Status.Fartuk = 0;
	//Light_Status.Floor = 0;
	//Light_Status.Cabinet = 0;
	
	//HAL_UART_Receive_IT (uart, &buf, 1); // запуск приема UART
	UART_Rx_Start(Used_uart);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Main cycle ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  {	
		//Обработчик UART
		//HAL_UART_RxCpltCallback(uart);
		UART_Buf_Rx_Handler();					//Обработчик полученного по UART буфера
		UART_Tx_Handler(Used_uart);			//Обработчик отправки кольцевого буфера
		
		//Запуск исполнителя команд получаемых по UART
		Command_Exec();
		
		//Обработчик вывода массива данных освещенности
		if( Comm_Task & t_LuxData_Show )
		{
			if( LuxData_Show(LuxData.Data, Lux_Data_Len, LuxData.Pos) == 0 ) Comm_Task &= (uint32_t) ~t_LuxData_Show;			//Выключаем разовай вывод освещенности
		}
		
		//Обработчик часов
		if( Time.Tik )Clock_Handler();
		
		if( Comm_Task & t_Time_Show )
		{
			Comm_Task &= (uint32_t) ~t_Time_Show;			//Выключаем разовай вывод времени
			Time_Show(Time.Year, Time.Month, Time.Day, Time.Hour, Time.Minute, Time.Second);
		}
		
		if( Comm_Task & t_Time_Set )
		{
			Comm_Task &= (uint32_t) ~t_Time_Set;			//Выключаем разовай вывод времени
			//if( isValid_DataTime ( ManualTime.Year
			Time_Set(ManualTime.Year, ManualTime.Month, ManualTime.Day, ManualTime.Hour, ManualTime.Minute, ManualTime.Second);
			Comm_Task |= t_Time_Show;													//Включаем разовый показ времени
		}
		
		//Обработчик изменения яркости Led
		for(i=0;i<Led_Ch_Cnt;i++)	{ if( Led.Channel[i].Delay == 0 ) Led.Channel[i].Delay = Led_Prog_Exec(i); }
		//Принудительное изменение яркости по команде от UART
		if( ManualLedSw.Led_Nbr ) { Led.Channel[ ManualLedSw.Led_Nbr-1 ].Target_Bright = ManualLedSw.Value; ManualLedSw.Led_Nbr = 0; }
		
		//Обработка данных с датчиков -	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
		MoveSensL = SensL;	//Считываем состояние портов с датчиков движения
		MoveSensR = SensR;
		
		if(MoveSensL != MoveSensL_)		//Если изменилось состояное с датчика
		{ MoveSensL_ = MoveSensL;
			if( MoveSensL == 1 )				//Датчик сработал
			{	MovSens.Channel[0].Detect = 1;
				MovSens.Channel[0].LifeTime = MovSens_Status_ON_Timeout;
			}
		}
		
		if(MoveSensR != MoveSensR_)		//Если изменилось состояное с датчика
		{ MoveSensR_ = MoveSensR;
			if( MoveSensR == 1 )				//Датчик сработал
			{	MovSens.Channel[1].Detect = 1;
				MovSens.Channel[1].LifeTime = MovSens_Status_ON_Timeout;
			}
		}
				
		//Обработчик сброса статуса срабатывания датчиков движения
		for(i=0;i<Mov_Sens_Cnt;i++)	
		{ if( MovSens.Channel[i].LifeTime == 0 ) 
			{	if( MovSens.Channel[i].Detect == 1)
				{
					MovSens.Channel[i].Detect = 0;
					MovSens.Channel[i].LifeTime = MovSens_Status_OFF_Timeout;
				}
				//else MovSens.Channel[i].Delay = MovSens_Status_ON_Timeout; 	
			}		
		}	// 	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	- -	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	

		//Обработчик включения/выключения питания 220В - - - - - - - - - - - - - - - -
		if (( Power.Status == 1 )&&( Power.Consumers == 0 )&&( Power.ChangeDelay == 0 ))
		{	//Параметры выключения
			Power.Status = 0;
			Power.ChangeFlag = 1;
			Power.ChangeDelay = Delay_Normal_Power_OFF;
		}
		
		if (( Power.Status == 0 )&&( Power.Consumers ))
		{	//Параметры включения
			Power.Status = 1;
			Power.ChangeFlag = 1;
			Power.ChangeDelay = 0;
		}
		
		//Обработчик статуса питания от сети
		if((Power.ChangeFlag == 1)&&( Power.ChangeDelay == 0))
		{	Power.ChangeFlag = 0;
			if(Power.Status == 1)	{ Relay_ON; Power.RelayState = 1; }
			else									{ Relay_OFF; Power.RelayState = 0; }
		} // 	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	- -	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	
		
		//Обработчик АЦП (считывание показаний Напряжения батареи и датчика света)
		if( ADC_Delay == 0 )
		{
			switch(ADC_Step)
			{
				case 0:	//Запуск измерения
				{	HAL_ADCEx_InjectedStart_IT(&hadc1);
					ADC_Delay = 10;
					ADC_Step++;
					break;
				}
				case 1:	//Ожидние окончания измерения
				{	static char cntRetry;
					if( ADC_compl_flag )	{ ADC_compl_flag = 0; ADC_Step++;	}
					else
					{
						if(cntRetry++ <= 10)	{ ADC_Delay = 10; }
						else 
						{	//Ошибка, не успевыет сделаться преобразования
							cntRetry = 0;
							ADC_Step = 0;
							ADC_Delay = 100;
							HAL_ADCEx_InjectedStop_IT(&hadc1);
						}
					}
					break;
				}
				case 2:	//Измерения готовы
				{
					Vbat[3]	= HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
					Vlsens[2] = 	HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
					ADC_Step = 0;
					ADC_Delay = 1990;//60000;					
					HAL_ADCEx_InjectedStop_IT(&hadc1);
					//Вывод значений в UART
					if(Comm_Task & t_Vbat_Show) Vbat_Show(Vbat[3]);
					if(Comm_Task & t_Vsolar_Show) Vsolar_Show(Vlsens[2]);
					break;
				}
			}
		} // 	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	- -	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
		
		//Обработчик контроллера заряда батареи - - - - - - - - - - - - - - - - - - - -
		if( Vbat[3] )
		{	//Вычисляем усредненное значение для Vbat[0], сдвигаем старые значения
			for(uint16_t i=1; i<=3; i++)
			{
				Vbat[0] >>= 1;
				Vbat[0] += Vbat[i]>>1;				
			}
			Vbat[1] = Vbat[2];
			Vbat[2] = Vbat[3];
			Vbat[3] = 0;
			
			if( Vbat[0] < vBat_Low )
			{	//Включаем режим заряда батареии
				Power.Consumers |= pc_Battery;							//Включаем флаг зарядка батареи
				Charger_Delay = Charge_Time;
			}
			
			if( Charger_Delay == 0 )
			{ //Выключаем режим заряда батареии
				Power.Consumers &= (uint8_t) ~pc_Battery;	//Выключаем флаг зарядка батареи
			}
			else if( Vbat[0] >= vBat_High ) Charger_Delay = 0;	//Если батарея заряжена ускоряем окончание зарядки
		} // 	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	- -	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
		
		//Обработчик данных с датчика освещения
		if( Vlsens[2] )
		{	//Вычисляем усредненное значение для Vlsens[0], сдвигаем старые значения			
			Vlsens[0] >>= 1;	Vlsens[0] += Vlsens[1]>>1;				
			Vlsens[0] >>= 1;	Vlsens[0] += Vlsens[2]>>1;				
			Vlsens[1] = Vlsens[2];
			Vlsens[2] = 0;
			
			LuxHandler(Vlsens[0]);			//Обработчик значений освещенности
			
			//Обработчик смены режина работы День/Ночь
			if(( Vlsens[0] < vLightSens_Night ) && ( Mode == 1 ))
			{
				Mode = 2;
			}
			
			if(( Vlsens[0] > vLightSens_Day ) && ( Mode == 3 ))
			{
				Mode = 4;
			}
		}
		
		switch(Mode)
		{
			case 0:		//Start Mode
			{
				switch(Mode_step)
				{
					case 0:
					{
//						Relay_ON;
//						i = SensL;
						Mode_step++;
						Mode_step = 10;
						break;
					}
					case 10:
					{
						Mode_step = 0;
						Mode++;
						break;
					}
				}
				
				
			}
			case 1:		//Day Mode
			{	
				if((Power.Consumers & pc_Fartuk) == 0 )			//Если фартук выключен
				//if( Light_Status.Fartuk == 0 )
				{	//Если сработали оба датчика движения - включаем свет на фартуке
					if((MovSens.Channel[0].Detect == 1) && (MovSens.Channel[1].Detect == 1)) 
					{	//Light_Status.Fartuk = 1;						
						Power.Consumers |= pc_Fartuk;							//Включаем флаг фартук
						Led.Channel[0].Target_Bright = Led.Channel[0].Day_Bright;
						Led.Channel[1].Target_Bright = Led.Channel[1].Day_Bright;		
						if( Power.Status == 0 ) Led.Channel[0].Delay = Relay_ON_Delay;	
						else										Led.Channel[0].Delay = 0;
						//Led.Channel[0].Delay = GetDelayAndPowerON();	
						Led.Channel[1].Delay = Led.Channel[0].Delay;
					}
				}
				else	//Поддержание работы освещения фартука
				{
					if((MovSens.Channel[0].Detect == 0) && (MovSens.Channel[1].Detect == 0)) 
					{
						//Начинаем выключение фартука
						Led.Channel[0].Target_Bright = 0;
						Led.Channel[1].Target_Bright = 0;					
						//Light_Status.Fartuk = 0;
						Power.Consumers &= (uint8_t) ~pc_Fartuk;	//Выключаем флаг фартук
					}
				}
				break;
			}
			case 2:		//Переход в Night Mode
			{
				Mode++;
				break;
			}			
			case 3:		//Night Mode
			{
				if((Power.Consumers & pc_Fartuk) == 0 )			//Если фартук выключен				
				{	//Если сработали оба датчика движения - включаем свет на фартуке
					if((MovSens.Channel[0].Detect == 1) && (MovSens.Channel[1].Detect == 1)) 
					{	Power.Consumers |= pc_Fartuk;							//Включаем флаг фартук
						Led.Channel[0].Target_Bright = Led.Channel[0].Night_Bright;
						Led.Channel[1].Target_Bright = Led.Channel[1].Night_Bright;		
						if( Power.Status == 0 ) Led.Channel[0].Delay = Relay_ON_Delay;	
						else										Led.Channel[0].Delay = 0;						
						Led.Channel[1].Delay = Led.Channel[0].Delay;
					}
				}
				else	//Поддержание работы освещения фартука
				{
					if((MovSens.Channel[0].Detect == 0) && (MovSens.Channel[1].Detect == 0)) 
					{	//Начинаем выключение фартука
						Led.Channel[0].Target_Bright = 0;
						Led.Channel[1].Target_Bright = 0;											
						Power.Consumers &= (uint8_t) ~pc_Fartuk;	//Выключаем флаг фартук
					}
				}							
				break;
			}
			case 4:		//Переход в Day Mode
			{
				Mode = 1;
				break;
			}	
		}
		
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_InjectionConfTypeDef sConfigInjected = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_3;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedNbrOfConversion = 2;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.InjectedOffset = 0;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_4;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
	//Добавить в HAL отсутсвующую строку
	//sConfigInjected.InjectedChannel = ADC_CHANNEL_4;
  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 7;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD0 PD1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA5 PA6 PA7
                           PA9 PA10 PA11 PA12
                           PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB12
                           PB13 PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure peripheral I/O remapping */
  __HAL_AFIO_REMAP_PD01_ENABLE();

}

/* USER CODE BEGIN 4 */
//Обработчик окончания измерения ADC
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{ if(hadc->Instance == ADC1) ADC_compl_flag = 1; }

void Set_Led_Bright(char Led_Number, int bright)
{switch(Led_Number)
	{	case 0:	{__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_1,bright);	break;}	//Led1	
		case 1:	{__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_2,bright);	break;}	//Led2	
		case 2:	{__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_3,bright);	break;}	//Led3	
		case 3:	{__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,bright);	break;}	//Led4			
		case 4:	{__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_1,bright);	break;}	//Led5			
		case 5:	{__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,bright);	break;}	//Led6		
	}	
}

uint32_t Led_Prog_Exec(char i)
{	uint32_t a;	
	if(Led.Channel[i].Bright != Led.Channel[i].Target_Bright)	//Если целевая яркость не достигнута -----------
	{	//Nзменяем текущую яркость Led	
		if( Led.Channel[i].Target_Bright > Led.Channel[i].Bright )
		
		{	if( (Led.Channel[i].Bright + Led.Channel[i].Step) >= Led.Channel[i].Target_Bright ) Led.Channel[i].Bright = Led.Channel[i].Target_Bright;
			else Led.Channel[i].Bright += Led.Channel[i].Step;
		}
		else																			
		{	if( Led.Channel[i].Bright <= Led.Channel[i].Step ) Led.Channel[i].Bright = 0;
			else Led.Channel[i].Bright -= Led.Channel[i].Step;
		}
		
		Set_Led_Bright(i, Led.Channel[i].Bright);

		a = Led.Channel[i].Step_Delay;
	}
	else														//????? ????????? ? ?????????? ???? ????????? ------------------------------
	{
		//a = 0xFFFF;
		a = Led.Channel[i].Step_Delay;
	}
	
	return a;
}

void LuxHandler(uint16_t Lux)
{
	static uint16_t Lux_data[500];
	static uint16_t index = 0;
	static uint16_t count = 0;
	static uint16_t step = 0;
	
	if(LuxIntegry_Period == 0)
	{	//Если время периода вышло переходим к следующей ячейке
		if(++index == 500) index = 0;
		if(++count > 500) count = 500;
		step = 0;
	}
	
	if(step == 0) Lux_data[index] = Lux;
	else
	{ //Заносим усредненное значение в массив
		Lux_data[index] = (int32_t)( Lux_data[index] + Lux ) / 2;
	}
	step++;
}

void Clock_Handler(void)
{
	Time.Tik = 0;
	if( ++Time.Second == 60 )
	{
		Time.Second = 0;
		if( ++Time.Minute == 60 )
		{
			Time.Minute = 0;
			if( ++Time.Hour == 24 )
			{
				Time.Hour = 0;
				if( ++Time.Day == getNumberOfDayInMonth(Time.Month, Time.Year)+1 )
				{
					Time.Day = 1;
					if( ++Time.Month == 13 )
					{
						Time.Month = 1;
						Time.Year++;
					}
				}
				
			}
		}
	}
}

//Подсчет количества дней в месяце
int16_t getNumberOfDayInMonth(uint16_t month, uint16_t year)
{
	uint16_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	uint16_t days = daysInMonth[month-1];
	if( ( month == 2 ) && (isLeapYear(year) == 1 ) ) days++;
	return days;
}

//Проверка года на високосность
char isLeapYear(uint16_t year)
{
	if (year % 400 == 0) return 1;
	if (year % 100 == 0) return 0;
	if (year % 4 == 0) return 1;
	return 0;
}

//Проверка и установка даты и времени
void Time_Set (uint16_t Year, uint16_t Month, uint16_t Day, uint16_t Hour, uint16_t Minute, uint16_t Second )
{
	if(( Year >= 2000 ) && ( Year < 3000 ))
		if( Month <= 12 )
			if( Day <= getNumberOfDayInMonth( Month, Year ) )
				if( Hour < 24 )
					if( Minute < 60 )
						if( Second < 60 ) 
						{ Time.Year = ManualTime.Year; 
							Time.Month = ManualTime.Month; 
							Time.Day = ManualTime.Day;
							Time.Hour = ManualTime.Hour;
							Time.Minute = ManualTime.Minute;
							Time.Second = ManualTime.Second;
						}
}

//Функция включения статусов питания и возврата задержки для ожидания стабилизации напряжения после включения
//Action: 0 - OFF
//				1 - ON
//uint32_t PowerSet_DelayGet(uint32_t Initiator, char Action)
//{	//Режим включения
//	if( Action == 1)
//	{	//Если режим ВКЛючения и потребитель уже включен
//		if( Power.Consumers & Initiator ) return 0;
//		else
//		{ //Если потребитель не включен
//			Power.Consumers |= Initiator;				//Включаем потребителя
//			if( Power.Status == 1 ) return 0;
//			else
//			{
//				Power.Status = 1;
//				Power.Change = 1;
//				//Relay_ON;
//				return Rel_ON_Delay;
//			}
//		}
//	}
//		//Режим выключения
//	else
//	{ //Если режим ВЫКЛючения и потребитель включен
//		if( Power.Consumers & Initiator ) 
//		{
//			Power.Consumers &= (uint32_t) ~Initiator;	//Выключаем потребителя
//			if( Power.Status == 1 )
//			{
//				Power.LifeTime = Delay_Normal_Power_OFF;
//			}
//		}
//	}
//}

void TimingDelay_Decrement(void)
{ char i;
	
	for(i=0;i<Led_Ch_Cnt;i++) { if( Led.Channel[i].Delay ) Led.Channel[i].Delay--; }
	for(i=0;i<Mov_Sens_Cnt;i++) { if( MovSens.Channel[i].LifeTime ) MovSens.Channel[i].LifeTime--; }
	if( ADC_Delay ) ADC_Delay--;
	if( Power.ChangeDelay ) Power.ChangeDelay--;
	if( Charger_Delay ) Charger_Delay--;
	if( LuxIntegry_Period ) LuxIntegry_Period--;
	if( Time.Delay ) Time.Delay--; else { Time.Delay = 1000; Time.Tik = 1; }
	
	#ifdef DBG_USART
		if( TimingDelay_DBG_Temp ) TimingDelay_DBG_Temp--;
	#endif
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
