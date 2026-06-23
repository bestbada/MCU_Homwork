////////////////////////////////////////////////////////////
// 과제명: HW3. RCC 제어
// 과제개요: SW 입력에 따라 SYSCLK가 달라지며, 달라진 SYSCLK를 LCD와 LED의 움직임을 통해 확인함
// 
// 사용한 하드웨어(기능): GPIO, RCC, GLCD
// 제출일: 2026. 5. 21
// 이름: 이주현
///////////////////////////////////////////////////////////////

//**************************************************************************//
// 	LCD_Init()(calling in GLCD_Display.c, called in GLCD.c)
//		ILI_Init() in calling in GLCD.c, called in ILI3168B.c
//			FSMC_GPIO_Config() in ILI3168B.c   
//			FSMC_LCD_Init() in ILI3168B.c
//	DisplayInitScreen();

#include "stm32f4xx.h"		// stm32F4 시리즈 MCU의 핵심적인 Hardware(특히 Register) 정의 및 초기화 header file
#include "GLCD.h"		// GLCD 초기화 및 주요 라이브러리 함수 선언

void _GPIO_Init(void);		// GPIO port 초기화 루틴
uint16_t KEY_Scan(void);	// Switch array(Key)중 어떤 switch가 눌렸는지 파악하여 출력하는 루틴

void BEEP(void);			// 부저 소리 발생하는 루틴
void DelayMS(unsigned short wMS);	// 시간 지연 함수(ms 단위)
void DelayUS(unsigned short wUS);		// 시간 지연 함수(us 단위)
void DisplayInitScreen(void);	// GLCD 초기 화면 실행 루틴

void RCC_HSI_16M(void);				//SYSCLK HSI 16M으로 설정하는 루틴 
void RCC_HSE_8M(void);				//SYSCLK HSE 8M으로 설정하는 루틴 
void RCC_PLL_HSI_50M(void);		//SYSCLK PLL HSI 150M으로 설정하는 루틴 
void RCC_PLL_HSE_100M(void);	//SYSCLK PLL HSE 100M으로 설정하는 루틴 
void RCC_PLL_HSE_168M(void);	//SYSCLK PLL HSE 168M으로 설정하는 루틴 

uint8_t ShiftFlag;

int main(void)
{
	_GPIO_Init(); 	// GPIO (LED, SW, Buzzer) 초기화
	GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
	LCD_Init();		// GLCD 모듈 초기화
	DelayMS(10);	// GLCD 안정화를 위한 delay 시간(필수 아님)
	BEEP();		// 시작을 음성으로 알림
	DisplayInitScreen();	// GLCD 초기화면
	RCC_PLL_HSE_168M(); //초기에 SYSCLK 168M로 설정

	while(1)
	{
		ShiftFlag++;								//0.1초 간격으로 ShiftFlag 1씩 증가
		if (ShiftFlag >= 8)						//LED7 켜지면 ShiftFlag 0으로 설정하여 LED0부터 다시 반복 
			ShiftFlag= 0;
		GPIOG->ODR = 1<< ShiftFlag ;	// LED shifting bit by bit
		DelayMS(100);							//0.1초 간격으로 반복
		
		switch(KEY_Scan())	// 입력된 Switch 정보 분류 
		{
			case 0xF700 : 	//SW3 입력
			{
				DisplayInitScreen();								// GLCD 초기화
				LCD_SetTextColor(RGB_RED);				//글자색 : 빨간색
				LCD_DisplayText(1,4,"HSI(16M)");			//LCD의 HSI(16M) 빨간색으로 변경
				RCC_HSI_16M();									//클록 주파수 HSI 16M으로 변경
				BEEP();												//부저 소리 발생
			}
			break;
			case 0xEF00 :	//SW4 입력
			{
				DisplayInitScreen();								// GLCD 초기화
				LCD_SetTextColor(RGB_RED);				//글자색 : 빨간색
				LCD_DisplayText(2,4,"HSE(8M)");			//LCD의 HSE(8M) 빨간색으로 변경
				RCC_HSE_8M();									//클록 주파수 HSE 8M으로 변경
				BEEP();												//부저 소리 발생
			}
			break;
			case 0xDF00 :	//SW5 입력
			{
				DisplayInitScreen();								// GLCD 초기화
				LCD_SetTextColor(RGB_RED);				//글자색 : 빨간색
				LCD_DisplayText(3,4,"PLL(HSI50M)");	//LCD의 PLL(HSI50M) 빨간색으로 변경
				RCC_PLL_HSI_50M();							//클록 주파수 PLL HSI 50M으로 변경
				BEEP();												//부저 소리 발생
			}
			break;
			case 0xBF00 :	//SW6 입력
			{
				DisplayInitScreen();								// GLCD 초기화
				LCD_SetTextColor(RGB_RED);				//글자색 : 빨간색
				LCD_DisplayText(4,4,"PLL(HSE100M)");//LCD의 PLL(HSE100M) 빨간색으로 변경
				RCC_PLL_HSE_100M();						//클록 주파수 PLL HSE 100M으로 변경
				BEEP();												//부저 소리 발생
			}
			break;
		}  // switch(KEY_Scan())
	}  // while(1)
}

void RCC_HSI_16M(void)	
	// SYSCLK source: HSI
	// SYSCLK frequency: 16MHz
{
	// RCC Clock 변경 : 16MHz
 	RCC->CR |= 0x0001; // HSI ON
	RCC->CFGR &= ~0x0003;  // SYSCLK SW : HSI
}

void RCC_HSE_8M(void)	
	// SYSCLK source: HSE
	// SYSCLK frequency: 8MHz
{
	// RCC Clock 변경 : 8MHz
 	RCC->CR |= (1<<16); // HSE ON
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI)
	RCC->CFGR |= 0x0001;  // SYSCLK SW : HSE
}

void RCC_PLL_HSI_50M(void)  
	// SYSCLK source: PLLCLK, PLL source: HSI
	// SYSCLK frequency: 50MHz
{
	// RCC Clock 변경 : 50MHz
 	RCC->CR |= 0x0001; // HSI ON
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI) : 

	RCC->CR &= ~0x05000000;  // PLL OFF
	RCC->PLLCFGR &= ~0xFFFFF ;  // 기존 PLL_P, PLL_N,PLL_M clear 
	// PLL source: HSI(16MHz), M=8, N=200, P=8   ---> 16M*200/8/8= 50MHz
	RCC->PLLCFGR |= 8 | (200 << 6) | (((8 >> 1) -1) << 16) ;
	RCC->PLLCFGR &= ~(1<<22);	// PLL source: HSI
	RCC->CR |= 0x05000000; // PLL ON
   
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI)
	RCC->CFGR |= 0x0002;  // SYSCLK SW : PLLCLK
}

void RCC_PLL_HSE_100M(void)  
	// SYSCLK source: PLLCLK, PLL source: HSE
	// SYSCLK frequency: 100MHz
{
	// RCC Clock 변경 : 100MHz
 	RCC->CR |= (1<<16); // HSE ON
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI) : 

	RCC->CR &= ~0x05000000;  // PLL OFF
	RCC->PLLCFGR &= ~0xFFFFF ;  // 기존 PLL_P, PLL_N,PLL_M clear 
	// PLL source: HSE(8MHz), M=8, N=200, P=2   ---> 8M*200/2/8= 100MHz
	RCC->PLLCFGR |= 8 | (200 << 6) | (((2 >> 1) -1) << 16) ;
	RCC->PLLCFGR |= (1<<22);	// PLL source: HSE 
	RCC->CR |= 0x05000000; // PLL ON
   
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI)
	RCC->CFGR |= 0x0002;  // SYSCLK SW : PLLCLK
}

void RCC_PLL_HSE_168M(void)  
	// SYSCLK source: PLLCLK, PLL source: HSE
	// SYSCLK frequency: 168MHz
{
	// RCC Clock 변경 : 168MHz
 	RCC->CR |= (1<<16); // HSE ON
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI) : 

	RCC->CR &= ~0x05000000;  // PLL OFF
	RCC->PLLCFGR &= ~0xFFFFF ;  // 기존 PLL_P, PLL_N,PLL_M clear 
	// PLL source: HSE(8MHz), M=8, N=336, P=2   ---> 8M*336/2/8= 168MHz
	RCC->PLLCFGR |= 8 | (336 << 6) | (((2 >> 1) -1) << 16) ;
	RCC->PLLCFGR |= (1<<22);	// PLL source: HSE 
	RCC->CR |= 0x05000000; // PLL ON
   
	RCC->CFGR &= ~0x0003;  // SYSCLK SW  Clear(HSI)
	RCC->CFGR |= 0x0002;  // SYSCLK SW : PLLCLK, MUX스위치
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정	*/
void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
	RCC->AHB1ENR	|=  0x00000040;		// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER &= 0xFFFF0000;		// GPIOG 0~7 : Clear (0b00)			
	GPIOG->MODER |=  0x00005555;		// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;		// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR &= ~0x0000FFFF;	// GPIOG 0~7 : Clear (0b00)		 	
	GPIOG->OSPEEDR |=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR	|=  0x00000080;		// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER &= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR &= ~0xFFFF0000;		// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
	RCC->AHB1ENR	|=  0x00000020; 		// RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	&= ~0x000C0000;	// GPIOF 9 : Clear (0b00)
	GPIOF->MODER 	|=    0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;		// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR &= ~0x000C0000;	// GPIOF 9 : Clear (0b00)
 	GPIOF->OSPEEDR |=  0x00040000;		// GPIOF 9 : Output speed 25MHZ Medium speed 
}	

/* GLCD 초기화면 표시 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		// 화면 클리어
	LCD_SetFont(&Gulim8);			// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
	LCD_DisplayText(0,0,"RCC:2022130026 LJH");	// Title

	LCD_SetBackColor(RGB_WHITE);	//글자배경색 : White
 	LCD_DisplayText(1,0,"SW3:");		//SW3에 대한 Title
	LCD_DisplayText(2,0,"SW4:");		//SW4에 대한 Title
	LCD_DisplayText(3,0,"SW5:");		//SW5에 대한 Title
	LCD_DisplayText(4,0,"SW6:");		//SW6에 대한 Title
	
	LCD_SetTextColor(RGB_BLUE);			// 글자색 : Blue
	LCD_DisplayText(1,4,"HSI(16M)");			//초기 HSI(16M) 파란색으로 출력
	LCD_DisplayText(2,4,"HSE(8M)");			//초기 HSE(8M) 파란색으로 출력
	LCD_DisplayText(3,4,"PLL(HSI50M)");	//초기 PLL(HSI50M) 파란색으로 출력
	LCD_DisplayText(4,4,"PLL(HSE100M)");//초기 PLL(HSE100M) 파란색으로 출력
}

/* Switch array(key: SW0 - SW7)중 switch 입력여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
/* 추가로 한번 누른 swtich에 손을 떼지 않고 계속(?) 누르고 있더라도 한번 누른 것으로 판단하는 기능이 있음 */ 
uint8_t key_flag = 0;	// Switch 입력(pressed) 여부를 저장하는 변수, '1': 입력(pressed)
uint16_t KEY_Scan(void)	 
{ 
	uint16_t key;	// Switch가 연결되어 있는 PORTH의 8~15번pin(IDR의 8~15번 비트)으로 부터 들어어는 input값을 저장
				// input값 '0': 해당 switch가 눌림, '1': 눌리지 않음
	key = GPIOH->IDR & 0xFF00;	// 0xFF00(MASK) : IDR의 8~15번 비트(PORTH의 8~15번 pin 정보 저장) 정보만 유지하고, 필요없는 그외 비트들은 '0'과 AND 연산하여 보이지 않게 막는 역할
						//   			 8개의 switch중 어떤 switch가 눌렸는지 파악하기 위해 경우의 수를 줄이는 역할(MASK를 하지 않으면 IDR의 값의 경우의 수는 2^16 임)  
// Any switch pressed ?
	if (key == 0xFF00)	// TRUE: No switch is pressed
	{  	if (key_flag == 0)		// 어떤 switch도 누르지 않은 상태
			return key;		// 0xFF00 을 return 
		else				// 어떤 switch가 눌려졌다가 손을 뗀 직후 상태
		{	DelayMS(2);		// switch의 누른 상태에서 해제되었을 경우에 debounce 현상(switch 내부의 스프링 탄성으로 인한 현상)을 감쇄하기 위한 delay time
			key_flag = 0;	// 눌려진 switch가 없다는 상태를 저장
			return key;		// 0xFF00 을 return 
		}
	}				
  	else				// FALSE: Some switch are pressed
	{	if (key_flag != 0)		// key_flag=1: 이미 어떤 switch가 입력된 상태를 의미
			return 0xFF00;	// 실제로 어떤 switch가 눌려지고 있는 상태임에도 불구하고, 입력된 switch가 없다고 return (손을 떼지 않으면 한 번 누른 것으로 간주하지 위해서)
 		else				// key_flag=0: 이 문장실행전까지 어떤 switch도 눌려지지 않았고, 새로운 switch가 방금 눌려진 상태임을 의미
		{	key_flag = 1;	// 어떤 switch가 새롭게 눌려진 상태임을 저장
 			return key;		// 현재의 key 상태( IDR.8~15 ) 정보를 return 
						//   (예) key= 0xFE00(0b1111 1110 0000 0000) : SW0 누름
						//   (예) key= 0xFD00(0b1111 1101 0000 0000) : SW1 누름 .....
						//   (예) key= 0x7F00(0b0111 1111 0000 0000) : SW7 누름
		}
	}
}

/* 부저 소리 발생하는 루틴 */
/* Buzzer: Beep for 30 ms 
	소리크기 변경: DelayMS 시간 조정  */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' : Buzzer on
	DelayMS(30);			// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' : Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}
