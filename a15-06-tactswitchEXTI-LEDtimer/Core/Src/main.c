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
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */




/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */



char pc13flag; //PC13의 플래그
char pc7flag; //PC7의 플래그
int LED_pa6_cnt,LED_pa6_Limit; //PA6카운트, 제한 선언
int LED_pa7_cnt,LED_pa7_Limit; //PA7카운트, 제한 선언

short LED_PA6_PINSTATE;
short LED_PA7_PINSTATE;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART2 and Loop until the end of transmission */
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//외부 인터럽트 발생시 (아마 TACT 스위치가 눌린듯✨✨✨✨)
	if(GPIO_Pin == GPIO_PIN_13){ //GPIO핀입력이 13번이라면
		//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		if('n' == pc13flag){ //PC13의 플래그가 no상태라면 (main에서 정의됨)
			pc13flag = 'y'; //PC13을 yes 상태로 전환
			printf("-----pc13flag = %c-----\r\n",pc13flag); //PC13 플래그를 콘솔로 출력
			pc13flag = 'n'; //다시 PC13을 no로 바꿈



			//토글이 구현된게 아니라 누를 때 잠깐 yes로 바뀌었다가 바로 🎁🤢구라치고 no로 바뀜
		}

		/*

		//🎉🌹🌹🌹🎉🌹🌹 토글 코드. 아래 코드 주석 제거해서 사용

		if('n' == pc13flag){
			pc13flag = 'y';
		}else if('y' == pc13flag){
			pc13flag = 'n';
		}


		printf("-----pc13flag = %c-----\r\n",pc13flag);
		//13 플래그 출력

		*/
	}
	if(GPIO_Pin == GPIO_PIN_7){ //입력 GPIO핀이 7이라면
		//----- 여기에 코드를 추가하기 ------------------------------------------------

		//아마도 pc7의 플래그를 토글해야할듯? 위의 코드는 토글이 아니라 🎁🤢순간적으로 구라를 치는 것



		//🎉🌹🌹🌹🎉🌹🌹 토글 코드. 아래 코드 주석 제거해서 사용

		if('n' == pc7flag){
			pc7flag = 'y';

			LED_pa6_Limit = 300;
			LED_pa7_Limit = 800;

		}else if('y' == pc7flag){
			pc7flag = 'n';

			LED_pa6_Limit = 900;
			LED_pa7_Limit = 200;

		}

		printf("-----pc7flag = %c-----\r\n",pc7flag);
	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{ //타이머 콜백 루프
	if(htim->Instance == htim2.Instance){

		LED_pa7_cnt++;
		LED_pa6_cnt++;


		if(LED_pa7_cnt >= LED_pa7_Limit){
			//LED7 cnt가 값을 넘어가면
			LED_pa7_cnt = 0;

			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
			//LED_PA7_PINSTATE = !(LED_PA7_PINSTATE); //핀 값 반전
		}

		if(LED_pa6_cnt >= LED_pa6_Limit){
			//LED6가 값을 넘으면

			LED_pa6_cnt = 0; //초기화
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
			//LED_PA6_PINSTATE = !(LED_PA6_PINSTATE); //핀 값 반전

		}

		//위의 코드는 1ms마다 실행됩니다
		//----- 여기에 코드를 추가하기 ------------------------------------------------
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	HAL_StatusTypeDef tim2_it_start;
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
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("-a15- tact Switch(exti) & LED(timer)***\r\n"); //시작시 tact switch & led임을 알림






  tim2_it_start = HAL_TIM_Base_Start_IT(&htim2);



  if(HAL_OK == tim2_it_start){ //타이머 실행시
	  printf("HAL_TIM_Base_Start_IT OK\r\n"); //프린트
  }else{
	  printf("HAL_TIM_Base_Start_IT error\r\n");
  }



  pc13flag = 'n'; //PC13 플래그 no로 초기화
	pc7flag = 'n';
  //pc7flag = 'f'; //왜 시발 얘는 n이 아니라 f냐

	LED_PA6_PINSTATE = 1; //아래 RESET을 가져옴 RESET이 1임
	LED_PA7_PINSTATE = 1;



  LED_pa6_cnt = 0; //PA6 카운터 0으로 초기화
  LED_pa6_Limit = 900; //최댓값은 900
  LED_pa7_cnt = 0; //PA7 카운터 0으로 초기화
  LED_pa7_Limit = 200; //최댓값은 200
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //PA6 RESET
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); //PA7 RESET
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */

		/*
		if(LED_PA7_PINSTATE){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		}else{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
		}

		if(LED_PA6_PINSTATE){ //true -> reset false -> set
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		}else{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		}
*/



		/*
		External interrupt(rising)
Initial value
LED(PA6):900msToggle
LED(PA7):200msToggle
If(tact switch[first])
LED(PA6):300msToggle
LED(PA7):800msToggle
else If(tact switch[second])
LED(PA6):900msToggle
LED(PA7):200msToggle

		PC7에서 Interupt가 Rising 되었을 때 작동하는 현상들
		무슨뜻인지는 잘 모르겠다.

		🔊🔊🔊🔊🔊

		200ms 기다리고 토글하는걸까? 아니면 토글 하고 200ms 다음에 다시 토글하는걸까

		둘다 아니였다. 역시 c. 알아먹기가 쉽지 않다
		PA6와 PA7은 TACT 스위치의 인터럽트가 들어올 떄 마다 상단에 표기된 시간마다 깜빡이는 시간을 변경하는 코드였다
		👊👊👊
		*/






		/*
	  if('y' == pc7flag){ //PC7 플래그가 y라면
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	  }else if('n' == pc7flag){

			//왜 하나만 켜지는건 없음?🐱‍👤🐱‍👤🐱‍👤🐱‍👤

		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	  }
		*/


	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	  HAL_Delay(300);
  }  //end of while
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 64-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : B1_Pin PC7 */
  GPIO_InitStruct.Pin = B1_Pin|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin PA6 PA7 */
  GPIO_InitStruct.Pin = LD2_Pin|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
