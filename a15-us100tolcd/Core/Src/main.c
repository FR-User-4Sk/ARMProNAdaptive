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


//?��RX_type?�� ?��?��?��?��?��?��. US100?��?�� ?��?��?��?�� ?��?�� 버퍼.
typedef struct ts_rxType{
	char start;
	unsigned char data;
	char parity;
	char stop;
} rxType;


#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
//?��?��?�� ?��?�� LCD 커맨?��

#define LCD_ENTRYDDRAMINCREMENT 0x02
#define LCD_ENTRYDDRAMDECREMENT 0x00
#define LCD_ENTRYDISPLAYSHIFTLEFT 0x03
#define LCD_ENTRYDISPLAYSHIFTRIGHT 0x01
//?��?��?�� ?��?�� 진입 커맨?��

#define LCD_DISPLAYON 0x04
#define LCD_CURSORON 0x02
#define LCD_BLINKON 0x01
//?��?��?�� ?��?�� 커서 & ?��?��?��?��?�� 커맨?��

#define LCD_SHIFTCURSORLEFTDECRESE 0x00
#define LCD_SHIFTCURSORRIGHTINCRESE 0x04
#define LCD_SHIFTALLDISPLAYLEFTCURSORLEFT 0x08
#define LCD_SHIFTALLDISPLAYLEFTCURSORRIGHT 0x0C
//?��?��?�� ?��?�� 커서 ??직임 커맨?��


#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


uint8_t urx[16], utx[16]; //?��받기?? 주기�? ?��?��?��?��?��
uint16_t i2cAddress = 0x4e; //?��i2cAddress?�� 0x4E?��. ?��별한 ?��?��?�� 모르겠다. ?���? LCD?�� 주소�? 4E�? ?��?��까❓ (?��?�� 결과 lcd search ?��?��?��?�� LCD?�� 주소�? 찾아?��?��?��... ?���? 진짜 뭔�? 모르겠다)
uint8_t _displayfunction = 0x00; //?��?��?��?��?��?�� 기능?? 0x00?���? ?��?�� (_�? ?��?�� 걸로 보기?�� ?��?���??��?��?��)
uint8_t _cursordisplayshift = 0x00; //?��cursor Display shift?�� 0x00?���? ?��?��.
uint8_t _displaycontrol = 0x00; //?��display control 0x00
uint8_t _displaymode = 0x00; //?��display mode 0x00
char clcdData[32]; //?��clcdData�? 뭘까?�� (?��리�? ?��?�� 문자?��?��???��. ?���? �??�� ?��?��?��기도 ?��?��. lcd_characterDatainput?��?��걸로)

//?��?��?�� 모양처럼 보이?�� ?��?�� 코드 배열?��?��. ?���? ?���? CGRAM?�� 집어?��?�� 것으�? 보인?��
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


//?��벨모?��
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



//?��?��?��리언?��?��?��?�� 모르겠다. ?��골같기도 ?���?
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


//?��체크 ?��?��
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


//?��?��?���? 모양... ?��?���? ?���?�? 그냥 ?��각형같기?�� ?��?��
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



//?��?��?��모양?��?��
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


//?��?��골모?��
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


//?��?��물쇠모양
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
//?��?���위?��까�?�? 기존 LCD ?��?��

//----- Character LCD -----------------------------------------------------------------------------


//?��?��?��기서�??�� US100?��?��. ?��번엔 tim3�? ?��?��?�� tim2�? ?��?��
//----- uart timer2 -------------------------------------------------------------------------------
int tim2_base_cnt; //?��???���? 카운?�� ?��?��... US100?��?�� ?��?��

//?��기존 US100 코드?��?��?�� ???��머�?? ?��개�?? ?��?��. 25ms�? ?��?��?��?�� �? ?��?��?? 100ms ?��?��?�� 것으�? 기억?��?��?��,
//?��?���? 4배로 ?��?�� ???���? ?��?���? ?��개처?�� ?�� ?�� ?��?��.
//?��추측컨데 ?��기서?�� �? ?��?���? ?��기�?? ?��?��?��?��?��
//----- uart timer2 -------------------------------------------------------------------------------

uint8_t urx[16], utx[16];
uint16_t Distance;
uint16_t Temperature;
//?��받�? ?��?��?��?�� urx[]�?, 보낼 ?��?��?��?�� utx[]�? �? 것같?��.
//?��uint16_t�? ?��?��?�� distance?? temperature?�� ?��?���? 기억

//----- uart tx mode ------------------------------------------------------------------------------
char Uart_Tx_Parity_Mode; //?��US100?�� ?��?�� ?��리티
int txBit_cnt; //?��?��?�� 비트?�� �??��... ?��?��?��?��?���? ?��?��
char Uart_Tx_flag; //?��Tx Flag?�� ?���?
char Uart_Tx_trigger; //?��Tx�? ?��?��?��?��?���?
char UartTxData[32]; //?��보낼 ?��?��?��... utx?��??��
//----- uart tx mode ------------------------------------------------------------------------------

//----- uart rx mode ------------------------------------------------------------------------------
char Uart_Rx_Parity_Mode;
rxType UartRxData[32]; //?��기존 rxData�? UartRxData�? �??��?��.
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

//?��?��머�??�� 그�?로인?��
//----- uart rx mode ------------------------------------------------------------------------------






/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM2_Init(void);
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
} //?��printf�? UART transmit�? ?��?�� 코드

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
} //?��?��?��?�� command 코드... 건드�? ?��?�� ?��?��

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
} //?��?��?��?�� data ?��?�� 코드... 건드�? ?��?�� ?��?��

void lcd_init(void)
{ //?��기존?��?�� 존재?��?�� lcd init 코드
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

  //?��?���? 기존 Initial 코드. �?�? ?��?�� ?��?��
}

void lcd_send_string(char *str)
{ //?��lcd�? 문자?��?�� 보내?�� 명령?��... �? 문자?��?�� 보낼 ?�� ?��?��
	while (*str) lcd_send_data(*str++);
}

void lcd_clear(void)
{ //?��?��리어
	lcd_send_cmd(LCD_CLEARDISPLAY);
}

void lcd_home(void)
{ //?��처음 ?��치로 ?��?��
	lcd_send_cmd(LCD_RETURNHOME);
}

void lcd_setCursor(uint8_t row, uint8_t col)
{ //?��커서�? ?��?�� ?��치로 ?��?��...
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
  //?��?��?�� ?�� ?�� 마다 문자?��?�� a?��?�� z�? 바꿔?�� ?��?��?��?�� 코드
	  //printf("%c\r\n",clcdData[0]);

	  lcd_send_data(clcdData[0]);
	  if('z' == clcdData[0]){
		  clcdData[0] = 'a';
	  }else{
		  clcdData[0] += 1;
	  }
}

void lcd_createSpecialCharacter(uint8_t location, uint8_t charmap[]) {
  //?��문자?��?�� ?��?��?��?�� 명령?��... 문자?��?�� ?��?�� 좌표?�� location(0...7 ?��?�� 걸듯)
  //?��거기?�� 비트 배열 (charmap)?�� ?��?���? ?���? ???�� �??��
	location &= 0x7; 				// we only have 8 locations 0-7
	lcd_send_cmd(LCD_SETCGRAMADDR | (location << 3));
	for (int i = 0; i < 8; i++) {
		lcd_send_data(charmap[i]);
	}
}

//----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display
void lcd_incrementDDRAM(void) // cmd : d
{
  //?��?��?��로는 ?��른쪽?���? 커서 ?��?��
	_displaymode = LCD_ENTRYDDRAMINCREMENT;	//0x02
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}

void lcd_decrementDDRAM(void) // cmd : e
{
  //?��?��?��로는 ?��쪽으�? 커서 ?��?��
	_displaymode = LCD_ENTRYDDRAMDECREMENT; // 0x00
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}

void lcd_shiftEntiredisplayLeft(void)  // cmd : f
{
  //?��?��?��로는 커서?? ?���? 문자?�� ?���? ?���?
	_displaymode = LCD_ENTRYDISPLAYSHIFTLEFT; // 0x03
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}

void lcd_shiftEntiredisplayRight(void) // cmd : g
{
  //?��?��?��로는 커서?? ?���? 문자?�� ?���? ?��른쪽
	_displaymode = LCD_ENTRYDISPLAYSHIFTRIGHT; // 0x01
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode);
}
//----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display

//----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----
void lcd_display(void) // cmd : m
{
  //?��?��?��?��?��?�� 켜기
	_displaycontrol |= LCD_DISPLAYON; // 0x04
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_nodisplay(void) // cmd : n
{
  //?��?��?��?��?��?�� ?���?
	_displaycontrol &= ~LCD_DISPLAYON; // ~(0x04)
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_cursor(void) // cmd : o
{
  //?��커서 켜기
	_displaycontrol |= LCD_CURSORON; // 0x02
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_nocursor(void) // cmd : p
{
  //?��커서 ?���?
	_displaycontrol &= ~LCD_CURSORON; // ~(0x02)
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_blink(void) // cmd : q
{
  //?��깜빡?�� 켜기
	_displaycontrol |= LCD_BLINKON; // 0x01
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_noblink(void) // cmd : r
{
  //?��깜빡?�� ?���?
	_displaycontrol &= ~LCD_BLINKON; // ~(0x01)
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol);
}
//----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----

//----- LCD_CURSORSHIFT : cursor & dispaly shift -----
void lcd_shiftCursorLeft(void) // cmd : s
{
  //?��?��?��로는 커서 ?��쪽으�?
	_cursordisplayshift = LCD_SHIFTCURSORLEFTDECRESE ; // 0x00
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}

void lcd_shiftCursorRight(void) // cmd : t
{
  //?��?��?��로는 커서 ?��른쪽?���?
	_cursordisplayshift = LCD_SHIFTCURSORRIGHTINCRESE ; // 0x04
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}

void lcd_shiftAllDisplayLeft(void) // cmd : u
{
  //?��?��?��로는 커서?? 모든 문자?�� ?��쪽으�?
	_cursordisplayshift = LCD_SHIFTALLDISPLAYLEFTCURSORLEFT; // 0x08
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}

void lcd_shiftAllDisplayRight(void) // cmd : v
{
  //?��?��?��로는 커서?? 모든 문자?�� ?��른쪽?���?
	_cursordisplayshift = LCD_SHIFTALLDISPLAYLEFTCURSORRIGHT; // 0x0C
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift);
}
//----- LCD_CURSORSHIFT : cursor & dispaly shift -----

void lcd_address_search(void)
{
  //?��주소 ?���?... ?�� ?�� ?���? ?��?��?�� ?��?��?�� 모르겠다.
  //?��?���? 문제�? 추정
	HAL_StatusTypeDef i2cstatus;

	for(uint16_t i = 0; i < 128; i++){
		i2cstatus = HAL_I2C_IsDeviceReady(&hi2c2, i << 1, 1, 10);
		if(HAL_OK == i2cstatus){
		  printf("OK addr=0x%02x\r\n",(i << 1)); //?��I2C�? 존재?��?�� 주소�? ?��?��
      //?��?��?�� 값을 ?��?��?�� 주소?�� ?��?��주면 ?��?��
		  //break;
		}else{
		  //printf("err addr=0x%02x\r\n",(i << 1));
		}
	}
	printf("lcd_address_search\r\n");
}
//?��?���LCD ?��?�� ?��?�� ?��!


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  //?��?�� US100?�� ?��?�� ???���?
	if(htim->Instance == htim2.Instance){
		//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8); //?��A8?�� 깜빡?��?�� 코드. ?�� 존재?��?���??�� 모른?��.
		tim2_base_cnt += 1; //?��?�� 코드�? ?��?��?���? ?��?��?�� ?��?��?�� ?��?��?��?��

    /*

      ?��
        기존?�� 코드?��?��?�� tim2_flag�? yes?�� ?�� 카운?���? ?��?��?��?��.
        �? ?��?��?�� RX?? ?��르게 TX(보내�?)?�� ?��?��?�� 맞춰?�� ?��?��?���? ?��?��?���? ?��문에...

        (?��?��?�� ?��?��)?��?���? ?��?��?�� ?�� �? ?��?���? flag�? 켜�?�? 꺼�?기�?? �?리했?��.
        그렇�? ?��문에 ?��?��?�� ?��?��?�� ?�� ???��머에 맞춰?�� cnt�? ?��?��.

        cnt?�� ?��?�� 비트�? 몇번째인�?�? 말하�?, ?���? ?��?�� ?��?�� ?��리�? ?��?��?��?��?��?�� 비트�? 몇번�? 비트?���? ?���? ?��?��?��?�� ?�� ?�� ?���? ?��?��
        ?��기서?�� �? ?���? ???��머�? ?��?��?��?��?��...

        �?�? ?��?��?��보니 Tx_Flag�? ?��?�� ?��?��?��?��?�� 보내?�� ?�� ?��?���? ?��?���?�? ?���??���? ?��문에 문제�? ?��?�� 보이�? ?��?��.



    */

		if((tim2_base_cnt % 4) == 0){
			if('y' == Uart_Tx_flag){
				txBit_cnt += 1;
				Uart_Tx_trigger = 'y';
			}
			tim2_base_cnt = 0;
		}//?��?���기�? 코드?? 조금 ?��르�?�?... ?��?? 문제 ?��?�� 것으�? 보임
		if('y' == Uart_Rx_flag){ //?��기존?��?�� 존재?��?�� rxflag..

      /*

        ?��
          코드�? ?���??��보니 코드�? ?��?��?���? Rx_Flag�? n???���? 바로 y�? 바�?�다.
          �? 켜�??�� ?���? ?��?��?�� ?��?��?���? 코드 ?��체에 ?��?��?�� 종료?��?�� 코드?�� ?��?��.

          ?��?��?�� ?��?��?��?��.


      */


			if('n' == Uart_Rx_active){
        //?��Rx�? Active�? ?��?��?���?(US100?�� Start Condition?�� 보내�? ?��?��?��?�� ?���?)
				Uart_Rx_Temp <<= 1; //?��Temp�? ?��쪽으�? 민다 (?�� 받으�? ?�� 받는??�? �?고있?��) (?��차피 ?��무것?�� ?��?��?�� ?���? ?��?��)
				if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)){
          //?��C0�? ?��?��?�� true?���?
					Uart_Rx_Temp += 1; //?��1?�� ?��?��?�� (TRUE)
				}
				if((Uart_Rx_Temp & 0x0f) == 0x0c){ //?��?��?��값이 Start Condition�? 같다�?
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
			}else{ //?��?��???�� 컨디?��?�� 받고 ?��기�? ?��?��?��
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
					Uart_Rx_trigger = 'n'; //?��기존 코드?��?�� ?��?��?��?��, ?���? Rx ?��?��?�� StartCondition?�� 감�??���? ?�� 까�??��
          //?��Rx_trigger ?��체�?? ?��경쓰�? ?��?��?�� 그랬?�� �? 같다. Rx_trigger�? n?���? 바�?�는�? ?��?��

          //?��?��?�� 코드
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
{ //?��기존?�� 바이?�� ?��?�� 코드... US100?�� ?��고싶?? 말을 ?��?��?��?��
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

        ?��
          갑자�? 0�? 비트�??�� 7�? 비트까�??�� 코드�? ?��?���? ?��쳐졌?��?��...
          ?��?��?���? 보인?��.

          기존?�� 0~7�? 비트�? 만드?�� 코드�? 보니 ?���? ?��?�� ?��같�? 코드�? 반복?��?��.
          ?�� 코드?�� 기존?�� ?���? ?��?�� 반복?��?�� 코드�? ?��?���? ?���? �?.


          ?��?��

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
			}else if('o' == Parity){ //?��코드 ?��???��?�� 바�?�었?��. (?��?��?? 그�?�?)
				if((ParityBit_cnt % 2) == 0){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				}else{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
				}
			}
		}else if(('y' == Uart_Tx_trigger) && (txBit_cnt == 12)){ // stop
			Uart_Tx_trigger = 'n'; //?��기존?��?�� ?��?�� Uart_Tx_Trigger = 'n'�? ?��겼다.
      //?��?��? ?���? : ?��기서?�� trigger�? ?��?��?��?��?�� ?��?��?��. ?��?��?�� 기술?�� ?��류�? 발생?�� ?�� ?��?��
      /*

        ?��
          ?���?�? 문맥?�� ?�� ?��?�� 코드?�� 반드?�� trigger�? y�? chExit�? y?�� 경우?���? ?��?��?��?��...

          그렇?���? ?��?��?�� Tx_flag�? n?�� ?��?�� 코드�? ?��?��?���? ?��?�� ?�� ?��?��.

          그렇?���? SendByte(?��?�� ?��?��)?�� 권한?�� Tx_Flag ?��기�? ?��??�? ?��?��?���? ?���?,
          ???��머는 계속?��?�� 비트?�� �??���? ?? 것이�? 그러�? Hashing?��?�� ?��?��?���? ?��?�� ?�� ?��?��.

          ?���? 보기?��?�� ?���? ?��?�� Tx_trigger = 'n'?�� ?���? ?��?��?�� 코드?��?��


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
	int m; //?���??��바이?���? ?��?��?��?�� 코드... ?���?�? US100?? 0x55-> 거리 측정 0x50 -> ?��?�� 측정?���? ?��문에 �??��바이?�� ?��?��?�� ?��?���? ?��?��

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

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
	  //?��US100�? LCD ?��?��?��?��?�� ?���?

	  //----- character LCD ---------------------------------------------------------------------------
	  HAL_Delay(10);
	  lcd_init();
	  HAL_Delay(20);
	  sprintf(clcdData,"chd");
	  lcd_send_string(clcdData);
	  HAL_Delay(20);
	  clcdData[0] = 'a';
	  //?��기존?�� ?��?�� LCD ?��?�� 코드... LCD init?�� ?���? send_string?���? chd�? ?��?��?��?��.
	  //----- character LCD ----------------------------------------------------------------------------

	  //----- timer2 -----------------------------------------------------------------------------------
	  tim2_base_cnt = 0; //?��???���? ?��?�� ?�� �?�?
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); //?��A0�? set?��. US100?�� Tx�? idle?�� High?���? ?���?

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
	  //?��US100?�� ?��?���? 초기?��... �??��명이 �? ?�� ?��?��?���? �? 같다

	  UartTxData[0] = 'a'; //?��?��?��. ?��?�� ?��?�� ?��?�� ?��?�� (?��직까�??��) - ?��?��?��보니 ?��?�� ?��?��
	  //----- uart tx mode ------------------------------------------------------------------------------

	  //----- uart rx mode ------------------------------------------------------------------------------
	  Uart_Rx_Parity_Mode = 'n'; // 'n':none, 'e':even, 'o':odd
	  rxByte_cnt = 0;
	  rxBit_cnt = 0;
	  rxSamp_cnt = 0;
	  Uart_Rx_flag = 'n';
	  Uart_Rx_active = 'n';
	  Uart_Rx_Temp = 0x00;
	  //?��기존 ?��?���? 코드 초기?��... �??��명이 바�?��? ?��?��?��.

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
	    //?��UART2로�??�� 1바이?�� ?��?��?�� (from LCD)
		  if(HAL_OK == uart2status){
	      //?��OK?���?
			  printf("*%c*\r\n",urx[0]); //?��Rx ?��?��?��?�� ?��?��?��?�� (방금 UART_Receive�? ?��?? �?) 값을 출력 *a*?��?���? 출력?��(?��?��)
			  switch(urx[0]){ //?��Rx�? swich�? ?���?
			  case 'c':
				  lcd_clear(); //?��기존 ?��리어 코드
				  break;
			  case 'h':
				  lcd_home(); //?��기존 return home 코드
				  break;
			  case 'i':
				  lcd_init(); //?��?��-initial 코드
				  sprintf(clcdData,"chd"); //?��chd�? clcdData?�� ?��?��
				  lcd_send_string(clcdData); //?��clcdData�? LCD?�� 출력
				  clcdData[0] = 'a'; //?��clcdData[0]?? a?��...

	        //?��?��무래?�� lcd_characterDatainput ?��?��걸로 ?��?��?���? ?��?��?�� 경우 초기?��?���? ?��?��?��?��
				  break;
			  case 'a':
				  lcd_setCursor(0, 0); //첫번째줄 처음?���? ?��?��
				  break;
			  case 'b':
				  lcd_setCursor(1, 0); //?��번째�? 처음?���? ?��?��
				  break;
			  case 'k':
				  lcd_characterDatainput(); //?��?��?��문자?��?�� 뽑아?�� LCD?�� 출력
				  break;
				  //----- special character -----
			  case 'x':
	      //?��?��문을 �??��
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
	      //?��?��문을 출력
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
	      ?��
	        Parity�? 체크?��?�� 코드�? ?��?�� ?��?�� ?��?��?�� 그런?��?

	        기존?�� 존재?��?�� LCD?�� ?��?��?�� 명령?��?�� ?��?��.
	        ?��?�� ?��?��?��

	    */

		  pc13pin = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
		  if(GPIO_PIN_RESET == pc13pin){
			  printf("btn pressed\r\n");
				  //----- distance -----
			  lcd_clear(); //?���??��
			  HAL_Delay(200);
			  lcd_setCursor(0, 0); //?��?���?
			  HAL_Delay(200);

			  Uart_Tx_Byte(0x55, 'n'); //?��?��기서 ?��리티�? ?��?��?�� (?��초에 측정?�� ?��?�� ?��?��)

			  HAL_Delay(200); //?��?���? 200ms 기다리고?��?��

			  if(('n' == Uart_Rx_active) && (1 <= rxByte_cnt)){
				int resultDistance = (UartRxData[1].data << 8) + UartRxData[2].data;
				sprintf(clcdData, "D : %dmm", resultDistance);
				lcd_send_string(clcdData);

				rxByte_cnt = 0;
			  }else{
				sprintf(clcdData, "DIST ERR");
				lcd_send_string(clcdData);
				rxByte_cnt = 0;
			  }
				  //----- ?��기에 코드 추�? ------...DONE!

				  //----- distance -----

			  HAL_Delay(1000); //?��1�? 기다리기
			  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //?��A5 ?��?���? �?�?
			  lcd_setCursor(1, 0);

			  HAL_Delay(200);

			  Uart_Tx_Byte(0x50, 'n');

			  HAL_Delay(200);

			  if(('n' == Uart_Rx_active) && (1 <= rxByte_cnt)){
				int resultTemperature = UartRxData[1].data - 45;
				sprintf(clcdData, "T : %d", resultTemperature);
				lcd_send_string(clcdData);

				rxByte_cnt = 0;
			  }else{
				sprintf(clcdData, "TEMP ERR");
				lcd_send_string(clcdData);
				rxByte_cnt = 0;
			  }

			  //----- temperature -----

			  //----- ?��기에 코드 추�? ------...DONE!

			  //----- temperature -----
    }

    HAL_Delay(100); //?��100ms 기다리기
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //?��A5�? 빠르�? 깜빡?��?��
  }
  /* USER CODE END 3 */
  }
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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

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
  htim2.Init.Period = 26-1;
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
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 LD2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
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
