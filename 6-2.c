/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stdio.h" //✨호출

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
#endif /* __GNUC__ */ //✨매일 보던거

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3; //✨이번엔 타이머 3를 쓴다

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t utx[16]; //✨보내는 버퍼
uint8_t urx[16]; //✨받는 버퍼

int tim3Cnt; //✨타이머에 카운터가 있다
int BitDataIndex; //✨비트 데이터를 인덱스에 넣는듯
uint8_t DHT11BitData[64]; //✨?
uint8_t DHT11value[8]; //✨?

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
	GPIO_InitTypeDef GPIO_InitStruct = {0}; //✨GPIO 구조체를 만들었다... struct 없이 선언이 되는듯
	
	/*Configure GPIO pin : PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5; //✨PIN5 지정
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //✨OUTPUT PP로 지정 (?)
	GPIO_InitStruct.Pull = GPIO_NOPULL; //✨no pull로 지정 pull up pull down 이런건듯
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; //✨Frequency low인듯
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //✨GPIOB에 GPIO_InitStruct라는 구조체를 넣는다... 아마 내 생각에 이렇게 하면 ioc를 건드리지 않고 핀설정을 할 수 있을 것으로 보인다
	
	//✨GPIOB5를 Output으로 쓰는듯...
}

void DHT11_GPIO_MODE_INPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; //✨구조체인듯

	/*Configure GPIO pin : PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5; //✨PIN5...
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; //✨Interupt Falling인듯
	GPIO_InitStruct.Pull = GPIO_NOPULL; //✨no pull
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //✨이번에도 B에 집어넣은 것으로 볼 때 이 함수는 B5를 입력모드로 만들고, 위는 출력 모드로 바꾸는 함수인듯

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); //✨NVIC 세팅인듯... External Interupt 9-5인듯... 우선순위 0 0 이런것으로 추정
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn); //✨IRQ 활성화하기
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{ //✨타이머 작동
	if(htim->Instance == htim3.Instance){ //✨타이머 3라면
		tim3Cnt += 1; //✨타이머 카운트를 1 늘린다... 타이머는 PSC 64-1, ARR 10-1로 10us마다 작동하는 것으로 추측한다. tim3Cnt가 24라면 240us가 지난것으로 보임
		// test pin
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); //✨A0핀을 토글한다... 왜?❓
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{ //✨인터럽트 발생시
	if(GPIO_Pin == GPIO_PIN_5){ //✨핀이 5번이라면... 위에서 B5를 핀으로 설정했다. 여기서 핀은 Single Wire - Two Way라서 핀 하나로 통신하는듯
		DHT11BitData[BitDataIndex++] = tim3Cnt; //✨DHT11데이터 비트 배열 (64까지 있다)를 main에서 0으로 선언한 데이터 인덱스에 타이머 카운트를 집어넣는다.
		//✨비트 인덱스는 데이터를 집어넣고 나서 1씩 커진다... 인터럽트가 발생할 때 마다 비트 인덱스가 커지는듯
		
		
		tim3Cnt = 0; //✨위에서 카운트를 집어넣은 길이를 넣고 0으로 반환한다... 아마 목적은 데이터의 길이 (시간)을 측정하려는 것으로 보임
		// test pin
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0); //✨C0핀의 값을 바꾼다 C0 핀은 용도가 뭘까❓
		
		
		
		/*
		
			✨ 이 인터럽트 함수에서는 pin5의 신호... 아마 B5에서
			인터럽트가 발생할 것으로 확신하고 있다.
		
		
		*/
	}
}

int DHT11_Data_Read(int sel)
{ //✨DHT11 데이터 읽기 함수... 무조건 sel은 1이다.
	int i,j,k; //✨ijk는 대표적인 반복문의 변수이다
	uint8_t checksum; //✨8바이트 체크섬...

	DHT11_GPIO_MODE_OUTPUT(); //✨GPIO를 Output으로 설정
	HAL_Delay(1); //✨1ms 대기
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //✨B5를 Low로...
	HAL_Delay(18); //✨18ms 대기
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //✨B5를 High로...
	
	
	
	/*
	
		✨위 코드는 시작 신호를 보내는 것으로 보인다
	
	
	
		💎추가 추측... 위에서 OUTPUT 모드로 바꾸는 함수를 써서 B5 핀을 통해 MCU->DHT로 시작 신호를 보낸다
	*/

	tim3Cnt = 0; //✨타이머 카운터를 0으로 만든다
	BitDataIndex = 0; //✨데이터 인덱스를 처음으로 옮긴다... 지금부터 발생하는 값은 타이머가 읽어들이는 것으로 보인다
	for(i = 0; i < 64; i++){
		DHT11BitData[i] = 0; //✨비트 데이터를 지운다 0부터 64의 인덱스를 다 지운다
	}
	for(i = 0; i < 8; i++){ //✨값을 지운다
		DHT11value[i] = 0; //✨값을 전부 0으로 만든다 (삭제)
	}

	DHT11_GPIO_MODE_INPUT(); //✨위와 다르게 입력 모드로 바뀐다

	HAL_Delay(500); // DHT11 received time //✨500ms 기다린다...
	
	/*
	
		✨추측
		
		위 코드는 현재까지 발생한 더러운 입력(시작 신호를 보낸적도 없는데 받은 입력. 왜냐하면 타이머는 시작 코드가 없어도 작동하기 때문에 더러운 입력이 들어왔을 것으로 추측된다)
		그런 입력을 전부 청소하고,
		
		가지고있던 데이터와 값도 전부 지운다.
		
		그 다음 B5핀을 입력모드로 바꾸고 500ms기다린다... 아무래도 DHT가 보내는건 타이머-인터럽트가 기록하고있기 떄문에 여기서는
		기다리기만 하는듯
	
	*/
	
	
	/*
	
		🐋풀이 추측
		
		데이터시트에서 길이가 70us인 High Level Signal은 1이라고 했다
		또한 길이가 26~28정도 되는 길이의 High Level Signal은 0이라고 했다
		
		
		이를 통해 위에서 측정한 (타이머는 10us마다 1을 체크한다) 타이머 데이터를 바탕으로
		n개의 DHT11BitData에는 측정된 길이가 들어있을 것이다.
		
		타이머는 10us단위로 측정하기 때문에
		
		2~5의 길이가 들어있는 값은 0이며, 7~10의 길이가 들어있는 값은 1이다.
		
		이를 통해 데이터를 정렬할 수 있고 n개의 DHT11BitData는 각각 실제 비트와 대응된다
	
	
	
		++🐋
		
		또한 데이터의 길이는 5바이트이다.
		왜냐하면 1번 바이트는 습도의 실수부, 2번 바이트는 습도의 소수부를 의미하며
		3번 바이트는 온도의 실수보, 4번 바이트는 온도의 소수부를 의미하고
		
		5번 바이트는 체크섬 크기를 의미한다
		
		
	*/


	//----- 여기에 코드 추가(비트 데이터를 분석하여 바이트 데이터로 변경) -----
	//...DONE!
	
	int RHiValue = 0;
	int RHdValue = 0;
	
	int TiValue = 0;
	int TdValue = 0;
	
	int ChecksumPredict = 0;
	
	
	for(int RHi = 0; RHi < 8; RHi++){
		
		if((7 <= BitDataIndex[RHi]) && (BitDataIndex[RHi] <= 10)){
			//🎈비트 해당 비트 값이 7~10 사이라면 (1)
			
			RHiValue += 1 << (7 - RHi);
		}
		/*
		else if((2 <= BitDataIndex[RHi]) && (BitDataIndex[RHi] <= 5)){
			//🎈 (0) 이거 아무것도 안하면 된느거 아님??
		}
		*/
	}
	
	for(int RHd = 8; RHd < 16; RHd++){
		if((7 <= BitDataIndex[RHd]) && (BitDataIndex[RHd] <= 10)){
			//🎈비트 해당 비트 값이 7~10 사이라면 (1)
			
			RHdValue += 1 << (7 - RHd);
		}
	}
	
	for(int Ti = 16; Ti < 24; Ti++){
		if((7 <= BitDataIndex[Ti]) && (BitDataIndex[Ti] <= 10)){
			//🎈비트 해당 비트 값이 7~10 사이라면 (1)
			
			TiValue += 1 << (7 - Ti);
		}
	}
	
	for(int Td = 24; Td < 32; Td++){
		if((7 <= BitDataIndex[Td]) && (BitDataIndex[Td] <= 10)){
			//🎈비트 해당 비트 값이 7~10 사이라면 (1)
			
			TdValue += 1 << (7 - Td);
		}
	}
	
	
	
	
	
	
	
	
	
	ChecksumPredict = RHiValue + RHdValue + TiValue + TdValue;
	
	DHT11value[0] = RHiValue;
	DHT11value[1] = RHdValue;
	DHT11value[2] = TiValue;
	DHT11value[3] = TdValue;

	printf("---------BitDataIndex=%d--------\r\n",BitDataIndex); //✨비트 인덱스를 출력한다.
	for(i = 0; i < BitDataIndex; i++){ //✨i는 0부터 비트 인덱스만큼 반복된다
		//✨비트 인덱스는 위에서 인터럽트가 발생할 때 마다 생성되었다. 그러니까 각각의 DHT11BitData[n]은 각 데이터 변화마다의 거리(시간)가 표시되어있을 것이다
		printf("%d,",DHT11BitData[i]); //✨비트 데이터를 하나씩 표기한다(10진수)
	}
	printf("\r\n"); //✨마지막에 줄바꿈을 한다

	//----- 여기에 코드 추가(체크섬 구하고 데이터와 확인하기) -----
	//...DONE!
	
	int ChSValue = 0;
	for(int ChS = 32; ChS < 40; ChS++){
		if((7 <= BitDataIndex[ChS]) && (BitDataIndex[ChS] <= 10)){
			//🎈비트 해당 비트 값이 7~10 사이라면 (1)
			
			ChSValue += 1 << (7 - ChS);
		}
	}
	
	if(ChSValue == ChecksumPredict){
		//🎈체크섬 성공
		
		return 0;
	}else{
		//🎈체크섬 실패
		return 1;
	}

	//return(0); //✨정상 종료 코드 아마 필요 없을듯
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	HAL_StatusTypeDef tim3it; //✨타이머 활성화 status 생성
	//HAL_StatusTypeDef uart2state; //✨uart2 status. 원래 비활성화 되어있었음
	int retvalue; //✨retvalue... 용도불명
	GPIO_PinState pc13pin; //✨13번핀. 파란버튼이 B13이었던 것으로 추측

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

	
	
	
	
	
	printf("---stmf103 dht11 TEST---\r\n"); //✨목적 설명으로 보임

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //✨B5에 핀을 High로 만듦.

	tim3it = HAL_TIM_Base_Start_IT(&htim3); //✨타이머가 실행됐는지 확인
	if(HAL_OK == tim3it){ //✨timer3 ... HAL_OK라면...
		printf("HAL_TIM_Base_Start_IT=%d OK\r\n",tim3it); //✨ok 표시
	}else{
		printf("HAL_TIM_Base_Start_IT=%d error\r\n",tim3it); //✨error 표시
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
