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
//✨메인 헤더파일

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
//✨STDIO...


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
//✨매일 보던거

//----- uart rx mode ------------------------------------------------------------------------------
typedef struct ts_rxType{
	char start;
	unsigned char data;
	char parity;
	char stop;
} rxType;
//✨RX_type이 선언되어있음. US100에서 사용하는 통신 버퍼.
//----- uart rx mode ------------------------------------------------------------------------------

//✨💎아래부터 LCD 정의


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//----- Character LCD -----------------------------------------------------------------------------
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
//✨전에 쓰던 LCD 커맨드

/*
	✨
	💬0xXXYY XXYY 식인듯?

	Clear Display는				1
	Return Home은					2
	EntryModeSet은					4
	Display Control은			8

	CursorShift 는					16

	Funtion Set은 0x20 =>	32
	SetCGRAM Addr	0x40 =>		64
	SetDDRAM Addr 0x80 =>	128

*/

// flags for display entry mode
#define LCD_ENTRYDDRAMINCREMENT 0x02
#define LCD_ENTRYDDRAMDECREMENT 0x00
#define LCD_ENTRYDISPLAYSHIFTLEFT 0x03
#define LCD_ENTRYDISPLAYSHIFTRIGHT 0x01
//✨전에 쓰던 진입 커맨드

/*
	✨
	0x02		Entry DDRAM Increment (DDRAM 증가인듯)
	0x00		Entry DDRAM Decrement (DDRAM 감소)
	0x03		Entry Display Shift Left (디스플레이 이동 왼쪽으로? 그런 뜻인듯)
	0x01		Entry Display Shift Right (디스플레이 이동 오른쪽으로)

*/

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_CURSORON 0x02
#define LCD_BLINKON 0x01
//✨전에 쓰던 커서 & 디스플레이 커맨드

/*
	✨
	0x04		DisplayOn - 📺디스플레이가 켜지는듯
	0x02		CursorOn - 🔼커서가 켜지는듯 끄는건 왜 없지❓ (살펴봤는데 ON 말고 OFF는 따로 없었다. 값을 안 넣으면 OFF인 것이 유력하다)
	0x01		BlinkOn - 🎆깜빡이가 켜지는듯


*/

// flags for cursor / display shift
#define LCD_SHIFTCURSORLEFTDECRESE 0x00
#define LCD_SHIFTCURSORRIGHTINCRESE 0x04
#define LCD_SHIFTALLDISPLAYLEFTCURSORLEFT 0x08
#define LCD_SHIFTALLDISPLAYLEFTCURSORRIGHT 0x0C
//✨전에 쓰던 커서 움직임 커맨드

/*

	✨많은 것들이 정의되어있다

	0x00		Shift Cursor Left Decrease - 커서 왼쪽으로 감소
	0x04		Shift Cursor Right Increase - 커서 오른쪽으로 증가
	0x08		Shift All Display Left Cursor Left - 모든 디스플레이와 커서가 왼쪽으로 이동
	0x0C		Shift All Display Left Cursor Right - 🎃모든 디스플레이가 왼쪽으로 가는데 커서는 오른쪽으로 감


*/

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

//✨기타... 버스 비트 크기와 라인 수, 픽셀 크기 정의됨

/*
	✨LCD 설정 정의

	0x10		LCD를 8비트 버스(모드)로 설정
	0x00		LCD를 4비트 버스(모드)로 설정

	0x08		LCD를 2개 라인 전부 사용
	0x00		LCD를 한개 라인만 사용

	0x04		LCD의 도트를 5x10으로 사용
	0x00		LCD의 도트를 5x8으로 사용

*/
//----- Character LCD -----------------------------------------------------------------------------

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2; //✨I2C2 사용

TIM_HandleTypeDef htim2; //✨타이머 2 사용

UART_HandleTypeDef huart2; //✨uart... pc 통신용

/* USER CODE BEGIN PV */
//----- Character LCD -----------------------------------------------------------------------------
uint8_t urx[16], utx[16]; //✨받기와 주기가 정의되었다
uint16_t i2cAddress = 0x4e; //✨i2cAddress는 0x4E다. 특별한 이유는 모르겠다. 아마 LCD의 주소가 4E가 아닐까❓ (확인 결과 lcd search 함수에서 LCD의 주소를 찾아다녔다... 이건 진짜 뭔지 모르겠다)
uint8_t _displayfunction = 0x00; //✨디스플레이 기능은 0x00으로 설정 (_가 있는 걸로 보기에 임시변수인듯)
uint8_t _cursordisplayshift = 0x00; //✨cursor Display shift도 0x00으로 설정.
uint8_t _displaycontrol = 0x00; //✨display control 0x00
uint8_t _displaymode = 0x00; //✨display mode 0x00
char clcdData[32]; //✨clcdData가 뭘까❓ (우리가 쓰는 문자열이였다. 이걸 가끔 전송하기도 한다. lcd_characterDatainput이런걸로)


//✨하트 모양처럼 보이는 숫자 코드 배열이다. 아마 이걸 CGRAM에 집어넣을 것으로 보인다
uint8_t Heart[] = {
  0x00, //B00000,
  0x0a, //B01010,
  0x1f, //B11111,
  0x1f, //B11111,
  0x0e, //B01110,
  0x04, //B00100,
  0x00, //B00000,
  0x00, //B00000
};


//✨벨모양
uint8_t Bell[] = {
  0x04, //B00100,
  0x0e, //B01110,
  0x0e, //B01110,
  0x0e, //B01110,
  0x1f, //B11111,
  0x00, //B00000,
  0x04, //B00100,
  0x00, //B00000
};



//✨에일리언이라는데 모르겠다. 해골같기도 하고
uint8_t Alien[] = {
	0x1f, //B11111,
	0x15, //B10101,
	0x1f, //B11111,
	0x1f, //B11111,
	0x0e, //B01110,
	0x0a, //B01010,
	0x1b, //B11011,
	0x00, //B00000
};


//✨체크 표시
uint8_t Check[] = {
	0x00, //B00000,
	0x01, //B00001,
	0x03, //B00011,
	0x16, //B10110,
	0x1c, //B11100,
	0x08, //B01000,
	0x00, //B00000,
	0x00, //B00000
};


//✨스피커 모양... 이라고 하지만 그냥 삼각형같기도 하다
uint8_t Speaker[] = {
	0x01, //B00001,
	0x03, //B00011,
	0x0f, //B01111,
	0x0f, //B01111,
	0x0f, //B01111,
	0x03, //B00011,
	0x01, //B00001,
	0x00, //B00000
};



//✨음표모양인듯
uint8_t Sound[] = {
	0x01, //B00001,
	0x03, //B00011,
	0x05, //B00101,
	0x09, //B01001,
	0x09, //B01001,
	0x0b, //B01011,
	0x1b, //B11011,
	0x18, //B11000
};


//✨해골모양
uint8_t Skull[] = {
	0x00, //B00000,
	0x0e, //B01110,
	0x15, //B10101,
	0x1b, //B11011,
	0x0e, //B01110,
	0x0e, //B01110,
	0x00, //B00000,
	0x00, //B00000
};


//✨자물쇠모양
uint8_t Lock[] = {
	0x0e, //B01110,
	0x11, //B10001,
	0x11, //B10001,
	0x1f, //B11111,
	0x1b, //B11011,
	0x1b, //B11011,
	0x1f, //B11111,
	0x00, //B00000
};
//✨💎위에까지가 기존 LCD 정의


//----- Character LCD -----------------------------------------------------------------------------


//✨⏰여기서부터 US100인듯. 이번엔 tim3가 아니라 tim2를 쓴다
//----- uart timer2 -------------------------------------------------------------------------------
int tim2_base_cnt; //✨타이머 카운트 생성... US100에서 쓸듯

//✨기존 US100 코드에서는 타이머를 두개를 썼다. 25ms로 작동하는 것 하나와 100ms 하나인 것으로 기억하는데,
//✨이걸 4배로 해서 타이머 하나를 두개처럼 쓸 수 있다.
//✨추측컨데 여기서는 그 하나만 쓰기를 사용하는듯
//----- uart timer2 -------------------------------------------------------------------------------

uint8_t urx[16], utx[16];
uint16_t Distance;
uint16_t Temperature;
//✨받은 데이터는 urx[]로, 보낼 데이터는 utx[]로 갈 것같다.
//✨uint16_t로 선언된 distance와 temperature는 숫자로 기억

//----- uart tx mode ------------------------------------------------------------------------------
char Uart_Tx_Parity_Mode; //✨US100의 송신 패리티
int txBit_cnt; //✨송신 비트의 갯수... 정의되어있지 않다
char Uart_Tx_flag; //✨Tx Flag의 여부
char Uart_Tx_trigger; //✨Tx가 실행되었는지
char UartTxData[32]; //✨보낼 데이터... utx는?❓
//----- uart tx mode ------------------------------------------------------------------------------

//----- uart rx mode ------------------------------------------------------------------------------
char Uart_Rx_Parity_Mode;
rxType UartRxData[32]; //✨기존 rxData가 UartRxData로 변했다.
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

//✨나머지는 그대로인듯
//----- uart rx mode ------------------------------------------------------------------------------

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM2_Init(void);
//✨I2C2와 TIM2가 정의됨...
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
} //✨printf로 UART transmit를 하는 코드

//----- character LCD ------------------------------------------------------------------
void lcd_send_cmd(char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	data_t[0] = data_u | 0x0C;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=0
	data_t[1] = data_u | 0x08;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=0
	data_t[2] = data_l | 0x0C;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=0
	data_t[3] = data_l | 0x08;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=0
	HAL_I2C_Master_Transmit(&hi2c2, i2cAddress,(uint8_t *) data_t, 4, 100);
} //✨완전한 command 코드... 건드릴 필요 없음

void lcd_send_data(char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=1
	data_t[1] = data_u | 0x09;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=1
	data_t[2] = data_l | 0x0D;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=1
	data_t[3] = data_l | 0x09;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=1
	HAL_I2C_Master_Transmit(&hi2c2, i2cAddress,(uint8_t *) data_t, 4, 100);
} //✨완전한 data 전송 코드... 건드릴 필요 없음

void lcd_init(void)
{ //✨기존에도 존재하던 lcd init 코드
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd(0x30);
	HAL_Delay(10);  // wait for >4.1ms
	lcd_send_cmd(0x30);
	HAL_Delay(10);  // wait for >100us
	lcd_send_cmd(0x30);
	HAL_Delay(10);
	lcd_send_cmd(0x20);  // 4bit mode
	HAL_Delay(10);

	// dislay initialisation
	_displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS; // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	lcd_send_cmd(LCD_FUNCTIONSET | _displayfunction);
	HAL_Delay(10);

	_displaymode = LCD_ENTRYDDRAMINCREMENT; //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
	HAL_Delay(10);

	_displaycontrol = LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON; //Display on/off control --> D=1, C=1, B=1
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
	HAL_Delay(10);

	lcd_send_cmd(LCD_CLEARDISPLAY); // clear display
	HAL_Delay(100);

  //✨전부 기존 Initial 코드. 변경 필요 없음
}

void lcd_send_string(char *str)
{ //✨lcd로 문자열을 보내는 명령어... 긴 문자열을 보낼 때 사용
	while (*str) lcd_send_data(*str++);
}

void lcd_clear(void)
{ //✨클리어
	lcd_send_cmd(LCD_CLEARDISPLAY);
}

void lcd_home(void)
{ //✨처음 위치로 이동
	lcd_send_cmd(LCD_RETURNHOME);
}

void lcd_setCursor(uint8_t row, uint8_t col)
{ //✨커서를 특정 위치로 이동...
	if(((row >= 0) && (row <= 1)) && ((col >= 0) && (col < 16))){
		if(row == 0){
			lcd_send_cmd(LCD_SETDDRAMADDR | (0x00 + col));
		}else if(row == 1){
			lcd_send_cmd(LCD_SETDDRAMADDR | (0x40 + col));
		}
	}
}

void lcd_characterDatainput(void) // a~z data
{
  //✨실행 할 때 마다 문자열을 a에서 z로 바꿔서 전송하는 코드
	  //printf("%c\r\n",clcdData[0]);

	  lcd_send_data(clcdData[0]);
	  if('z' == clcdData[0]){
		  clcdData[0] = 'a';
	  }else{
		  clcdData[0] += 1;
	  }
}

void lcd_createSpecialCharacter(uint8_t location, uint8_t charmap[]) {
  //✨문자열을 생성하는 명령어... 문자열을 넣을 좌표인 location(0...7 이런 걸듯)
  //✨거기에 비트 배열 (charmap)을 넣으면 특문 저장 가능
	location &= 0x7; 				// we only have 8 locations 0-7
	lcd_send_cmd(LCD_SETCGRAMADDR | (location << 3));
	for (int i = 0; i < 8; i++) {
		lcd_send_data(charmap[i]);
	}
}

//----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display
void lcd_incrementDDRAM(void) // cmd : d
{
  //✨앞으로는 오른쪽으로 커서 이동
	_displaymode = LCD_ENTRYDDRAMINCREMENT;	//0x02
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}

void lcd_decrementDDRAM(void) // cmd : e
{
  //✨앞으로는 왼쪽으로 커서 이동
	_displaymode = LCD_ENTRYDDRAMDECREMENT; // 0x00
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}

void lcd_shiftEntiredisplayLeft(void)  // cmd : f
{
  //✨앞으로는 커서와 전체 문자열 전부 왼쪽
	_displaymode = LCD_ENTRYDISPLAYSHIFTLEFT; // 0x03
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}

void lcd_shiftEntiredisplayRight(void) // cmd : g
{
  //✨앞으로는 커서와 전체 문자열 전부 오른쪽
	_displaymode = LCD_ENTRYDISPLAYSHIFTRIGHT; // 0x01
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}
//----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display

//----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----
void lcd_display(void) // cmd : m
{
  //✨디스플레이 켜기
	_displaycontrol |= LCD_DISPLAYON; // 0x04
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_nodisplay(void) // cmd : n
{
  //✨디스플레이 끄기
	_displaycontrol &= ~LCD_DISPLAYON; // ~(0x04)
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_cursor(void) // cmd : o
{
  //✨커서 켜기
	_displaycontrol |= LCD_CURSORON; // 0x02
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_nocursor(void) // cmd : p
{
  //✨커서 끄기
	_displaycontrol &= ~LCD_CURSORON; // ~(0x02)
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_blink(void) // cmd : q
{
  //✨깜빡이 켜기
	_displaycontrol |= LCD_BLINKON; // 0x01
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_noblink(void) // cmd : r
{
  //✨깜빡이 끄기
	_displaycontrol &= ~LCD_BLINKON; // ~(0x01)
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}
//----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----

//----- LCD_CURSORSHIFT : cursor & dispaly shift -----
void lcd_shiftCursorLeft(void) // cmd : s
{
  //✨앞으로는 커서 왼쪽으로
	_cursordisplayshift = LCD_SHIFTCURSORLEFTDECRESE ; // 0x00
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}

void lcd_shiftCursorRight(void) // cmd : t
{
  //✨앞으로는 커서 오른쪽으로
	_cursordisplayshift = LCD_SHIFTCURSORRIGHTINCRESE ; // 0x04
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}

void lcd_shiftAllDisplayLeft(void) // cmd : u
{
  //✨앞으로는 커서와 모든 문자열 왼쪽으로
	_cursordisplayshift = LCD_SHIFTALLDISPLAYLEFTCURSORLEFT; // 0x08
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}

void lcd_shiftAllDisplayRight(void) // cmd : v
{
  //✨앞으로는 커서와 모든 문자열 오른쪽으로
	_cursordisplayshift = LCD_SHIFTALLDISPLAYLEFTCURSORRIGHT; // 0x0C
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}
//----- LCD_CURSORSHIFT : cursor & dispaly shift -----

void lcd_address_search(void)
{
  //✨주소 서치... 안 될 때가 있던데 이유는 모르겠다.
  //✨연결 문제로 추정
	HAL_StatusTypeDef i2cstatus;

	for(uint16_t i = 0; i < 128; i++){
		i2cstatus = HAL_I2C_IsDeviceReady(&hi2c2, i << 1, 1, 10);
		if(HAL_OK == i2cstatus){
		  printf("OK addr=0x%02x\r\n",(i << 1)); //✨I2C가 존재하는 주소를 확인
      //✨나온 값을 정의된 주소에 넣어주면 된다
		  //break;
		}else{
		  //printf("err addr=0x%02x\r\n",(i << 1));
		}
	}
	printf("lcd_address_search\r\n");
}
//✨📺LCD 함수 정의 끝!
//----- character LCD ------------------------------------------------------------------

//----- timer callback ----------------------------------------------------------------------------
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  //✨⏰ US100을 위한 타이머
	if(htim->Instance == htim2.Instance){
		//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8); //✨A8이 깜빡이는 코드. 왜 존재하는지는 모른다.
		tim2_base_cnt += 1; //✨이 코드가 실행되면 아래의 현상이 예상된다

    /*

      ✨
        기존의 코드에서는 tim2_flag가 yes일 때 카운트를 확인했다.
        그 이유는 RX와 다르게 TX(보내기)는 클럭에 맞춰서 데이터를 써야하기 때문에...

        (데이터 전송)함수가 실행될 때 그 함수가 flag가 켜지고 꺼지기를 관리했다.
        그렇기 때문에 전송을 시작할 때 타이머에 맞춰서 cnt를 센다.

        cnt는 현재 비트가 몇번째인지를 말하고, 이를 통해 현재 우리가 전송해야하는 비트가 몇번째 비트인지 다른 함수에서 알 수 있게 된다
        여기서는 매 순간 타이머가 작동하는데...

        지금 확인해보니 Tx_Flag가 안에 들어있어서 보내야 할 때인지 아닌지를 점검하기 때문에 문제가 없어 보이긴 한다.



    */

		if((tim2_base_cnt % 4) == 0){
			if('y' == Uart_Tx_flag){
				txBit_cnt += 1;
				Uart_Tx_trigger = 'y';
			}
			tim2_base_cnt = 0;
		}//✨💧기존 코드와 조금 다르지만... 전혀 문제 없을 것으로 보임
		if('y' == Uart_Rx_flag){ //✨기존에도 존재하던 rxflag..

      /*

        ✨
          코드를 점검해보니 코드가 시작되면 Rx_Flag가 n였다가 바로 y로 바뀐다.
          즉 켜지는 순간 수신이 시작되며 코드 전체에 수신을 종료하는 코드는 없다.

          영원히 수신한다.


      */


			if('n' == Uart_Rx_active){
        //✨Rx가 Active가 아니라면(US100이 Start Condition을 보내지 않았다는 의미)
				Uart_Rx_Temp <<= 1; //✨Temp를 왼쪽으로 민다 (안 받으면 안 받는대로 밀고있음) (어차피 아무것도 없어서 상관 없다)
				if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)){
          //✨C0를 읽어서 true라면
					Uart_Rx_Temp += 1; //✨1을 더한다 (TRUE)
				}
				if((Uart_Rx_Temp & 0x0f) == 0x0c){ //✨임시값이 Start Condition과 같다면
					Uart_Rx_active = 'y';
					Uart_Rx_Temp = 0x00;
					rxByte_cnt += 1;
					UartRxData[rxByte_cnt].start = 'y';
					UartRxData[rxByte_cnt].data = 0x00;
					UartRxData[rxByte_cnt].parity = 'n';
					UartRxData[rxByte_cnt].stop = 'x';
					rxBit_cnt = 0;
					rxSamp_cnt = 0;
					rxParity_bit = 0;
					Uart_Rx_trigger = 'n';
					Uart_Rx_data = 'n';
					Uart_Rx_parity = 'n';
					Uart_Rx_stop = 'n';
				}
			}else{ //✨스타트 컨디션을 받고 읽기가 시작됨
				Uart_Rx_trigger = 'n';
				rxSamp_cnt += 1;
				if((rxSamp_cnt % 4) == 0){
					Uart_Rx_trigger = 'y';
					if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)){
						rxBit_value = 1;
					}else{
						rxBit_value = 0;
					}
					rxBit_cnt += 1;
					if((1 <= rxBit_cnt) && (rxBit_cnt <= 8)){
						Uart_Rx_data = 'y';
					}else if(9 == rxBit_cnt){
						Uart_Rx_data = 'n';
						if('n' == Uart_Rx_Parity_Mode){
							Uart_Rx_parity = 'n';
							Uart_Rx_stop = 'y';
						}else if('e' == Uart_Rx_Parity_Mode){
							Uart_Rx_parity = 'y';
						}else if('o' == Uart_Rx_Parity_Mode){
							Uart_Rx_parity = 'y';
						}
					}else if(10 == rxBit_cnt){
						Uart_Rx_parity = 'n';
						Uart_Rx_stop = 'y';
					}
				}
				if(('y' == Uart_Rx_trigger) && ('y' == Uart_Rx_data)){
					Uart_Rx_trigger = 'n';
					UartRxData[rxByte_cnt].data >>= 1;
					if(1 == rxBit_value){
						UartRxData[rxByte_cnt].data += 0x80;
						rxParity_bit += 1;
					}
				}
				if(('y' == Uart_Rx_trigger) && ('y' == Uart_Rx_parity)){
					Uart_Rx_trigger = 'n';
					if('e' == Uart_Rx_Parity_Mode){
						if(((rxParity_bit % 2) == 0) && (0 == rxBit_value)){
							UartRxData[rxByte_cnt].parity = 'e';
						}else if(((rxParity_bit % 2) == 1) && (1 == rxBit_value)){
							UartRxData[rxByte_cnt].parity = 'e';
						}else{
							UartRxData[rxByte_cnt].parity = 'f';
						}
					}else if('o' == Uart_Rx_Parity_Mode){
						if(((rxParity_bit % 2) == 0) && (1 == rxBit_value)){
							UartRxData[rxByte_cnt].parity = 'o';
						}else if(((rxParity_bit % 2) == 1) && (0 == rxBit_value)){
							UartRxData[rxByte_cnt].parity = 'o';
						}else{
							UartRxData[rxByte_cnt].parity = 'f';
						}
					}
				}
				if(('y' == Uart_Rx_trigger) && ('y' == Uart_Rx_stop)){
					Uart_Rx_trigger = 'n'; //✨기존 코드에는 없었는데, 아마 Rx 특성상 StartCondition을 감지하기 전 까지는
          //✨Rx_trigger 자체를 신경쓰지 않아서 그랬던 것 같다. Rx_trigger가 n으로 바뀌는게 정상

          //✨정상 코드
					Uart_Rx_active = 'n';
					if(1 == rxBit_value){
						UartRxData[rxByte_cnt].stop = 'y';
					}else{
						UartRxData[rxByte_cnt].stop = 'n';
					}
				}
			}
		}
	}
}
//----- timer callback ----------------------------------------------------------------------------

//----- uart tx mode ------------------------------------------------------------------------------
void Uart_Tx_Byte(char TxData, char Parity)
{ //✨기존의 바이트 전송 코드... US100에 하고싶은 말을 전송한다
	char chExit = 'n';
	char chComp = 0x01;
	int ParityBit_cnt = 0;

	txBit_cnt = 0;
	Uart_Tx_flag = 'y';
	Uart_Tx_trigger = 'n';

	while(1){
		if(('y' == Uart_Tx_trigger) && (txBit_cnt == 1)){ // idle
			Uart_Tx_trigger = 'n';
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
		}else if(('y' == Uart_Tx_trigger) && (txBit_cnt == 2)){ // start
			Uart_Tx_trigger = 'n';
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		}else if(('y' == Uart_Tx_trigger) && ((txBit_cnt >= 3) && (txBit_cnt <= 10))){ // bit 0~7
			Uart_Tx_trigger = 'n';
			if((TxData & chComp) == chComp){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				ParityBit_cnt += 1;
			}else{
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			chComp <<= 1;
      /*

        ✨
          갑자기 0번 비트부터 7번 비트까지의 코드가 하나로 합쳐졌는데...
          정상으로 보인다.

          기존의 0~7번 비트를 만드는 코드를 보니 의미 없이 똑같은 코드가 반복된다.
          이 코드는 기존의 쓸모 없이 반복되는 코드를 하나로 합친 것.


          정상

      */
		}else if(('y' == Uart_Tx_trigger) && (txBit_cnt == 11)){ // [parity : none/even/odd] or [stop]
			Uart_Tx_trigger = 'n';
			if('n' == Parity){	// stop
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				chExit = 'y';
			}else if('e' == Parity){
				if((ParityBit_cnt % 2) == 0){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
				}else{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				}
			}else if('o' == Parity){ //✨코드 스타일이 바뀌었다. (동작은 그대로)
				if((ParityBit_cnt % 2) == 0){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				}else{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
				}
			}
		}else if(('y' == Uart_Tx_trigger) && (txBit_cnt == 12)){ // stop
			Uart_Tx_trigger = 'n'; //✨기존에는 없던 Uart_Tx_Trigger = 'n'가 생겼다.
      //✨🔥 오류 : 여기서는 trigger를 제어해서는 안된다. 아래에 기술된 오류가 발생할 수 있음
      /*

        ✨
          하지만 문맥상 이 다음 코드는 반드시 trigger가 y며 chExit가 y인 경우에만 작동한다...

          그렇다면 다음의 Tx_flag가 n이 되는 코드가 작동하지 않을 수 있다.

          그렇다면 SendByte(현재 함수)의 권한인 Tx_Flag 끄기가 제대로 실행되지 않고,
          타이머는 계속해서 비트의 갯수를 셀 것이고 그러면 Hashing등이 동작하지 않을 수 있다.

          내가 보기에는 여기 있는 Tx_trigger = 'n'는 잘못 작성된 코드이다


      */

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
			chExit = 'y';
		}
		if(('y' == Uart_Tx_trigger) && ('y' == chExit)){
			Uart_Tx_trigger = 'n';
			Uart_Tx_flag = 'n';
			break;
		}
	}
}

int Uart_Tx_multiByte(int Tx_cnt, char* TxData, char Parity)
{
	int m; //✨멀티바이트를 전송하는 코드... 하지만 US100은 0x55-> 거리 측정 0x50 -> 온도 측정이기 때문에 멀티바이트 전송할 이유가 없다

	for(m = 0; m < Tx_cnt; m++){
		Uart_Tx_Byte(TxData[m], Parity);
	}

	return(0);
}
//----- uart tx mode ------------------------------------------------------------------------------

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	HAL_StatusTypeDef uart2status;
	HAL_StatusTypeDef tim2_it_start;
	GPIO_PinState pc13pin;


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
  MX_I2C2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  printf("---US100 & character LCD(i2c) test---\r\n");
  //✨US100과 LCD 테스트임을 알림

  //----- character LCD ---------------------------------------------------------------------------
  HAL_Delay(10);
  lcd_init();
  HAL_Delay(20);
  sprintf(clcdData,"chd");
  lcd_send_string(clcdData);
  HAL_Delay(20);
  clcdData[0] = 'a';
  //✨기존에 쓰던 LCD 시작 코드... LCD init도 있고 send_string으로 chd로 표시한다.
  //----- character LCD ----------------------------------------------------------------------------

  //----- timer2 -----------------------------------------------------------------------------------
  tim2_base_cnt = 0; //✨타이머 시작 전 준비
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); //✨A0를 set함. US100의 Tx가 idle시 High이기 때문

  tim2_it_start = HAL_TIM_Base_Start_IT(&htim2);
  if(HAL_OK == tim2_it_start){
	  printf("HAL_TIM_Base_Start_IT OK\r\n");
  }else{
	  printf("HAL_TIM_Base_Start_IT error\r\n");
  }
  //----- timer2 -----------------------------------------------------------------------------------

  //----- uart tx mode ------------------------------------------------------------------------------
  Uart_Tx_Parity_Mode = 'n'; // 'n':none, 'e':even, 'o':odd
  txBit_cnt = 0;
  Uart_Tx_flag = 'n';
  Uart_Tx_trigger = 'n';
  //✨US100의 송신부 초기화... 변수명이 좀 더 자세해진 것 같다

  UartTxData[0] = 'a'; //✨신입. 딱히 하는 일이 없다 (아직까지는) - 확인해보니 신입 아님
  //----- uart tx mode ------------------------------------------------------------------------------

  //----- uart rx mode ------------------------------------------------------------------------------
  Uart_Rx_Parity_Mode = 'n'; // 'n':none, 'e':even, 'o':odd
  rxByte_cnt = 0;
  rxBit_cnt = 0;
  rxSamp_cnt = 0;
  Uart_Rx_flag = 'n';
  Uart_Rx_active = 'n';
  Uart_Rx_Temp = 0x00;
  //✨기존 수신부 코드 초기화... 변수명이 바뀌지 않았다.

  //-----uart rx mode : start-----
  Uart_Rx_flag = 'y';
  Uart_Rx_active = 'n';
  //----- uart rx mode ------------------------------------------------------------------------------


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  uart2status = HAL_UART_Receive(&huart2, urx, 1, 300);
    //✨UART2로부터 1바이트 읽어옴 (from LCD)
	  if(HAL_OK == uart2status){
      //✨OK라면
		  printf("*%c*\r\n",urx[0]); //✨Rx 데이터에 들어있는 (방금 UART_Receive로 읽은 값) 값을 출력 *a*등으로 출력됨(예시)
		  switch(urx[0]){ //✨Rx를 swich로 넘김
		  case 'c':
			  lcd_clear(); //✨기존 클리어 코드
			  break;
		  case 'h':
			  lcd_home(); //✨기존 return home 코드
			  break;
		  case 'i':
			  lcd_init(); //✨재-initial 코드
			  sprintf(clcdData,"chd"); //✨chd를 clcdData에 넣음
			  lcd_send_string(clcdData); //✨clcdData를 LCD에 출력
			  clcdData[0] = 'a'; //✨clcdData[0]은 a다...

        //✨아무래도 lcd_characterDatainput 이런걸로 데이터가 오염된 경우 초기화하기 위함인듯
			  break;
		  case 'a':
			  lcd_setCursor(0, 0); //첫번째줄 처음으로 이동
			  break;
		  case 'b':
			  lcd_setCursor(1, 0); //두번째줄 처음으로 이동
			  break;
		  case 'k':
			  lcd_characterDatainput(); //✨다음문자열을 뽑아서 LCD에 출력
			  break;
			  //----- special character -----
		  case 'x':
      //✨특문을 지정
			  lcd_createSpecialCharacter(0, Heart);
			  lcd_createSpecialCharacter(1, Bell);
			  lcd_createSpecialCharacter(2, Alien);
			  lcd_createSpecialCharacter(3, Check);
			  lcd_createSpecialCharacter(4, Speaker);
			  lcd_createSpecialCharacter(5, Sound);
			  lcd_createSpecialCharacter(6, Skull);
			  lcd_createSpecialCharacter(7, Lock);
			  break;
		  case 'y':
      //✨특문을 출력
			  lcd_setCursor(1, 0); lcd_send_data(0);
			  lcd_setCursor(1, 1); lcd_send_data(1);
			  lcd_setCursor(1, 2); lcd_send_data(2);
			  lcd_setCursor(1, 3); lcd_send_data(3);
			  lcd_setCursor(1, 4); lcd_send_data(4);
			  lcd_setCursor(1, 5); lcd_send_data(5);
			  lcd_setCursor(1, 6); lcd_send_data(6);
			  lcd_setCursor(1, 7); lcd_send_data(7);
			  break;
			  //----- special character -----
		  default:
			  break;
		  }
	  }


    /*
      ✨
        Parity를 체크하는 코드가 없다 필요 없어서 그런듯?

        기존에 존재하던 LCD의 다양한 명령어도 없다.
        필요 없는듯

    */

	  pc13pin = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
	  if(GPIO_PIN_RESET == pc13pin){
		  //----- distance -----
      lcd_clear(); //✨청소
      lcd_setCursor(0, 0); //✨윗줄

      Uart_Tx_Byte(0x55, 'n'); //✨여기서 패리티를 정한다 (애초에 측정이 필요 없다)

      HAL_Delay(200); //✨내가 200ms 기다리고싶음

      if(('n' == Uart_Rx_active) && (1 <= rxByte_cnt)){
        int resultDistance = (UartRxData[1].data << 8) + UartRxData[2].data;
        sprintf(clcdData, "Distance : %d", resultDistance);
        lcd_send_string(clcdData);

        rxByte_cnt = 0;
      }else{
        sprintf(clcdData, "DIST ERR");
        lcd_send_string(clcdData);
        rxByte_cnt = 0;
      }
		  //----- 여기에 코드 추가 ------...DONE!

		  //----- distance -----

		  HAL_Delay(1000); //✨1초 기다리기
		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //✨A5 상태를 변경
      lcd_setCursor(1, 0);

      Uart_Tx_Byte(0x50, 'n');

      HAL_Delay(200);

      if(('n' == Uart_Rx_active) && (1 <= rxByte_cnt)){
        int resultTemperature = UartRxData[1].data - 45;
        sprintf(clcdData, "Temperature : %d", resultTemperature);
        lcd_send_string(clcdData);

        rxByte_cnt = 0;
      }else{
        sprintf(clcdData, "TEMP ERR");
        lcd_send_string(clcdData);
        rxByte_cnt = 0;
      }

		  //----- temperature -----

		  //----- 여기에 코드 추가 ------

		  //----- temperature -----
	  }

	  HAL_Delay(100); //✨100ms 기다리기
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //✨A5가 빠르게 깜빡일듯


  }
  /* USER CODE END 3 */
}
