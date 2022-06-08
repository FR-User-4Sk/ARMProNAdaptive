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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

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



/*
	✨LCD 설정 정의
	
	0x10		LCD를 8비트 버스(모드)로 설정
	0x00		LCD를 4비트 버스(모드)로 설정
	
	0x08		LCD를 2개 라인 전부 사용
	0x00		LCD를 한개 라인만 사용
	
	0x04		LCD의 도트를 5x10으로 사용
	0x00		LCD의 도트를 5x8으로 사용

*/

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C2_Init(void);
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
} //✨printf를 사용하게 만들어주는 함수. 많이 봐왔다

//----- character LCD ------------------------------------------------------------------
void lcd_send_cmd(char cmd)
{
	//✨cmd에는 뭐가 들어올까? 일단 1바이트짜리 문자열로 정의되어있다❓
	char data_u, data_l; //✨문자 ASCII의 왼쪽 오른쪽을 말하는듯
	uint8_t data_t[4];
	data_u = (cmd & 0xf0); //✨들어온 문자열의 🧡왼쪽을 살렸다
	data_l = ((cmd << 4) & 0xf0); //✨들어온 문자열의 💛오른쪽을 살렸다.
	data_t[0] = data_u | 0x0C;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=0 ✨DDRAM 주소 설정
	data_t[1] = data_u | 0x08;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=0 ✨CGRAM 주소 설정
	data_t[2] = data_l | 0x0C;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=0 ✨DDRAM 주소 설정
	data_t[3] = data_l | 0x08;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=0 ✨CGRAM 주소 설정
	
	/*
		✨ data가 4개 선언되었길래 뭐 하는 건가 했는데, 글자를 처리해서 하나씩 보내고 있다.
		
		만약 0x1011 1101이라는 문자열이 들어왔다고 가정하면... (저 문자열이 ASCII에서 뭔지는 모르겠다)
		
		데이터가
			data_u = 0x1011 0000
			data_l = 0x1101 0000
			
		이렇게 변하고
		
		t[0] = 0x1011 1100
		t[1] = 0x1011 1000
		
		t[2] = 0x1101 1100
		t[3] = 0x1101 1000
		
		이렇게 된다.
		
		이걸 바로 밑의 함수가 i2cAddress로 보내는 것 같다.
	
	
	*/
	
	
	
	HAL_I2C_Master_Transmit(&hi2c2, i2cAddress,(uint8_t *) data_t, 4, 100);
	
	//✨추정하건데, i2c2를 통해 i2cAddress로 data_t를 0부터 3까지 4개 보내고 100ms 기다리는 것 같다
}

void lcd_send_data(char data)
{
	
	//✨위와 같은 파라메터지만 이름이 data이다. 아무래도 위에껀 실수가 아니라 명령어 Command를 말하는 것인가보다
	char data_u, data_l; //✨아마 왼쪽 오른쪽인듯
	uint8_t data_t[4]; //✨보낼 4개의 문자
	data_u = (data & 0xf0); //✨왼쪽만 살린다
	data_l = ((data << 4) & 0xf0); //✨오른쪽만 살린다
	data_t[0] = data_u | 0x0D;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=1 ✨주소에 데이터 쓰기 첫 주소비트와 두번째 주소 비트가 전부 1이다
	data_t[1] = data_u | 0x09;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=1 ✨주소에 데이터를 쓰는데... 첫 주소 비트가 0이고 두번째는 1이다. 왜지?
	data_t[2] = data_l | 0x0D;  //[3]backlight=1, [2]en=1, [1]r/w=0, [0]rs=1 ✨주소에 데이터 쓰기 첫 주소비트와 두번째 주소 비트가 전부 1이다
	data_t[3] = data_l | 0x09;  //[3]backlight=1, [2]en=0, [1]r/w=0, [0]rs=1 ✨주소에 데이터를 쓴다. 첫비트는 0이고 두번째 비트는 1이다
	
	/*
	
		✨만약 0x0110 1001이라는 문자열이 들어왔다면 (뭔지는 모른다)
	
	
		t[0] = 0x0110 1101
		t[1] = 0x0110 1001
		t[2] = 0x1001 1101
		t[3] = 0x1001 1001
		
		이런식으로 변할 것이다.
		밑에 보내는 함수는 똑같다.
	
	
	*/
	
	
	HAL_I2C_Master_Transmit(&hi2c2, i2cAddress,(uint8_t *) data_t, 4, 100);
}

void lcd_init(void)
{
	//✨LCD를 시작할 때 쓰려는 명령어인 것 같다. 아마 main에서 호출될 것으로 추측된다
	
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms //✨50ms 기다렸다
	lcd_send_cmd(0x30); //✨0x30을 명령어로 보낸다. 3은 0011이니, 0x0011 1100, 0x0011 1000, 0x0000 1100, 0x0000 1000 이런 문자열이 보내졌을 것이다
	HAL_Delay(10);  // wait for >4.1ms //✨10ms 기다렸다
	lcd_send_cmd(0x30); //✨위와 똑같은 명령어를 보낸다. 뭐지?
	HAL_Delay(10);  // wait for >100us //✨10ms 기다렸다
	lcd_send_cmd(0x30); //✨위와 똑같은 명령어를 보낸다.
	HAL_Delay(10); //✨ 10ms 기다렸다
	lcd_send_cmd(0x20);  // 4bit mode //✨앞의 주석은 4비트 모드를 말한다고 한다.
	HAL_Delay(10); //✨10ms 기다렸다

	// dislay initialisation
	_displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS; // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	//✨표시 함수는... 0x10 | 0x08 | 0x00 -> 0x18; (0b0001 1000);
	lcd_send_cmd(LCD_FUNCTIONSET | _displayfunction); //✨0x20 | 0x18 -> 0x38이 된다. (FUNCTION SET을 수행하고, 4비트 모드, 2라인이고, 5x8도트)
	HAL_Delay(10); //✨10ms 기다리기

	_displaymode = LCD_ENTRYDDRAMINCREMENT; //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift) ✨엔트리 DDRAM 증가를 디스플레이 모드로 설정한다
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode); //✨이 함수는 entry mode set인듯? 값은 entry DDRAM 증가로 넣은듯
	HAL_Delay(10); //✨10ms 기다리기

	_displaycontrol = LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON; //Display on/off control --> D=1, C=1, B=1 //✨Display 켜기, Cursor 켜기, Blink 켜기로 설정
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //✨디스플레이 설정을 Display Control에 넣어서 보낸다
	HAL_Delay(10); //✨10ms 기다리기

	lcd_send_cmd(LCD_CLEARDISPLAY); // clear display //✨Clear Display 수행. 이전에 있는 값을 지우려는 걸 수도 있겠다
	HAL_Delay(100); //✨100ms 기다리기
}

void lcd_send_string(char *str)
{
	while (*str) lcd_send_data(*str++); //✨sendString이 시작되면 파라메터로 받은 문자열 값이 비어버릴 때 까지 데이터를 보내고 값을 밀길 반복한다. 값이 다 밀려서 없어지면 끝.
}

void lcd_clear(void)
{
	//✨clear display 수행하는듯
	lcd_send_cmd(LCD_CLEARDISPLAY);
}

void lcd_home(void)
{
	//✨return home을 수행하는듯
	lcd_send_cmd(LCD_RETURNHOME);
}

void lcd_setCursor(uint8_t row, uint8_t col)
{
	//✨줄과 열을 받아옴... 저기로 가고싶은듯?
	if(((row >= 0) && (row <= 1)) && ((col >= 0) && (col < 16))){
		//✨row가 0이나 1이라면... 당연하다
		//✨col이 0에서 16사이라면... 이것도 당연하다
		if(row == 0){
			//✨row가 0이라면
			lcd_send_cmd(LCD_SETDDRAMADDR | (0x00 + col)); //✨DDRAM address를 0x0{COL}으로 설정. 첫째줄이 그렇다
		}else if(row == 1){
			//✨row가 1이라면
			lcd_send_cmd(LCD_SETDDRAMADDR | (0x40 + col)); //✨DDRAM address를 0x4{COL}으로 설정. 둘째줄이 그렇다
		}
	}
}

void lcd_characterDatainput(void) // a~z data
{
	  //printf("%c\r\n",clcdData[0]);

	  lcd_send_data(clcdData[0]); //✨clcdData의 첫번째 문자를 보낸다. 명령 없이.
	  if('z' == clcdData[0]){ //✨clcdData[0]가 z라면
		  clcdData[0] = 'a'; //✨clcdData의 첫번째 문자열은 다시 0이다
	  }else{
			//✨z가 아니면
		  clcdData[0] += 1; //✨clcdData의 첫번째 문자열에 1을 더해서 반복한다
	  }
}

void lcd_createSpecialCharacter(uint8_t location, uint8_t charmap[]) {
	//✨특문 전송용 함수인듯. location은 위치고 charmap은 배열이다... skull등은 배열로 이루어져있다.
	location &= 0x7; 				// we only have 8 locations 0-7
	//✨location에 0x7을 Bitwise AND한다... 0x?X라고 치면 맨 오른쪽만 살린다는 의미인듯
	lcd_send_cmd(LCD_SETCGRAMADDR | (location << 3)); //✨명령어를 보낸다... SET CGRAMADDR인데... location(주소값)을 3칸 왼쪽으로 민다... 맨 오른쪽 3비트의 주소는 필요 없기 때문일까?
	for (int i = 0; i < 8; i++) { //✨i=0부터 7까지 반복
		lcd_send_data(charmap[i]); //✨데이터를 보낸다... charmap의 0부터 7까지...
	}
}

//----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display
void lcd_incrementDDRAM(void) // cmd : d
{
	_displaymode = LCD_ENTRYDDRAMINCREMENT;	//0x02 //✨DDRAM 증가... 모든 디스플레이 이동이라는데... 밑에 있는 건 뭘까?
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode); //✨DDRAM 증가를 입력
}

void lcd_decrementDDRAM(void) // cmd : e
{
	//----- 코드 추가 -----
	_displaymode = LCD_ENTRYDDRAMDECREMENT;	//0x02 //💎DDRAM 감소 설정
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode); //💎DDRAM 감소 전송
}

void lcd_shiftEntiredisplayLeft(void)  // cmd : f
{
	//----- 코드 추가 -----
	_displaymode = LCD_ENTRYDISPLAYSHIFTLEFT;	//0x02 //💎ENTRY DIsplay Shift Left 설정
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode); //💎ENTRY MODE SET 명령어로 전송
}

void lcd_shiftEntiredisplayRight(void) // cmd : g
{
	//----- 코드 추가 -----
	_displaymode = LCD_ENTRYDISPLAYSHIFTRIGHT;	//0x02 //💎ENTRY DIsplay Shift RIght 설정
	lcd_send_cmd(LCD_ENTRYMODESET | _displaymode); //💎ENTRY MODE SET 명령어로 전송
}
//----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display

//----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----
void lcd_display(void) // cmd : m
{
	_displaycontrol |= LCD_DISPLAYON; // 0x04 //✨켜기
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //✨디스플레이를 켜는듯
}

void lcd_nodisplay(void) // cmd : n
{
	//----- 코드 추가 -----
	_displaycontrol &= (0xFF ^ LCD_DISPLAYON); // 0x04 //💎0xFF를 Display On과 XOR 시키면 그 비트만 사라진다... 그걸 원하는 값에 masking 하면 그 부분만 지우고 나머지를 살릴 수 있다.
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //💎디스플레이 컨트롤에 값 전송
}

void lcd_cursor(void) // cmd : o
{
	_displaycontrol |= LCD_CURSORON; //💎위와 같다. 켜서 켜기
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //💎전송
	//----- 코드 추가 -----
}

void lcd_nocursor(void) // cmd : p
{
	_displaycontrol &= (0xFF ^ LCD_CURSORON); //💎위의 반대. 커서 끄기
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //💎전송
	//----- 코드 추가 -----
}

void lcd_blink(void) // cmd : q
{
	//----- 코드 추가 -----
	_displaycontrol |= LCD_BLINKON; //💎위의 반대. 커서 끄기
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //💎전송
}

void lcd_noblink(void) // cmd : r
{
	//----- 코드 추가 -----
	_displaycontrol &= (0xFF ^ LCD_BLINKON); //💎위의 반대. 커서 끄기
	lcd_send_cmd(LCD_DISPLAYCONTROL | _displaycontrol); //💎전송
}
//----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----

//----- LCD_CURSORSHIFT : cursor & dispaly shift -----
void lcd_shiftCursorLeft(void) // cmd : s
{
	_cursordisplayshift = LCD_SHIFTCURSORLEFTDECRESE ; // 0x00 //✨_cursordisplayshift는 커서가 왼쪽으로 감소되는 값을 받았다
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift); //✨명령은 0x10... 저 값이 커서를 움직이게 하는 명령어인듯. 뒤에는 사실상 파라메터로 보인다
}

void lcd_shiftCursorRight(void) // cmd : t
{
	//----- 코드 추가 -----
	
	_cursordisplayshift = LCD_SHIFTCURSORRIGHTINCRESE; //💎명령은 Shift Cursor Right Increase... 오른쪽으로 상승하는 값
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift); //💎합친 명령어 전송
}

void lcd_shiftAllDisplayLeft(void) // cmd : u
{ //🎈오류 날 수 있음
	//----- 코드 추가 -----
	_cursordisplayshift = LCD_SHIFTALLDISPLAYLEFTCURSORLEFT; //💎명령은 Shift All Display Left Cursor Left...
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift); //💎합친 명령어 전송
	//⏰문제로 예상되는 부분... LCD_CURSORSHIFT가 과연 DISPLAY 방향까지 관여할까?...
	//⏰하지만 예상컨데 _cursordisplayshift가 계속 바뀌고 변수 자체에 display와 cursor가 다 들어가있는 걸로 미루어보아 둘다 관여할 가능성이 높다.
}

void lcd_shiftAllDisplayRight(void) // cmd : v
{ //🎈오류 날 수 있음
	//----- 코드 추가 -----
	_cursordisplayshift = LCD_SHIFTALLDISPLAYLEFTCURSORRIGHT; //💎명령은 Shift All Display Left Cursor Right...
	lcd_send_cmd(LCD_CURSORSHIFT | _cursordisplayshift); //💎합친 명령어 전송
	
	//⏰문제로 예상되는 부분... LCD_CURSORSHIFT가 과연 DISPLAY 방향까지 관여할까?...
	//⏰하지만 예상컨데 _cursordisplayshift가 계속 바뀌고 변수 자체에 display와 cursor가 다 들어가있는 걸로 미루어보아 둘다 관여할 가능성이 높다.
	
}
//----- LCD_CURSORSHIFT : cursor & dispaly shift -----

void lcd_address_search(void)
{
	//✨주소 찾기 굳이 처음에 수행하지는 않는다. 왜일까?
	HAL_StatusTypeDef i2cstatus; //✨i2cstatus 상태를 정의한다... 초기기 때문에 값이 없을 것이다

	for(uint16_t i = 0; i < 128; i++){ //✨i는 0부터 127까지 반복된다...
		i2cstatus = HAL_I2C_IsDeviceReady(&hi2c2, i << 1, 1, 10); //✨I2C Device Ready 함수에 i를 1부터 128비트까지 밀어넣어본다... 왜 굳이 값을 다르게 하는걸까?
		if(HAL_OK == i2cstatus){ //✨위에서 Ready가 HAL_OK로 반환되었다면
		  printf("OK addr=0x%02x\r\n",(i << 1)); //✨OK addr = {ADDR}로 받아온다... 여기서 알겠다.
			//✨아마도 I2C의 주소는 0부터 127까지 있을 것이다. 그중에서 반응하는 주소를 찾는 것. 여기서 찾은 i가 주소가 된다
		  break; //✨브레이크가 있는 게 좋을 것 같긴한데... 없어도 되려나? 그래봐야 여러 주소를 다 쓸 수 있을지 모르겠다.
			
			//✨브레이크는 필요없을시 삭제
		}else{
		  //printf("err addr=0x%02x\r\n",(i << 1));
		}
	}
	
	printf("lcd_address_search\r\n"); //✨LCD 주소를 찾은 것에 대한 축포를 쏜다
}

//----- character LCD ------------------------------------------------------------------


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	HAL_StatusTypeDef uart2status; //✨uart 상태를 받아옴

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init(); //✨HAL 시작

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
  /* USER CODE BEGIN 2 */

  printf("---character LCD(i2c) test---\r\n"); //✨LCD 잘된다고 출력. 실제로는 코드 작동 여부 확인

  HAL_Delay(10); //✨10ms 기다림
  lcd_init(); //✨위에서 선언된 초기 세팅 수행
  HAL_Delay(20); //✨20ms 기다림
  sprintf(clcdData,"chd"); //✨clcdData에 "chd"를 집어넣는다. 
  lcd_send_string(clcdData); //✨lcd에 문자열을 전송한다
  HAL_Delay(20); //✨20ms 기다린다
  clcdData[0] = 'a'; //✨첫번째 문자값을 a로 바꾼다. 값이 ahd가 될듯

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { //✨while의 switch는 urx를 한글자만 받는다는 조건 하에 다 만들어진 것으로 추측된다... 아마도?
		//✨z가 없긴하다 지금 보니까 없는 글자가 많긴 하다
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  uart2status = HAL_UART_Receive(&huart2, urx, 1, 300); //✨uart로 값을 하나 받아옵니다
	  if(HAL_OK == uart2status){ //✨uart가 작동한다면
		  printf("*%c*\r\n",urx[0]); //✨uart에 들어온 값을 모니터에 출력한다
		  switch(urx[0]){ //✨받은 값을 switch문으로 넘긴다
		  case '1': //🥩1이라면
			  lcd_address_search(); //✨주소 검색을 수행한다
			  break;
		  case 'c': //🥩c라면
			  lcd_clear(); //✨lcd 초기화
			  break;
		  case 'h': //🥩h라면
			  lcd_home(); //✨lcd 커서를 홈으로 보낸다
			  break;
		  case 'i': //🥩i라면...?
			  lcd_init(); //✨lcd init을 다시 수행한다. 아마도 세팅이 망가지면 한 번 수행하면 좋을 듯 보인다
			  sprintf(clcdData,"chd"); //✨clcdData에 "chd"를 넣는다
			  lcd_send_string(clcdData); //✨clcdData를 보낸다.
			  clcdData[0] = 'a'; //✨clcdData의 첫글자를 a로 바꾼다. main 처음 함수랑 똑같다. 흠.
			  break;
		  case 'a': //🥩a라면... 아마 이건 a가 첫째줄 b가 둘째줄을 말하는듯
			  lcd_setCursor(0, 0); //✨커서를 첫째줄의 첫째로 이동시킨다
			  break;
		  case 'b': //🥩b라면
			  lcd_setCursor(1, 0); //✨커서를 두번째줄의 첫째로 이동시킨다
			  break;
		  case 'k': //🥩k라면
			  lcd_characterDatainput(); //✨문자 데이터 받기 ❓ 이건 뭘까
			  break;
		  //----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display
		  case 'd': //🥩d라면
			  lcd_incrementDDRAM(); //✨DDRAM 증가... 뭐임??
			  break;
		  case 'e': //🥩e라면
			  lcd_decrementDDRAM(); //✨DDRAM 감소.
			  break;
		  case 'f': //🥩f라면
			  lcd_shiftEntiredisplayLeft(); //✨전체를 왼쪽으로
			  break;
		  case 'g': //🥩g라면
			  lcd_shiftEntiredisplayRight(); //✨전체를 오른쪽으로... 아마도?
			  break;
		  //----- LCD_ENTRYMODESET : inc/dec DDRAM address, shift of entire display
		  //----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----
		  case 'm': //🥩m이라면
			  lcd_display(); //✨lcd display를 켜기?
			  break;
		  case 'n': //🥩n이라면
			  lcd_nodisplay(); //✨lcd display를 끄기인듯
			  break;
		  case 'o': //🥩o라면
			  lcd_cursor(); //✨커서를 켜는듯?
			  break;
		  case 'p': //🥩p라면
			  lcd_nocursor(); //✨커서를 끄는듯
			  break;
		  case 'q': //🥩q라면
			  lcd_blink(); //✨깜빡이를 켠다
			  break;
		  case 'r': //🥩r이라면
			  lcd_noblink(); //✨깜빡이를 끈다
			  break;
		  //----- LCD_DISPLAYCONTROL : display on/off cursor, blink -----
		  //----- LCD_CURSORSHIFT : cursor & dispaly shift -----
		  case 's': //🥩s라면
			  lcd_shiftCursorLeft(); //✨커서를 왼쪽으로
			  break;
		  case 't': //🥩t라면
			  lcd_shiftCursorRight(); //✨커서를 오른쪽으로
			  break;
		  case 'u': //🥩u라면
			  lcd_shiftAllDisplayLeft(); //✨디스플레이를 왼쪽으로
			  break;
		  case 'v': //🥩v라면
			  lcd_shiftAllDisplayRight(); //✨디스플레이를 오른쪽으로
			  break;
		  //----- LCD_CURSORSHIFT : cursor & dispaly shift -----
		  //----- special character -----
		  case 'x': //🥩x라면
			  lcd_createSpecialCharacter(0, Heart); //✨특별한 문자 생성... 0? 하트
			  lcd_createSpecialCharacter(1, Bell); //✨특별문자생성... 1 벨
			  lcd_createSpecialCharacter(2, Alien); //✨특문생성... 2 에일리언
			  lcd_createSpecialCharacter(3, Check); //✨특문 ... 3 체크표시
			  lcd_createSpecialCharacter(4, Speaker); //✨특문 ... 4 스피커
			  lcd_createSpecialCharacter(5, Sound); //✨특문 ... 5 소리? 음표
			  lcd_createSpecialCharacter(6, Skull); //✨특문 ... 해골
			  lcd_createSpecialCharacter(7, Lock); //✨특문 ... 자물쇠
			  break;
		  case 'y': //🥩y라면
			  lcd_setCursor(1, 0); lcd_send_data(0); //✨아랫줄을 다 지워버리는건가? 커서를 아래 첫째줄로 이동 후 0을 전송
			  lcd_setCursor(1, 1); lcd_send_data(1); //✨"
			  lcd_setCursor(1, 2); lcd_send_data(2); //✨" 위에서 확인하고 온 결과, data전송 명령어는 설정된 주소에 데이터를 쓰는 것을 말한다. 숫자가 간다. (ㅇㅎ 지우는게 아니라 숫자인듯)
			  lcd_setCursor(1, 3); lcd_send_data(3); //✨"
			  lcd_setCursor(1, 4); lcd_send_data(4); //✨"
			  lcd_setCursor(1, 5); lcd_send_data(5); //✨"
			  lcd_setCursor(1, 6); lcd_send_data(6); //✨"
			  lcd_setCursor(1, 7); lcd_send_data(7); //✨"
			  break;
			  //----- special character ----- //✨z가 없네?
		  default: //✨아무것도 아니라면
			  break; //✨걍 암것도 하지 말자
		  }
	  }

	  //pc13pin = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
   	  //if(GPIO_PIN_RESET == pc13pin){
   	  //} //✨이건 왜있는거지? 일단 쓸데 없어보임

	  HAL_Delay(100); //✨100ms 기다림
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); //✨LED 토글. 결과적으로 반짝거림


  }
  /* USER CODE END 3 */
}