////////////////////////////////////////////////////////////
// 과제명: TP. 엘리베이터 제어기
// 과제개요: 목표층을 입력하고 동작을 실행하면 엘리베이터가 목표층까지 이동하는 프로그램
// EXTI를 사용하여 NAVI와 SW의 입력을 받음
// 사용한 하드웨어(기능): GPIO, RCC, GLCD, EXTI, Joy-stick, FRAM 
// 제출일: 2026. 6. 6
// 이름: 이주현
///////////////////////////////////////////////////////////////

//*************************************************//
// EXTI 역할(EXTI7, 15)
// NAVI_DOWN(PI7) 입력: 입력: EXTI7 발생, 이동 중 Pause 모드 활성화, 부저 2회 후 3초 정지
// SW0(PH8) 입력: 부저 1회 후, LED0 ON, LCD의 FL에 B표시
// SW1(PH9) 입력: 부저 1회 후, LED1 ON, LCD의 FL에 1표시
// SW2(PH10) 입력: 부저 1회 후, LED2 ON, LCD의 FL에 2표시
// SW3(PH11) 입력: 부저 1회 후, LED3 ON, LCD의 FL에 3표시
// SW4(PH12) 입력: 부저 1회 후, LED4 ON, LCD의 FL에 4표시
// SW5(PH13) 입력: 부저 1회 후, LED5 ON, LCD의 FL에 5표시
// SW7(PH15) 입력: EXTI15 발생, 동작 모드 활성화, 부저 2회, 선택한 층으로 동작

#include "stm32f4xx.h"		// stm32F4 시리즈 MCU의 핵심적인 Hardware(특히 Register) 정의 및 초기화 header file
#include "GLCD.h"		// GLCD 초기화 및 주요 라이브러리 함수 선언
#include "FRAM.h"		// FRAM 초기화 및 주요 라이브러리 함수 선언

void _GPIO_Init(void);		// GPIO port 초기화 루틴
uint16_t KEY_Scan(void);	// Switch array(Key)중 어떤 switch가 눌렸는지 파악하여 출력하는 루틴
void BEEP(void);			// 부저 소리 발생하는 루틴
void DelayMS(unsigned short wMS);	// 시간 지연 함수(ms 단위)
void DelayUS(unsigned short wUS);		// 시간 지연 함수(us 단위)
void DisplayInitScreen(void);	// GLCD 초기 화면 실행 루틴
void _EXTI_Init(void);		// EXTI 초기화
void BarDraw(int start, int finish);		//이동하는 층에 따라 바를 늘리는 함수

unsigned char BNo, TNo, src_floor = 1, dst_floor=1;	//B와 5층에 도달한 횟수를 저장하는 변수, 현재 층수와 목표 층수를 저장하는 변수

int main(void)
{
	_GPIO_Init();	// GPIO (LED, SW, Buzzer, Navi_SW) 초기화
	LCD_Init();		// GLCD 모듈 초기화
	DelayMS(10);	// GLCD 안정화를 위한 delay 시간(필수 아님)
	_EXTI_Init();		// EXTI 초기화
	
	Fram_Init();			// FRAM 초기화 H/W 초기화
	Fram_Status_Config();	// FRAM 초기화 S/W 초기화
	
	BEEP();		//초기 상태 부저 1회
	GPIOG->ODR |= 0x0002;		//초기 LED1 ON
	if ( Fram_Read(605) > 9 || Fram_Read(605)<0)		// FRAM 605번지에서 데이터 read한 후, 10 이상(오류)이면 0으로 초기화 
		BNo= 0;
	else 
		BNo= Fram_Read(605);	//  FRAM 605번지에서 데이터 read (0~9)
 
	if ( Fram_Read(606) > 9 || Fram_Read(606)<0)		// FRAM 606번지에서 데이터 read한 후, 10이상(오류)이면 0으로 초기화 
		TNo= 0;
	else 
		TNo= Fram_Read(606);	//  FRAM 606번지에서 데이터 read (0~9)
	DisplayInitScreen();	// GLCD 초기화면
	
    while(1)
	{
		switch(KEY_Scan())
		{
			case 0xFE00 : 
				BEEP();	//스위치 입력 시 부저 1회
				if(src_floor != 0)	//현재 층과 동일한 층 입력시 무시
				{
					GPIOG->ODR &= 0x0000;		//모든 LED OFF
					dst_floor = 0;		//목표층 설정
					LCD_SetTextColor(RGB_RED);	//글자색 : RED
					LCD_DisplayText(2, 6, "B");		//목표 층 표시	
					GPIOG->ODR |= 0x0001;		//목표 층 LED ON
				}
				break;
			case 0xFD00 : 
				BEEP();	//스위치 입력 시 부저 1회
				if(src_floor != 1)	//현재 층과 동일한 층 입력시 무시
				{
					GPIOG->ODR &= 0x0000;		//모든 LED OFF
					dst_floor = 1;		//목표층 설정
					LCD_SetTextColor(RGB_RED);	//글자색 : RED
					LCD_DisplayText(2, 6, "1");		//목표 층 표시	
					GPIOG->ODR |= 0x0002;		//목표 층 LED ON
				}
				break;
			case 0xFB00 : 	
				BEEP();	//스위치 입력 시 부저 1회
				if(src_floor != 2)	//현재 층과 동일한 층 입력시 무시
				{
					GPIOG->ODR &= 0x0000;		//모든 LED OFF
					dst_floor = 2;		//목표층 설정
					LCD_SetTextColor(RGB_RED);	//글자색 : RED
					LCD_DisplayText(2, 6, "2");		//목표 층 표시	
					GPIOG->ODR |= 0x0004;		//목표 층 LED ON
				}
				break;
			case 0xF700 : 
				BEEP();	//스위치 입력 시 부저 1회
				if(src_floor != 3)	//현재 층과 동일한 층 입력시 무시
				{
					GPIOG->ODR &= 0x0000;		//모든 LED OFF
					dst_floor = 3;		//목표층 설정
					LCD_SetTextColor(RGB_RED);	//글자색 : RED
					LCD_DisplayText(2, 6, "3");		//목표 층 표시	
					GPIOG->ODR |= 0x0008;		//목표 층 LED ON
				}
				break;
			case 0xEF00 : 
				BEEP();	//스위치 입력 시 부저 1회
				if(src_floor != 4)	//현재 층과 동일한 층 입력시 무시
				{
					GPIOG->ODR &= 0x0000;		//모든 LED OFF
					dst_floor = 4;		//목표층 설정
					LCD_SetTextColor(RGB_RED);	//글자색 : RED
					LCD_DisplayText(2, 6, "4");		//목표 층 표시	
					GPIOG->ODR |= 0x0010;		//목표 층 LED ON
				}
				break;
			case 0xDF00 : 
				BEEP();	//스위치 입력 시 부저 1회
				if(src_floor != 5)	//현재 층과 동일한 층 입력시 무시
				{
					GPIOG->ODR &= 0x0000;		//모든 LED OFF
					dst_floor = 5;		//목표층 설정
					LCD_SetTextColor(RGB_RED);	//글자색 : RED
					LCD_DisplayText(2, 6, "5");		//목표 층 표시	
					GPIOG->ODR |= 0x0020;		//목표 층 LED ON
				}
				break;
			}
	}	
}

/* EXTI (EXTI7(GPIOI.7, NAVI_DOWN), EXT15(GPIOH15, SW7)) 초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->AHB1ENR 	|= 0x0100;	// RCC_AHB1ENR GPIOI Enable
	RCC->APB2ENR 	|= (1<<14);	// Enable System Configuration Controller Clock
	
	GPIOI->MODER &= ~0xC000;	// GPIOI 7 : Input mode (reset state)	

	SYSCFG->EXTICR[1] |= 0x8000;	// EXTI7에 대한 소스 입력은 GPIOI로 설정
					// EXTI7 <- PI7 
					// EXTICR2(EXTICR[1])를 이용 
					// reset value: 0x0000	
	SYSCFG->EXTICR[3] |= 0x7000;	// EXTI15에 대한 소스 입력은 GPIOH로 설정
					// EXTI15 <- PH15 
					// EXTICR4(EXTICR[3])를 이용 
					// reset value: 0x0000	
	EXTI->FTSR |= 0x8080;		// EXTI7, 15: Falling Trigger Enable 
	EXTI->IMR  |= 0x8080;		// EXTI7, 15 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= ( 1<<23  );		// Enable 'Global Interrupt EXTI 7'
	NVIC->ISER[1] |= ( 1<<8  ); 			// Enable 'Global Interrupt EXTI 15'
					// Vector table Position 참조
	
	NVIC->IP[23]= 0xE0;  	// EXTI9_5 vector : High Priority (14) 
	NVIC->IP[40]= 0xF0;  	// EXTI15_10 vector : Low Priority (15)
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void) 
{	
	if(EXTI->PR & 0x0080)					// EXTI7 Interrupt Pending(발생) 여부
	{															//NAVI_DOWN시의 동작
		EXTI->PR |= 0x0080;						// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if (src_floor != dst_floor)
		{
			LCD_SetTextColor(RGB_RED);	//글자색 : RED
			LCD_DisplayText(1, 0, "P");		//Pause 모드 표시
			BEEP();		//부저 2회 0.5초 간격으로 반복
			DelayMS(500);
			BEEP();
			for(int i = 0; i < 3; i++)		//부저 3회 1초 간격으로 반복
			{
				DelayMS(1000);
				BEEP();
			}
			LCD_SetTextColor(RGB_RED);	//글자색 : RED
			LCD_DisplayText(1, 0, "M");	//Moving 모드 표시
		}
		
	}	
}

/* EXTI10~15 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	if(EXTI->PR & 0x8000)												// EXTI15 Interrupt Pending(발생) 여부?
	{																						//SW7 입력시 동작
		EXTI->PR |= 0x8000;													// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if(src_floor != dst_floor)		//현재 층과 목표 층 같을 때 입력 무시
		{
			GPIOG->ODR |= 0x0080;		//LED7 ON
			LCD_SetTextColor(RGB_RED);		//글자색 : RED
			LCD_DisplayText(1, 0, "M");		//Moving 모드 표시
			BEEP();		//부저 2회
			DelayMS(500);
			BEEP();
			if(src_floor<dst_floor)		//상승 시
			{
				LCD_SetTextColor(RGB_GREEN);	//글자색 : GREEN
				LCD_DisplayText(1, 1, ">");		//상승 방향 표시 문자
				for(int i = 0; i <= dst_floor-src_floor; i++)		//동작 Bar 우측으로 1초 간격으로 커짐
				{
					BarDraw(src_floor, src_floor + i);		
					DelayMS(1000);
				}
			}
			else if(src_floor>dst_floor)		//하강 시
			{
				LCD_SetTextColor(RGB_BLUE);	//글자색 : BLUE
				LCD_DisplayText(1, 1, "<");		//하강 방향 표시 문자
				for(int i = 0; i <= src_floor - dst_floor; i++)		//동작 Bar 좌측으로 1초 간격으로 커짐
				{
					BarDraw(src_floor, src_floor-i);
					DelayMS(1000);
				}
			}
			if(dst_floor == 0)		//목표층 B일 때 BNo에 +1
			{
				BNo++;
				if(BNo > 9)		//만약 BNo 10이상일 때 0으로 초기화
					BNo = 0;
				Fram_Write(605, BNo);		//BNo값 FRAM에 저장
			}	
			if(dst_floor == 5)		//목표층 5일 때 TNo에 +1
			{
				TNo++;		
				if(TNo > 9)		//만약 TNo 10이상일 때 0으로 초기화
					TNo = 0;
				Fram_Write(606, TNo);		//TNo값 FRAM에 저장
			}
			LCD_SetTextColor(RGB_RED);	//글자색 : RED
			LCD_DisplayChar(1, 7, BNo + '0'); 	//BNo값 표시
			LCD_DisplayChar(1, 13, TNo + '0'); 	//5No값 표시	
			for(int i = 0; i < 3; i++)	//부저 3번 반복
			{
				BEEP();
				DelayMS(1000);
			}
			src_floor = dst_floor;	//현재 층과 목표 층 일치
			LCD_DisplayText(1, 0, "S ");	//Stop 모드 표시, 부등호 초기화
			GPIOG->ODR &= ~0x0080;	//LED7 OFF
		}
	}
}

void BarDraw(int start, int finish)	//이동하는 층에 따라 바를 늘리는 함수
{
	LCD_SetBrushColor(RGB_YELLOW);	//브러쉬색 : YELLOW
	LCD_DrawFillRect(0, 40, 190, 4); 	//기존 바 삭제
	if(finish >= start && dst_floor > src_floor)	//목표층이 현재 층보다 높을 때(상승)
	{
		LCD_SetBrushColor(RGB_GREEN);	//브러쉬색 : GREEN
		LCD_DrawFillRect(64+16*start, 40, 8+16*(finish-start), 4);		//시작층과 목표층으로 바 길이 조절 
	}
	else if(finish <= start && dst_floor < src_floor)	//목표층이 현재 층보다 낮을 때(하강)
	{
		LCD_SetBrushColor(RGB_BLUE);		//브러쉬색 : BLUE
		LCD_DrawFillRect(64+16*finish, 40, 8 + 16*(start-finish), 4);		//시작층과 목표층으로 바 길이 조절 
	}
}

/* GLCD 초기화면 표시 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_YELLOW);	// 화면 클리어
	LCD_SetFont(&Gulim8);		// 폰트 : 굴림 8
	LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : YELLOW
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
	LCD_DisplayText(0, 0, "ELV: 2022130026 LJH");  	// Title
	LCD_DisplayText(1, 3, "BNO:  5NO:");	//BNo, 5No 값 표시 배경 
	LCD_DisplayText(2, 3, "FL:  B 1 2 3 4 5");	//층 표시 배경 
	LCD_SetTextColor(RGB_RED);	//글자색 : RED
	LCD_DisplayText(1, 0, "S");	//초기 Stop 모드 표시
	LCD_DisplayText(2, 6, "1");	//초기 1층 설정
	LCD_DisplayChar(1, 7, BNo + '0');	//BNo 현재 값 표시
	LCD_DisplayChar(1, 13, TNo + '0');	//5No 현재 값 표시 
	LCD_SetBrushColor(RGB_GREEN);	//브러쉬색 : GREEN
	LCD_DrawFillRect(80, 40, 8, 4); 	//1 아래에 초록색 바 생성
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

/* Switch array(key: SW0 - SW7)중 switch 입력여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
/* 추가로 한번 누른 swtich에 손을 떼지 않고 계속(?) 누르고 있더라도 한번 누른 것으로 판단하는 기능이 있음 */ 
uint8_t key_flag = 0;	// Switch 입력(pressed) 여부를 저장하는 변수, '1': 입력(pressed)
uint16_t KEY_Scan(void)	 
{ 
	uint16_t key;	// Switch가 연결되어 있는 PORTH의 8~15번pin(IDR의 8~15번 비트)으로 부터 들어어는 input값을 저장
				// input값 '0': 해당 switch가 눌림, '1': 눌리지 않음
	key = GPIOH->IDR & 0xFF00;	// 0xFF00(MASK) :
 
// Any switch pressed ?
	if (key == 0xFF00)	// TRUE: No switch is pressed
	{	if (key_flag == 0)		// 어떤 switch도 누르지 않은 상태
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
		}
	}
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
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
