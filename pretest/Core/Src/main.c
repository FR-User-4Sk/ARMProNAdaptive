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
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

typedef struct ts_rxType{
	char start;
	unsigned char data;
	char parity;
	char stop;
	char error;
} rxType;





/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


//LED On,Off Variable

char pc13flag; //PC13 (Unused)
char pc7flag; //PC7
int LED_pa6_cnt,LED_pa6_Limit; //PA6 counter and Limits
int LED_pa7_cnt,LED_pa7_Limit; //PA7 counter and Limits



//





//US100 Sonic Sensor Variable






//TX VAR

char Uart_Tx_Parity_Mode;
int tim2_cnt;
char tim2_flag;
char tim2_trigger;
char chUartTxData[32];

//TX VAR




//RX VAR

char Uart_Rx_Parity_Mode;
rxType rxData[32]; //rxdata structure
int rxByte_cnt;
int rxBit_cnt;
int rxBit_value;
int rxSamp_cnt;
int rxParity_bit;
char Uart_Rx_flag;
char Uart_Rx_active;
char Uart_Rx_Temp;
char Uart_Rx_trigger;
char Uart_Rx_data;
char Uart_Rx_parity;
char Uart_Rx_stop;

//RX VAR


//global var


int globalCounter = 0;

int led5blinker = 0;

short actState = 0; //0 default, 1 distance 2 temp
short superActState = 0;

GPIO_PinState pa8pin, pb8pin;
GPIO_PinState pc13pin;

//








/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */


void Uart_Tx_Byte(char TxData, char Parity);
void parScan();
void doUS100Temp();
void doUS100Distance();
int Uart_Tx_multiByte(int Tx_cnt, char* TxData, char Parity);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}


//Button Event
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_13){
		//Button13 (Unused)
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		actState = 2;


	}


	if(GPIO_Pin == GPIO_PIN_7){ //Button7
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

		actState = 1;
	}

}

//Timer
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim2.Instance){ //timer2 for led blink

		led5blinker++;


		if(led5blinker >= 200){
			led5blinker = 0;

			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		}

	}//end timer 2

	if(htim->Instance == htim3.Instance){
		if(tim2_flag == 'y'){
			globalCounter = globalCounter + 1;

			if(globalCounter >= 4){
				tim2_cnt = tim2_cnt + 1;
				tim2_trigger = 'y';

				globalCounter = 0;
			}
		}

		if(Uart_Rx_flag == 'y'){
			if(Uart_Rx_active == 'n'){
				Uart_Rx_Temp <<= 1;
				if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_SET){
					Uart_Rx_Temp += 1;
				}
				if((Uart_Rx_Temp & 0x0f) == 0x0c){ //if data start
					Uart_Rx_active = 'y'; //make active yes
					Uart_Rx_Temp = 0x00; //rid Temp variable
					rxByte_cnt += 1; //Count one Byte
					rxData[rxByte_cnt].start = 'y'; //start yes
					rxData[rxByte_cnt].data = 0x00; //data 0x00
					rxData[rxByte_cnt].parity = 'n'; //parity n
					rxData[rxByte_cnt].stop = 'x'; //stop x
					rxBit_cnt = 0; //bit count 0
					rxSamp_cnt = 0; // Sample count 0
					rxParity_bit = 0; //parity 0
					Uart_Rx_trigger = 'n'; //trigger no
					Uart_Rx_data = 'n'; //data no
					Uart_Rx_parity = 'n'; //parity none
					Uart_Rx_stop = 'n'; //stop no
				}
			}else{ //if rx active yes
				Uart_Rx_trigger = 'n';
				rxSamp_cnt += 1;
				if((rxSamp_cnt % 4) == 0){
					//try 4 sampling
					Uart_Rx_trigger = 'y'; //and then trigger is "yes"
					if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)){
						rxBit_value = 1;
						//if value is true, Bitvalue set to 1
					}else{
						rxBit_value = 0;
						//if false, Bitvalue set to 0
					}
					rxBit_cnt += 1;
					//add bit counter
					if((1 <= rxBit_cnt) && (rxBit_cnt <= 8)){
						//bit counter is between 1 and 8
						Uart_Rx_data = 'y'; //make data "yes"
					}else if(9 == rxBit_cnt){
						//bit counter
						Uart_Rx_data = 'n'; //data no
						if('n' == Uart_Rx_Parity_Mode){
							//if parity "no"
							Uart_Rx_parity = 'n'; //parity no
							Uart_Rx_stop = 'y'; //stop yes
						}else if('e' == Uart_Rx_Parity_Mode){
							//if parity "even"
							Uart_Rx_parity = 'y'; //parity yes
						}else if('o' == Uart_Rx_Parity_Mode){
							//if parity "odd"
							Uart_Rx_parity = 'y'; //parity yes
						}
					}else if(10 == rxBit_cnt){
						Uart_Rx_parity = 'n'; //parity no
						Uart_Rx_stop = 'y'; //stop yes!
					}
				}
				if(('y' == Uart_Rx_trigger) && ('y' == Uart_Rx_data)){
					//if trigger yes, so if you were success sampling
					//and data is yes
					rxData[rxByte_cnt].data >>= 1; //push bit to right as one
					if(1 == rxBit_value){
						//if bitvalue is one
						rxData[rxByte_cnt].data += 0x80;
						//write data 1 in MSB
						rxParity_bit += 1;
						//count parity bit
					}
				}
				if(('y' == Uart_Rx_trigger) && ('y' == Uart_Rx_parity)){
					//if sampling completed and parity is yes
					if('e' == Uart_Rx_Parity_Mode){
						//if parity e
						if(((rxParity_bit % 2) == 0) && (0 == rxBit_value)){
							//if bit is even, but any bit is not left
							rxData[rxByte_cnt].parity = 'e'; //set to 'e'
						}else if(((rxParity_bit % 2) == 1) && (1 == rxBit_value)){
							//if bit is odd and bit is still left
							rxData[rxByte_cnt].parity = 'e';//set to 'e'
						}else{
							rxData[rxByte_cnt].parity = 'f'; //else 'f'
						}
					}else if('o' == Uart_Rx_Parity_Mode){
						//if parity o
						if(((rxParity_bit % 2) == 0) && (1 == rxBit_value)){
							//if bit is even, and bit is still left
							rxData[rxByte_cnt].parity = 'o'; //set to 'o'

						}else if(((rxParity_bit % 2) == 1) && (0 == rxBit_value)){
							//if bit is odd, but any bit is not left;
							rxData[rxByte_cnt].parity = 'o'; //set to 'o'
						}else{
							rxData[rxByte_cnt].parity = 'f'; //else 'f'
						}
					}
				}
				if(('y' == Uart_Rx_trigger) && ('y' == Uart_Rx_stop)){

					Uart_Rx_active = 'n';
					//active set to n
					if(1 == rxBit_value){
						//if bitvalue is one
						rxData[rxByte_cnt].stop = 'y';
						//stop
					}else{
						rxData[rxByte_cnt].stop = 'n';
						//dont stop
					}
				}

			}

		}

	}

}




//loop code for check system is working



void parScan(){
	pa8pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
	pb8pin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);

	if((GPIO_PIN_SET == pa8pin) && (GPIO_PIN_RESET == pb8pin)){
	  Uart_Tx_Parity_Mode = 'e';
	  Uart_Rx_Parity_Mode = 'e';
	}else if((GPIO_PIN_RESET == pa8pin) && (GPIO_PIN_SET == pb8pin)){
	  Uart_Tx_Parity_Mode = 'o';
	  Uart_Rx_Parity_Mode = 'o';
	}else{
	  Uart_Tx_Parity_Mode = 'n';
	  Uart_Rx_Parity_Mode = 'n';
	}
}




void doUS100Distance(){
	Uart_Tx_Byte(0x55, 'n');
	printf("check distance\r\n");

	HAL_Delay(200);

	if(('n' == Uart_Rx_active) && (1 <= rxByte_cnt)){
		int resultDistance = (rxData[1].data << 8) + rxData[2].data;

		printf("distance : %d mm\r\n", resultDistance);

		rxByte_cnt = 0;
	}
}


//us100 4 temp

void doUS100Temp(){
	Uart_Tx_Byte(0x50, 'n');

	printf("check temp\r\n");

	HAL_Delay(200);

	if(('n' == Uart_Rx_active) && (1 <= rxByte_cnt)){
		printf("temp : %d\r\n", rxData[1].data - 45);

		rxByte_cnt = 0;
	}
}





//original TX_BYTE code
void Uart_Tx_Byte(char TxData, char Parity)
{
	char chExit = 'n';
	char chComp = 0x01;
	int ParityBit_cnt = 0;

	tim2_cnt = 0;
	tim2_flag = 'y';
	tim2_trigger = 'n';

	while(1){
		if(('y' == tim2_trigger) && (tim2_cnt == 1)){ // idle
			tim2_trigger = 'n';
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
		}else if(('y' == tim2_trigger) && (tim2_cnt == 2)){ // start
			tim2_trigger = 'n';
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		}else if(('y' == tim2_trigger) && (tim2_cnt == 3)){ // bit 0
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 4)){ // bit 1
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 5)){ // bit 2
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 6)){ // bit 3
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 7)){ // bit 4
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 8)){ // bit 5
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 9)){ // bit 6
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}else if(('y' == tim2_trigger) && (tim2_cnt == 10)){ // bit 7
			tim2_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
		}
		else if(('y' == tim2_trigger) && (tim2_cnt == 11)){ // [parity : none/even/odd] or [stop]
			tim2_trigger = 'n';

			if(Parity == 'n'){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				chExit = 'y';
			}else if(Parity == 'e'){
				if((ParityBit_cnt % 2) == 0){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
				}else{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				}

			}else if(Parity == 'o'){
				if((ParityBit_cnt % 2) == 1){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
				}else{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				}
			}

		}else if(('y' == tim2_trigger) && (tim2_cnt == 12)){ // stop

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
			chExit = 'y';

		}
		if(('y' == tim2_trigger) && ('y' == chExit)){
			tim2_trigger = 'n';
			tim2_flag = 'n';
			break;
		}
	}
}
int Uart_Tx_multiByte(int Tx_cnt, char* TxData, char Parity)
{
	int m;

	for(m = 0; m < Tx_cnt; m++){
		Uart_Tx_Byte(TxData[m], Parity);
	}

	return(0);
}





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  HAL_StatusTypeDef tim2_it_start; //variable for check timer interupt status

  tim2_it_start = HAL_TIM_Base_Start_IT(&htim2); //read interupt status

  if(tim2_it_start == HAL_OK){
	  printf("Timer2 OK\r\n");
  }else{
	  printf("Timer2 ERROR!\r\n");
  }



  //init setting


  pc13flag = 'n'; //unused
  pc7flag = 'n';

  LED_pa6_cnt = 0;
  LED_pa6_Limit = SLOW6;
  LED_pa7_cnt = 0;
  LED_pa7_Limit = SLOW7;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);




  //US100 Sensor Code

  HAL_StatusTypeDef tim3_it_start;

  tim3_it_start = HAL_TIM_Base_Start_IT(&htim3);

  if(tim3_it_start == HAL_OK){
	  printf("Timer3 OK\r\n");
  }else{
	  printf("Timer3 ERROR!\r\n");
  }





  Uart_Rx_Parity_Mode = 'n'; // 'n':none, 'e':even, 'o':odd
  rxByte_cnt = 0;
  rxBit_cnt = 0;
  rxSamp_cnt = 0;
  Uart_Rx_flag = 'n';
  Uart_Rx_active = 'n';
  Uart_Rx_Temp = 0x00;

  Uart_Tx_Parity_Mode = 'n'; // 'n':none, 'e':even, 'o':odd
  tim2_cnt = 0;
  tim2_flag = 'n';
  tim2_trigger = 'n';
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);


  Uart_Rx_flag = 'y';
  Uart_Rx_active = 'n';






  chUartTxData[0] = 'a';

  actState = 0; //reset state
	superActState = 0;
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */


  printf("executed\r\n");

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  parScan();


	  if(actState == 2){
		  doUS100Temp();
	  }else if(actState == 1){
		  doUS100Distance();
	  }
	  
	  
	  
	  
	  if(superActState = 0){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	  }else if(superActState == 1){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	  }else if(superActState == 2){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	  }

	  actState = 0;

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
  htim3.Init.Period = 26-1;
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
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|LD2_Pin|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : B1_Pin PC7 */
  GPIO_InitStruct.Pin = B1_Pin|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 LD2_Pin PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|LD2_Pin|GPIO_PIN_6|GPIO_PIN_7;
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
