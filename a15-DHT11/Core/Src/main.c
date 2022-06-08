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
#include "stdio.h" //?��?���?
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
#endif /* __GNUC__ */ //?��매일 보던�?


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t utx[16]; //?��보내?�� 버퍼
uint8_t urx[16]; //?��받는 버퍼

int tim3Cnt; //?��???��머에 카운?���? ?��?��
int BitDataIndex; //?��비트 ?��?��?���? ?��?��?��?�� ?��?��?��
uint8_t DHT11BitData[64]; //?��?
uint8_t DHT11value[8]; //?��?



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
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

void DHT11_GPIO_MODE_OUTPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; //?��GPIO 구조체�?? 만들?��?��... struct ?��?�� ?��?��?�� ?��?��?��

	/*Configure GPIO pin : PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5; //?��PIN5 �??��
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //?��OUTPUT PP�? �??�� (?)
	GPIO_InitStruct.Pull = GPIO_NOPULL; //?��no pull�? �??�� pull up pull down ?��?��건듯
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; //?��Frequency low?��?��
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //?��GPIOB?�� GPIO_InitStruct?��?�� 구조체�?? ?��?��?��... ?���? ?�� ?��각에 ?��?���? ?���? ioc�? 건드리�? ?���? ???��?��?�� ?�� ?�� ?��?�� 것으�? 보인?��

	//?��GPIOB5�? Output?���? ?��?��?��...
}

void DHT11_GPIO_MODE_INPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; //?��구조체인?��

	/*Configure GPIO pin : PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5; //?��PIN5...
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; //?��Interupt Falling?��?��
	GPIO_InitStruct.Pull = GPIO_NOPULL; //?��no pull
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //?��?��번에?�� B?�� 집어?��?? 것으�? �? ?�� ?�� ?��?��?�� B5�? ?��?��모드�? 만들�?, ?��?�� 출력 모드�? 바꾸?�� ?��?��?��?��

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); //?��NVIC ?��?��?��?��... External Interupt 9-5?��?��... ?��?��?��?�� 0 0 ?��?��것으�? 추정
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn); //?��IRQ ?��?��?��?���?
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{ //?��???���? ?��?��
	if(htim->Instance == htim3.Instance){ //?��???���? 3?���?
		tim3Cnt += 1; //?��???���? 카운?���? 1 ?��린다... ???��머는 PSC 64-1, ARR 10-1�? 10us마다 ?��?��?��?�� 것으�? 추측?��?��. tim3Cnt�? 24?���? 240us�? �??��것으�? 보임
		// test pin
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); //?��A0???�� ?���??��?��... ?��??��
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{ //?��?��?��?��?�� 발생?��
	if(GPIO_Pin == GPIO_PIN_5){ //?��???�� 5번이?���?... ?��?��?�� B5�? ???���? ?��?��?��?��. ?��기서 ???? Single Wire - Two Way?��?�� ?? ?��?���? ?��?��?��?��?��
		DHT11BitData[BitDataIndex++] = tim3Cnt; //?��DHT11?��?��?�� 비트 배열 (64까�? ?��?��)�? main?��?�� 0?���? ?��?��?�� ?��?��?�� ?��?��?��?�� ???���? 카운?���? 집어?��?��?��.
		//?��비트 ?��?��?��?�� ?��?��?���? 집어?���? ?��?�� 1?�� 커진?��... ?��?��?��?���? 발생?�� ?�� 마다 비트 ?��?��?���? 커�??��?��


		tim3Cnt = 0; //?��?��?��?�� 카운?���? 집어?��?? 길이�? ?���? 0?���? 반환?��?��... ?���? 목적?? ?��?��?��?�� 길이 (?���?)?�� 측정?��?��?�� 것으�? 보임
		// test pin
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0); //?��C0???�� 값을 바꾼?�� C0 ???? ?��?���? 뭘까?��



		/*

			?�� ?�� ?��?��?��?�� ?��?��?��?��?�� pin5?�� ?��?��... ?���? B5?��?��
			?��?��?��?���? 발생?�� 것으�? ?��?��?���? ?��?��.


		*/
	}
}

int DHT11_Data_Read(int sel)
{ //?��DHT11 ?��?��?�� ?���? ?��?��... 무조�? sel?? 1?��?��.
	int i,j,k; //?��ijk?�� ???��?��?�� 반복문의 �??��?��?��
	//uint8_t checksum; //?��8바이?�� 체크?��...

	DHT11_GPIO_MODE_OUTPUT(); //?��GPIO�? Output?���? ?��?��
	HAL_Delay(1); //?��1ms ??�?
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //?��B5�? Low�?...
	HAL_Delay(18); //?��18ms ??�?
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //?��B5�? High�?...



	/*

		?��?�� 코드?�� ?��?�� ?��?���? 보내?�� 것으�? 보인?��



		?���추�? 추측... ?��?��?�� OUTPUT 모드�? 바꾸?�� ?��?���? ?��?�� B5 ???�� ?��?�� MCU->DHT�? ?��?�� ?��?���? 보낸?��
	*/

	tim3Cnt = 0; //?��???���? 카운?���? 0?���? 만든?��
	BitDataIndex = 0; //?��?��?��?�� ?��?��?���? 처음?���? ?��긴다... �?금�??�� 발생?��?�� 값�? ???��머�? ?��?��?��?��?�� 것으�? 보인?��
	for(i = 0; i < 64; i++){
		DHT11BitData[i] = 0; //?��비트 ?��?��?���? �??��?�� 0�??�� 64?�� ?��?��?���? ?�� �??��?��
	}
	for(i = 0; i < 8; i++){ //?��값을 �??��?��
		DHT11value[i] = 0; //?��값을 ?���? 0?���? 만든?�� (?��?��)
	}

	DHT11_GPIO_MODE_INPUT(); //?��?��?? ?��르게 ?��?�� 모드�? 바�?�다

	HAL_Delay(500); // DHT11 received time //?��500ms 기다린다...

	/*

		?��추측

		?�� 코드?�� ?��?��까�? 발생?�� ?��?��?�� ?��?��(?��?�� ?��?���? 보낸?��?�� ?��?��?�� 받�? ?��?��. ?��?��?���? ???��머는 ?��?�� 코드�? ?��?��?�� ?��?��?���? ?��문에 ?��?��?�� ?��?��?�� ?��?��?��?�� 것으�? 추측?��?��)
		그런 ?��?��?�� ?���? �??��?���?,

		�?�?고있?�� ?��?��?��?? 값도 ?���? �??��?��.

		�? ?��?�� B5???�� ?��?��모드�? 바꾸�? 500ms기다린다... ?��무래?�� DHT�? 보내?���? ???���?-?��?��?��?���? 기록?��고있�? ?��문에 ?��기서?��
		기다리기�? ?��?��?��

	*/


	/*

		?����??�� 추측

		?��?��?��?��?��?��?�� 길이�? 70us?�� High Level Signal?? 1?��?���? ?��?��
		?��?�� 길이�? 26~28?��?�� ?��?�� 길이?�� High Level Signal?? 0?��?���? ?��?��


		?���? ?��?�� ?��?��?�� 측정?�� (???��머는 10us마다 1?�� 체크?��?��) ???���? ?��?��?���? 바탕?���?
		n개의 DHT11BitData?��?�� 측정?�� 길이�? ?��?��?��?�� 것이?��.

		???��머는 10us?��?���? 측정?���? ?��문에

		2~5?�� 길이�? ?��?��?��?�� 값�? 0?���?, 7~10?�� 길이�? ?��?��?��?�� 값�? 1?��?��.

		?���? ?��?�� ?��?��?���? ?��?��?�� ?�� ?���? n개의 DHT11BitData?�� 각각 ?��?�� 비트?? ???��?��?��



		++?��?

		?��?�� ?��?��?��?�� 길이?�� 5바이?��?��?��.
		?��?��?���? 1�? 바이?��?�� ?��?��?�� ?��?���?, 2�? 바이?��?�� ?��?��?�� ?��?���?�? ?��미하�?
		3�? 바이?��?�� ?��?��?�� ?��?���?, 4�? 바이?��?�� ?��?��?�� ?��?���?�? ?��미하�?

		5�? 바이?��?�� 체크?�� ?��기�?? ?��미한?��


	*/


	//----- ?��기에 코드 추�?(비트 ?��?��?���? 분석?��?�� 바이?�� ?��?��?���? �?�?) -----
	//...DONE!

	uint8_t RHiValue = 0;
	uint8_t RHdValue = 0;

	uint8_t TiValue = 0;
	uint8_t TdValue = 0;

	uint8_t ChecksumPredict = 0;

	char initalNum = 2;
/*
	if((6 <= DHT11BitData[initalNum]) && (DHT11BitData[initalNum] <= 10)){
		//2번 읽기
		RHiValue += 1 << 7;
	}

*/
	for(int RHi = initalNum; RHi < initalNum+8; RHi++){
		//3~11비트
		//만약 0이라면
		//1~9
		if((11 <= DHT11BitData[RHi]) && (DHT11BitData[RHi] <= 15)){
			//?���비?�� ?��?�� 비트 값이 7~10 ?��?��?���? (1)

			RHiValue += 1 << (7 - (RHi - initalNum));
		}
		/*
		else if((2 <= BitDataIndex[RHi]) && (BitDataIndex[RHi] <= 5)){
			//?��? (0) ?���? ?��무것?�� ?��?���? ?��?���? ?��?��??
		}
		*/
	}

	for(int RHd = (initalNum+8); RHd < (initalNum+16); RHd++){
		if((11 <= DHT11BitData[RHd]) && (DHT11BitData[RHd] <= 15)){
			//?���비?�� ?��?�� 비트 값이 7~10 ?��?��?���? (1)

			RHdValue += 1 << (7 - (RHd - (initalNum+8)));
		}
	}

	for(int Ti = (initalNum+16); Ti < (initalNum+24); Ti++){
		if((11 <= DHT11BitData[Ti]) && (DHT11BitData[Ti] <= 15)){
			//?���비?�� ?��?�� 비트 값이 7~10 ?��?��?���? (1)

			TiValue += 1 << (7 - (Ti - (initalNum+16)));
		}
	}

	for(int Td = (initalNum+24); Td < (initalNum+32); Td++){
		if((11 <= DHT11BitData[Td]) && (DHT11BitData[Td] <= 15)){
			//?���비?�� ?��?�� 비트 값이 7~10 ?��?��?���? (1)

			TdValue += 1 << (7 - (Td - (initalNum+24)));
		}
	}









	ChecksumPredict = RHiValue + RHdValue + TiValue + TdValue;

	DHT11value[0] = RHiValue;
	DHT11value[1] = RHdValue;
	DHT11value[2] = TiValue;
	DHT11value[3] = TdValue;

	printf("---------BitDataIndex=%d--------\r\n",BitDataIndex); //?��비트 ?��?��?���? 출력?��?��.
	for(i = 0; i < BitDataIndex; i++){ //?��i?�� 0�??�� 비트 ?��?��?��만큼 반복?��?��
		//?��비트 ?��?��?��?�� ?��?��?�� ?��?��?��?���? 발생?�� ?�� 마다 ?��?��?��?��?��. 그러?���? 각각?�� DHT11BitData[n]?? �? ?��?��?�� �??��마다?�� 거리(?���?)�? ?��?��?��?��?��?�� 것이?��
		printf("%d,",DHT11BitData[i]); //?��비트 ?��?��?���? ?��?��?�� ?��기한?��(10진수)
		if((i%8) == 7){
			printf("H");
		}
	}
	printf("\r\n"); //?��마�?막에 줄바꿈을 ?��?��

	//----- ?��기에 코드 추�?(체크?�� 구하�? ?��?��?��?? ?��?��?���?) -----
	//...DONE!

	uint8_t ChSValue = 0;
	for(int ChS = (initalNum+32); ChS < (initalNum+40); ChS++){
		if((11 <= DHT11BitData[ChS]) && (DHT11BitData[ChS] <= 15)){
			//?���비?�� ?��?�� 비트 값이 7~10 ?��?��?���? (1)

			ChSValue += 1 << (7 - (ChS - (initalNum+32)));
		}
	}

	if(ChSValue == ChecksumPredict){
		//?���체?��?�� ?���?

		return 0;
	}else{
		//?���체?��?�� ?��?��
		return 1;
	}

	//return(0); //?��?��?�� 종료 코드 ?���? ?��?�� ?��?��?��
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	HAL_StatusTypeDef tim3it; //?��???���? ?��?��?�� status ?��?��
	//HAL_StatusTypeDef uart2state; //?��uart2 status. ?��?�� 비활?��?�� ?��?��?��?��?��
	int retvalue; //?��retvalue... ?��?��불명
	GPIO_PinState pc13pin; //?��13번�?. ?��??버튼?�� B13?��?��?�� 것으�? 추측

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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	printf("---stmf103 dht11 TEST---\r\n"); //?��목적 ?��명으�? 보임

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //?��B5?�� ???�� High�? 만듦.

	tim3it = HAL_TIM_Base_Start_IT(&htim3); //?��???��머�? ?��?��?��?���? ?��?��
	if(HAL_OK == tim3it){ //?��timer3 ... HAL_OK?���?...
		printf("HAL_TIM_Base_Start_IT=%d OK\r\n",tim3it); //?��ok ?��?��
	}else{
		printf("HAL_TIM_Base_Start_IT=%d error\r\n",tim3it); //?��error ?��?��
	}

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  pc13pin = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13); //✨C13번 핀을 읽는다. 이게 파란버튼인듯
	  if(GPIO_PIN_RESET == pc13pin){ //✨13pin이 RESET이라면... (눌린 경우일 수도 있을듯)
		  retvalue = DHT11_Data_Read(1); //✨DHT11 데이터 읽기 함수에 1을 보낸값의 반환값을 retvalue에 넣음
		  if(0 == retvalue){ //✨retvalue가 0이라면... 성공인듯
			  printf("Humirity=%d.%d / Temperature=%d.%d\r\n",DHT11value[0],DHT11value[1],DHT11value[2],DHT11value[3]);
		  }else{ //✨그 외 다른 값은 실패
			  printf("dht11 checksum error\r\n"); //✨체크섬 실패인듯
		  }
	  }

	  HAL_Delay(100); //✨100ms마다 감지
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //✨LED 깜빡이기

  }
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 64-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 LD2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
