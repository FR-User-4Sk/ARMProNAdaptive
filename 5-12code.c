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

//✨printf 함수가 작동하기 위한 프로토타입


#define MYZERO = GPIO_PIN_RESET
#define MYONE = GPIO_PIN_SET


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4; //✨타이머 4 사용... 왜지?

UART_HandleTypeDef huart2; //✨UART2 사용. PCOM을 위해서인듯

/* USER CODE BEGIN PV */
uint8_t utx[16]; //✨tx는 16개의 데이터 배열
uint8_t urx[16]; //✨rx도 16개의 데이터 배열

char SPI_TICK_1 = 'n'; //✨1은 아래로 평평한 값
char SPI_TICK_2 = 'n'; //✨2는 rising edge
char SPI_TICK_3 = 'n'; //✨3은 위로 평평한 값
char SPI_TICK_4 = 'n'; //✨4는 falling edge
char SPI_CS_HIGH = 'n'; //✨Chip Select가 High인가요?... 아니오
char SPI_CS_LOW = 'n'; //✨Chip Select가 Low인가요?... 아니오
char SPI_CLK_HIGH = 'n'; //✨CLK가 High인가요?... 아니오
char SPI_CLK_LOW = 'n'; //✨CLK가 Low인가요?... 아니오
char SPI_DI_LOAD = 'n'; //✨DI_LOAD... 아니오
char SPI_DO_CHECK = 'n'; //✨DO 확인... 아니오
char SPI_BREAK = 'n'; //✨SPI BREAK... 아니오


int tim4Cnt; //✨타이머 카운터(인트)
int SPI_on = 0; //✨SPI - ON = 0 (0이 무슨 의미일까?)
int SPI_BitCnt = 0; //✨BPI 비트 카운터 = 0... 아마 아마 ORG 따라 비트 갯수가 다른 걸 표현한 걸지도?... 모르겠다.

char pc13flag; //✨pc13flag... STM의 파란 버튼을 말한다

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM4_Init(void);

//✨시작 함수. 별 의미는 없다.

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
} //✨printf를 사용시 uart2로 데이터를 내보내는 프로토타입

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//✨ 타이머 작동 코드
	if(htim->Instance == htim4.Instance){
		//✨💬작동한 타이머가 타이머 4라면
		// test pin
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1); // test
		//✨A1을 토글한다.
		if(1 == SPI_on){
			//✨💬SPI_on이 1이라면... 그런데 초기값 SPI_on은 0이다. 다른 부분에서 SPI_on이 제어되고 있다.
			tim4Cnt += 1;//✨타이머 카운터를 1 더한다
			if((tim4Cnt % 4) == 1){
				//✨💬tim counter ... 1이라면...
				SPI_TICK_1 = 'y'; //✨아래로 평평한 CLK
			}
			if((tim4Cnt % 4) == 2){
				//✨💬 카운터가 2라면
				SPI_TICK_2 = 'y'; //✨상승 엣지
			}
			if((tim4Cnt % 4) == 3){
				//✨💬 카운터가 3이라면
				SPI_TICK_3 = 'y'; //✨ 위로 평평한 엣지
			}
			if((tim4Cnt % 4) == 0){
				//✨💬 카운터가 4라면
				SPI_TICK_4 = 'y'; //✨하강 엣지
				tim4Cnt = 0; //✨타이머 카운터 초기화... 다시 0부터 세자
			}
		}
	}
} //✨ 🥩 전체적인 추론 결과 타이머는 CLK를 만들기만 할 뿐 다른 효과는 없는듯 하다


int setCS(int PinState){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, PinState);
	return 0;
}
int setCLK(int PinState){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, PinState);
	return 0;
}
int setDI(int PinState){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, PinState);
	return 0;
}



int spi_93c56c_EWEN(void)
{
	//✨SPI-93C56C의 E/W -ENable 코드... 위에 프로토타입이 선언되지 않았음으로 아래에서 호출하는 것으로 보임
	int SPI_DI_OUTPUT = 0x0980; //✨SPI-DI에 0x0980을 보냄. 원래 Slave의 DI는 마스터의 출력임 (반대여서)
	int SPI_Data_Cnt = 12; //✨데이터 카운트는 12... 위의 비트 카운트와 다르게 스코프 내부에서 작동하는 코드임
	int SPI_Data_Comp = 0x0800; //✨데이터 Compare? 인듯. 0x0800이 값

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // CS
	//✨A9의 전원을 내림. CS가 A9인듯
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // CLK
	//✨A10의 전원을 내림. CLK가 A10
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); // DI
	//✨B8의 전원을 내림. DI가 B8인듯
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); // DO
	//✨해당 코드는 원래 주석처리로 DO핀을 의미하는듯. ⏰ 그런데 Slave의 Output은 Master가 무조건 읽어야 하는데 왜 핀을 씀???
	
	
	//✨ 🥩 위의 전체적인 코드는 설명서에서 보았던 CLK와 CS, DI가 전부 1일 경우, Start Condition이라는 말과 같은 것으로 보인다.

	SPI_TICK_1 = 'n'; //✨아래로 평평한가요?... 아니오
	SPI_TICK_2 = 'n'; //✨상승 엣지인가요?... 아니오
	SPI_TICK_3 = 'n'; //✨위로 평평한가요?... 아니오
	SPI_TICK_4 = 'n'; //✨하강 엣지인가요?... 아니오
	SPI_CS_HIGH = 'n'; //✨CS가 High인가요?... 아니오
	SPI_CS_LOW = 'n'; //✨CS가 Low인가요?... 아니오
	SPI_CLK_HIGH = 'n'; //✨CLK가 High인가요?... 아니오
	SPI_CLK_LOW = 'n'; //✨CLK가 Low인가요?... 아니오
	SPI_DI_LOAD = 'n'; //✨DI Load인가요?... 아니오
	SPI_DO_CHECK = 'n'; //✨DO 확인인가요?... 아니오
	SPI_BREAK = 'n'; //✨SPI가 break되었나요?... 아니오

	SPI_BitCnt = 0; //✨전역 비트 카운트는 0
	tim4Cnt = 0; //✨타이머 카운트도 0. (CLK가 초기화 된 것으로 보임)
	SPI_on = 1; //✨SPI가 켜짐. 위에서 타이머 옵션이 작동 할 것으로 보임. 타이머는 SPI-on == 1에서만 💎의미있는 작동을 함
	
	
	/*
		✨ 위 코드 해석. Start Condition을 주겠다. (⏰근데 RESET이라서 스타트 컨티션을 안 주는 걸 수 도 있을듯)
		✨ 모든 값을 초기화하겠다.
		✨ SPI는 시작해라.
	*/

	while(1){
		//✨while은 영원히 실행됨 (가장 아래의 spi break에서 break 코드를 만날 때 까지.)
		
		//✨여기서 물어보는 SPI_TICK_1..2..3..4는 타이머에서 계속 바뀌고 있다.
		//✨추가로 알아낸건데, 타이머는 시작하자마자 1을 더하고 TICK을 체크하기 때문에 시작하길 원하는 값에서 1을 빼주어야 한다.
		//✨0일시 첫 CLK(아래로 평평)이 됨
		if('y' == SPI_TICK_1){//🎨
			//✨💬타이머가 TICK-1을 yes로 만들음
			SPI_TICK_1 = 'n'; //✨다시 SPI_TICK_1을 n으로 만들음. 그러고보니 ⏰ 타이머는 다 yes로 만들고 도망만 감. 뒤처리는 여기서 하는듯
			SPI_BitCnt += 1; //✨ 비트 카운터가 1 올라갔다.
			if(1 == SPI_BitCnt){
				//✨💬비트 카운터가 1이라면...
				SPI_CS_HIGH = 'y'; //✨CS High가 yes가 된다.
			}
			if((SPI_Data_Cnt + 1) == SPI_BitCnt){ //💛
				//✨💬SPI-data-cnt는 내부 변수로 여기서의 값은 13. BitCnt가 13이 될 때 까지 작동하지 않는 코드
				SPI_CS_LOW = 'y'; //✨CS Low가 yes가 된다
				SPI_BREAK = 'y'; //✨SPI break가 yes가 된다
			}
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨💬 비트 카운터가 1보다 크거나 같고... 아마 시작부터 크거나 같을듯. 시작 조건이 0이고, 코드가 시작하자마자 1이 더해지니까.
				//✨💬 게다가... 비트 카운트가 데이터 카운트보다 작거나 같다면... (💛)가 되기 전까지 계속 성립하는 코드
				SPI_DI_LOAD = 'y'; //✨DI Load는 Yes!
			}
		}//🎨
		if('y' == SPI_TICK_2){
			//✨💬TICK2가 YES라면?
			SPI_TICK_2 = 'n'; //✨TICK2는 no
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨💬비트 카운터가 1보다 크거나 같고... 비트 카운터가 데이터 카운터보다 작거나 같다면?
				SPI_CLK_HIGH = 'y'; //✨CLK는 high!
			}
		}
		if('y' == SPI_TICK_3){
			//✨💬 TICK-3가 yes라면?
			SPI_TICK_3 = 'n'; //✨TICK3는 no (위로 평평은 아무것도 없네)
		}
		if('y' == SPI_TICK_4){
			//✨💬TICK-4가 yes라면?
			SPI_TICK_4 = 'n'; //✨TICK4는 no
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨💬위에도 비슷한거 엄청 많음.
				//✨💬비트 카운터가 1보다 크고... 게다가 비트 카운터가 데이터 카운터보다 작거나 같다면...
				SPI_CLK_LOW = 'y'; //✨CLK - Low는 Yes!
			}
		}

		if('y' == SPI_CS_HIGH){
			//✨💬CS High인가요?... Yes!
			SPI_CS_HIGH = 'n'; //✨cs high는 no
			//--- 코드 추가 --- // CS:1
			
			setCS(MYONE); //✨🌿내코드!
			
		}else if('y' == SPI_CS_LOW){
			//✨💬CS Low인가요?... yes
			SPI_CS_LOW = 'n'; //✨ cs low는 no
			//--- 코드 추가 --- // CS:0
			setCS(MYZERO); //✨🌿내코드!
		}

		if('y' == SPI_CLK_HIGH){
			//✨CLK High인가요?... yes
			SPI_CLK_HIGH = 'n'; //✨CLK High는 no
			//--- 코드 추가 --- // CLK:1
			setCLK(MYONE); //✨🌿내코드!
		}else if('y' == SPI_CLK_LOW){
			//✨CLK Low인가요?... yes
			SPI_CLK_LOW = 'n'; //✨CLK Low는 no
			setCLK(MYZERO); //✨🌿내코드!
			//--- 코드 추가 --- // CLK:0
		}

		if('y' == SPI_DI_LOAD){
			//✨DI Load가 yes라면...
			SPI_DI_LOAD = 'n'; //✨DI Load는 no
			if((SPI_DI_OUTPUT & SPI_Data_Comp) == SPI_Data_Comp){
				//✨만약에 DI_Output과 Data 비교의 비트 AND가 Data Comp와 같다면...
				
				setDI(MYONE); //✨🌿내코드!
				
				//--- 코드 추가 --- // DI:1
			}else{
				//✨같지 않다면
				setDI(MYZERO); //✨🌿내코드!
				//--- 코드 추가 --- // DI:0
			}
			
			
			SPI_Data_Comp >>= 1;//✨데이터 비교 변수를 오른쪽으로 한 칸 밀자
		}

		if('y' == SPI_BREAK){
			//✨spi break가 yes라면?
			SPI_BREAK = 'n'; //✨spi break는 no.
			SPI_on = 0; //✨spi _on은 0이 된다.
			break; //✨while 종료
		}
	}

	return(0); //✨EWEN을 종료
}

int spi_93c56c_WRITE(int SPI_ADDR, int SPI_DATA_WRITE)
{
	//✨WRITE는 주소와 데이터를 파라메터로 받는다.
	int SPI_DI_OUTPUT = 0x0A0000 + (SPI_ADDR << 8) + SPI_DATA_WRITE;
	//✨데이터 아웃풋(DI로 보낼 것)은 0x0A0000에 (주소를 왼쪽으로 8번 시프트한 것) + 데이터가 된다.
	int SPI_Data_Cnt = 20; //✨데이터 카운트는 20. 이번엔 좀 길다
	int SPI_Data_Comp = 0x80000; //✨비교 데이터는 0x80000

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // CS ✨CS RESET
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // CLK ✨CLK RESET
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); // DI ✨DI RESET
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); // DO
	
	//✨위와 같아보임. 시작전 초기화

	SPI_TICK_1 = 'n'; //✨아래로 평평한가요?... 아니오
	SPI_TICK_2 = 'n'; //✨상승 엣지인가요?... 아니오
	SPI_TICK_3 = 'n'; //✨위로 평평한가요?... 아니오
	SPI_TICK_4 = 'n'; //✨하강 엣지인가요?... 아니오
	SPI_CS_HIGH = 'n'; //✨CS가 High인가요?... 아니오
	SPI_CS_LOW = 'n'; //✨CS가 Low인가요?... 아니오
	SPI_CLK_HIGH = 'n'; //✨CLK가 High인가요?... 아니오
	SPI_CLK_LOW = 'n'; //✨CLK가 Low인가요?... 아니오
	SPI_DI_LOAD = 'n'; //✨DI Load인가요?... 아니오
	SPI_DO_CHECK = 'n'; //✨DO 확인인가요?... 아니오
	SPI_BREAK = 'n'; //✨SPI가 break되었나요?... 아니오

	SPI_BitCnt = 0; //✨비트카운트도 0부터
	tim4Cnt = 0; //✨카운트는 0부터
	SPI_on = 1; //✨SPI가 on!

	while(1){
		if('y' == SPI_TICK_1){
			//✨💬tick1 yes라면
			SPI_TICK_1 = 'n'; //✨tick1 no
			SPI_BitCnt += 1; //✨비트 카운터 1 증가
			if(1 == SPI_BitCnt){ //✨비트 카운터가 1이라면
				SPI_CS_HIGH = 'y'; //✨ cs high가 yes!
			}
			if((SPI_Data_Cnt + 1) == SPI_BitCnt){
				//✨💬데이터 카운트+1 (여기서는 21)이 비트 카운터와 같다면
				SPI_CS_LOW = 'y'; //✨ cs low 가 yes가 된다
				SPI_BREAK = 'y'; //✨ spi break또한 yes (종료하는듯)
			}
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨💬비트 카운터가 1보다 크고... 게다가 비트 카운터가 데이터 카운터보다 작거나 같을 때
				SPI_DI_LOAD = 'y'; //DI Load가 yes가 됨
			}
		}
		if('y' == SPI_TICK_2){
			//✨💬tick2가 yes라면
			SPI_TICK_2 = 'n'; //✨tick2를 no로 설정
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨💬만약 비트 카운터가 1보다 크고... 게다가 비트 카운터가 데이터 카운터보다 작거나 같을 때
				SPI_CLK_HIGH = 'y'; //✨CLK High는 yes!
				
			}
		}
		if('y' == SPI_TICK_3){
			//✨💬tick3가 yes라면
			SPI_TICK_3 = 'n'; //✨tick3는 no입니다. 이 이상 작업은 없음.
		}
		if('y' == SPI_TICK_4){
			//✨💬tick4가 yes라면
			SPI_TICK_4 = 'n'; //✨tick4는 no
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨💬비트 카운터가 1보다 크거나... 게다가 비트 카운터가 데이터 카운터보다 작거나 같다면
				SPI_CLK_LOW = 'y'; //✨CLK Low는 yes...
			}
		}

		if('y' == SPI_CS_HIGH){ //✨💬CS High yes라면
			SPI_CS_HIGH = 'n'; //✨CS High no로 설정
			setCS(MYONE); //✨🌿내코드!
			//--- 코드 추가 --- // CS:1
		}else if('y' == SPI_CS_LOW){
			//✨💬CS Low가 yes라면
			SPI_CS_LOW = 'n';
			//✨CS Low는 no
			
			setCS(MYZERO); //✨🌿내코드!
			
			//--- 코드 추가 --- // CS:0
		}

		if('y' == SPI_CLK_HIGH){
			//✨💬CLK High가 Yes라면
			SPI_CLK_HIGH = 'n'; //✨CLK high는 no
			
			setCLK(MYONE); //✨🌿내코드!
			
			//--- 코드 추가 --- // CLK:1
		}else if('y' == SPI_CLK_LOW){
			//✨💬CLK Low가 yes라면
			SPI_CLK_LOW = 'n'; //CLK low는 no
			
			setCLK(MYZERO); //✨🌿내코드!
			
			//--- 코드 추가 --- // CLK:0
		}

		if('y' == SPI_DI_LOAD){
			//✨💬DI - Load가 yes라면
			SPI_DI_LOAD = 'n'; //✨DI Load는 no
			if((SPI_DI_OUTPUT & SPI_Data_Comp) == SPI_Data_Comp){
				//✨💬DI Output과 DataComp의 비트 AND가 DataComp와 같다면
				
				
				setDI(MYONE); //✨🌿내코드!
				
				
				
				//--- 코드 추가 --- // DI:1
			}else{
				//✨💬아니라면
				
				setDI(MYZERO); //✨🌿내코드!
				
				//--- 코드 추가 --- // DI:0
			}
			SPI_Data_Comp >>= 1; //✨데이터 비교기를 오른쪽으로 한 비트 밀음
		}

		if('y' == SPI_BREAK){
			//✨💬SPI break가 yes라면
			SPI_BREAK = 'n'; //✨spi break = no가 된다.
			SPI_on = 0; //✨spi on도 0. spi 꺼짐
			break; //✨while 탈출
		}
	}

	return(0); //정상적으로 종료시 0 반환
}

int spi_93c56c_READ(int SPI_ADDR, int* SPI_DATA_READ)
{
	int SPI_DI_OUTPUT = 0x0C00 + SPI_ADDR; //✨DI로 보내는 값은... 0x0C00 + 주소값을 더한 것
	int SPI_DO_INPUT = 0; //✨DO에서 받는 값은... 0
	int SPI_Data_Cnt = 12; //✨데이터 카운트는 12
	int SPI_Data_Comp = 0x800; //✨ 데이터 비교 - 0x800
	int SPI_Data_IN = 8; //✨데이터 받기 = 8 왜지?
	
	
	int iwishallbefine = 0;

	GPIO_PinState pb9pin; //✨b9핀 상태 변수가 생성되었다

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // CS ✨CS RESET
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // CLK ✨CLK RESET
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); // DI ✨DI RESET
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); // DO
	
	//✨위와 같아보임. 시작전 초기화

	SPI_TICK_1 = 'n'; //✨아래로 평평한가요?... 아니오
	SPI_TICK_2 = 'n'; //✨상승 엣지인가요?... 아니오
	SPI_TICK_3 = 'n'; //✨위로 평평한가요?... 아니오
	SPI_TICK_4 = 'n'; //✨하강 엣지인가요?... 아니오
	SPI_CS_HIGH = 'n'; //✨CS가 High인가요?... 아니오
	SPI_CS_LOW = 'n'; //✨CS가 Low인가요?... 아니오
	SPI_CLK_HIGH = 'n'; //✨CLK가 High인가요?... 아니오
	SPI_CLK_LOW = 'n'; //✨CLK가 Low인가요?... 아니오
	SPI_DI_LOAD = 'n'; //✨DI Load인가요?... 아니오
	SPI_DO_CHECK = 'n'; //✨DO 확인인가요?... 아니오
	SPI_BREAK = 'n'; //✨SPI가 break되었나요?... 아니오

	SPI_BitCnt = 0; //✨비트카운트도 0부터
	tim4Cnt = 0; //✨카운트는 0부터
	SPI_on = 1; //✨SPI가 on!

	while(1){
		//✨break 전까지 작동
		if('y' == SPI_TICK_1){
			//✨Tick1이 yes라면
			SPI_TICK_1 = 'n'; //✨tick 1 no로 설정
			SPI_BitCnt += 1; //✨비트 카운터 1증가 (맨날 똑같네)
			if(1 == SPI_BitCnt){
				//✨비트 카운터가 1이라면
				SPI_CS_HIGH = 'y'; //✨CS high가 yes가 됨
			}
			if((SPI_Data_Cnt + SPI_Data_IN + 1) == SPI_BitCnt){
				//✨데이터 카운터 + 데이터 입력 + 1 ..21이 비트 카운터와 같다면
				SPI_CS_LOW = 'y'; //✨CS low는 yes
				SPI_BREAK = 'y'; //✨spi는 break. 꺼지는듯
			}
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= SPI_Data_Cnt)){
				//✨비트 카운터가 1보다 크거나 같고... 게다가 비트 카운터가 데이터 카운터보다 작거나 같다면
				SPI_DI_LOAD = 'y'; //✨DI Load는 yes
			}
		}
		if('y' == SPI_TICK_2){
			//✨Tick2이 yes라면
			SPI_TICK_2 = 'n'; //✨tick2 no로 설정
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= (SPI_Data_Cnt + SPI_Data_IN))){
				//✨비트 카운터가 1보다 크거나 같고... 게다가 비트 카운터가 데이터 카운터 + 데이터 입력(여기선 20)보다 작거나 같다면
				SPI_CLK_HIGH = 'y'; //✨CLK High가 yes가 된다
			}
		}
		if('y' == SPI_TICK_3){
			//✨ tick3가 yes라면
			SPI_TICK_3 = 'n'; //✨tick3는 no. 얘는 맨날 작업이 없네
		}
		if('y' == SPI_TICK_4){
			//✨tick4가 yes라면
			SPI_TICK_4 = 'n'; //✨tick4는 no.
			if((1 <= SPI_BitCnt) && (SPI_BitCnt <= (SPI_Data_Cnt + SPI_Data_IN))){
				//✨비트 카운터가 1보다 크거나 같고... 게다가 비트 카운터가 데이터 카운터+데이터 입력(여기선 20)보다 작거나 같다면
				SPI_CLK_LOW = 'y'; //✨CLK Low는 yes
			}
			if((SPI_Data_Cnt < SPI_BitCnt) && (SPI_BitCnt <= (SPI_Data_Cnt + SPI_Data_IN))){
				//✨데이터 카운터가 비트 카운터보다 작고, 비트 카운터가 (데이터 카운터+데이터 입력 (여기선 20)) 보다 작거나 같다면
				SPI_DO_CHECK = 'y'; //✨Do Check가 yes가 된다 (🧡)
				
				/*
				
					✨이 코드는 현재 읽은 갯수가 데이터 카운터보다 많고, (bitcnt는 현재까지 읽은 갯수기 때문)
					
					DataCnt는 고정된 길이다.
					그러니까... 데이터 비트가 데이터 카운트를 넘어섰지만, 데이터 카운트+데이터 입력크기를 넘어서진 못했다면 이 코드가 호출된다
				
				*/
			}
		}

		if('y' == SPI_CS_HIGH){
			//✨CS High가 Yes라면
			SPI_CS_HIGH = 'n'; //✨cs high는 no가 된다
			setCS(MYONE);
			//--- 코드 추가 --- // CS:1 
		}else if('y' == SPI_CS_LOW){
			//✨cs low가 yes라면 
			SPI_CS_LOW = 'n'; //✨cs low는 no가 된다
			setCS(MYZERO);
			//--- 코드 추가 --- // CS:0
		}

		if('y' == SPI_CLK_HIGH){
			//✨clk high가 yes라면
			SPI_CLK_HIGH = 'n'; //✨clk high는 no
			setCLK(MYONE);
			//--- 코드 추가 --- // CLK:1
		}else if('y' == SPI_CLK_LOW){
			//✨clk low가 yes라면
			SPI_CLK_LOW = 'n'; //✨clk low는 no
			setCLK(MYZERO);
			//--- 코드 추가 --- // CLK:0
		}

		if('y' == SPI_DI_LOAD){
			//✨di load가 yes라면
			SPI_DI_LOAD = 'n'; //✨di load = no
			if((SPI_DI_OUTPUT & SPI_Data_Comp) == SPI_Data_Comp){
				//✨데이터 보냄과 데이터 비교의 비트 AND가 데이터 비교와 같다면...
				setDI(MYONE); //✨내코드!
				//--- 코드 추가 --- // DI:1
			}else{
				setDI(MYZERO); //✨내코드!
				//--- 코드 추가 --- // DI:0
			}
			//✨데이터 비교를 오른쪽으로 한 비트 민다
			SPI_Data_Comp >>= 1;
		}

		if('y' == SPI_DO_CHECK){
			//✨ SPI Do check가 yes라면... 🧡에서 확인
			SPI_DO_CHECK = 'n';
			
			
			SPI_DO_INPUT = SPI_DO_INPUT + (1 << iwishallbefine);
			
			iwishallbefine++;
			
			//--- 코드 추가 ---

			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1); // test
			//✨C1핀을 토글함. 이게 뭐지?
		}

		if('y' == SPI_BREAK){
			//✨spi break가 yes라면.
			SPI_BREAK = 'n'; //✨spi break를 no로 설정
			SPI_on = 0; //✨spi on은 0... 꺼짐
			break; //✨while 종료
		}
	}

	*SPI_DATA_READ = SPI_DO_INPUT; //✨Data Read 포인터에 Do Input 값을 넣는다

	return(0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//✨NVIC으로 설정된 인터럽트가 발생시 이 함수를 호출함
	if(GPIO_Pin == GPIO_PIN_13){
		//✨GPIO_pin이 13이라면
		if('n' == pc13flag){ //✨그런데 13flag가 no라면?...
			
			pc13flag = 'y'; //✨13flag는 yes가 됨
			printf("-----pc13flag = %c-----\r\n",pc13flag); //✨13 flag는 (13flag의 값)입니다!를 전송함
			
			//✨앞으로 영원히 13flag는 yes일 것임...
		}
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

	HAL_StatusTypeDef tim4it; //✨tim4 시작했는지 변수
	HAL_StatusTypeDef uart2state; //✨uart2 상태 변수
	int retvalue; //✨??? 이건 뭘까
	//GPIO_PinState pc13pin;
	int spi_read; //✨spi 읽은 값 변수. 비어있다

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init(); //✨HAL 시작

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config(); //✨클럭 컨피그 설정

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

	printf("---93c56c spi(gpio) TEST---\r\n"); //✨SPI 테스트... 그냥 코드 정상 작동시 작동

	tim4it = HAL_TIM_Base_Start_IT(&htim4); //✨타이머4를 시작하고 반환값을 tim4it에 넣음
	if(HAL_OK == tim4it){
		//✨tim4it이 HAL_OK라면?...
		printf("HAL_TIM_Base_Start_IT=%d OK\r\n",tim4it); //✨타이머 OK!
	}else{
		printf("HAL_TIM_Base_Start_IT=%d error\r\n",tim4it); //✨타이머 error.
	}

	SPI_on = 0; //✨spi-on은 0이다 (spi가 켜졌는지 확인하는 변수)
	tim4Cnt = 0; //✨tim4Cnt는 0이다. (CLK을 만드는 변수)

	pc13flag = 'n';//✨ 초기 13flag는 no입니다

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		//✨while은 영원히 반복됩니다... 브레이크 없음
		
	  uart2state = HAL_UART_Receive(&huart2, urx, 3, 500);
		//✨uart2state는 uart2에서 값을 받아옴. 아마 뒤의 3, 500은 3글자를 받고, 500ms 안에 와야한다... urx에 넣는다. 이뜻인듯
 	  if(HAL_OK == uart2state){
			//✨uart2state가 HAL_OK라면
		  if('w' == urx[0]){ // 0x77='w'
				//✨혹시 urx 첫번째 값이 0x77(w)인가요?...
			  retvalue = spi_93c56c_EWEN();
				//✨ret value는 함수의 반환값입니다. (EWEN은 정상 작동시 0 반환)
			  if(0 == retvalue){
					//✨retvalue가 0인가요? (정상 작동)
				  //printf("spi_93c56c_EWEN=%d\r\n",retvalue);
					//✨EWEN이 0이다를 말하려는 것 같은데... 주석처리됨
			  }else{
					//✨0이 아니라면 EWEN은 오류가 생겼을 것
				  printf("spi_93c56c_EWEN error\r\n");
			  }
			  HAL_Delay(10); //✨10ms기다립시다
			  retvalue = spi_93c56c_WRITE(urx[1],urx[2]);
				//✨retvalue는 WRITE함수를 실행한 결과입니다. 파라메터는 urx의 두번쨰 글자, 세번째 글자입니다. (정상 실행시 0을 반환합니다.)
			  if(0 == retvalue){
					//✨retvalue가 0인가요?
					
				  //printf("spi_93c56c_WRITE=%d\r\n",retvalue);
					//✨WRITE는 0입니다 (정상 실행 코드). 주석처리되어있다.
			  }else{
					//✨retvalue가 0이 아니라면... (비정상)
				  printf("spi_93c56c_WRITE error\r\n");
					//✨ error 출력
					
			  }
			  printf("addr=%3d(0x%02x),write data=%3d(0x%02x)\r\n",urx[1],urx[1],urx[2],urx[2]);
				//✨마지막에 실행되는 printf... urx1은 주소고 urx2는 값인듯
		  }else if('r' == urx[0]){ // 0x72='r'
				//✨urx 첫글자가 0x72(r)이라면
			  retvalue = spi_93c56c_READ(urx[1],&spi_read);
				//✨retvalue는 READ 함수의 반환값 (파라메터는 rx1과, spi_read) 정상 작동시 0을 가져옵니다 아마 spi_read에 저장하는듯
			  if(0 == retvalue){
					//✨retvalue가 0이라면 (정상 작동)
				  //printf("spi_93c56c_READ=%d\r\n",retvalue);
					//✨read가 0임을 출력... 하지만 주석처리된 코드
			  }else{
					//✨retvalue가 0이 아니라면
				  printf("spi_93c56c_READ error\r\n");
					//✨에러를 표기
					
			  }
			  printf("addr=%3d(0x%02x),read data=%3d(0x%02x)\r\n",urx[1],urx[1],spi_read,spi_read);
				//✨최종적으로 urx의 두번째 글자(주소)에서 읽어온 값 spi_read를 가져옵니다
		  }else if('h' == urx[0]){ //0x68
				printf("안녕!\r\n");
			}
  	  }

	  HAL_Delay(100); //✨100ms 기다립니다
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //✨A5(LED)를 토글합니다. LED가 깜빡거리겠네요



  }
  /* USER CODE END 3 */
}
