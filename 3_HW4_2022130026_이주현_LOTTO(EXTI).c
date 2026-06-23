////////////////////////////////////////////////////////////
// 과제명: HW4. LOTTO(EXTI)
// 과제개요: Lotto머신에서 숫자를 뽑아 4개의 번호를 추첨함. 
// EXTI를 사용하여 NAVI와 SW의 입력을 받음
// 사용한 하드웨어(기능): GPIO, RCC, GLCD, EXTI, NAVI
// 제출일: 2026. 5. 31
// 제출자 클래스: 목요일반
// 학번: 2022130026
// 이름: 이주현
///////////////////////////////////////////////////////////////

//*************************************************//
// EXTI 역할(EXTI6, 7, 8, 9, 14, 15)
// NAVI_UP(PI6) 입력: EXTI6 발생, 부저 2회 후, 초기화면으로 리셋
// NAVI_DOWN(PI7) 입력: EXTI7 발생, 부저 2회 후, 1~45까지의 숫자가 반복적으로 증가함
// SW0(PH8) 입력: EXTI8 발생, LCD화면 'EXTI8' 출력, 부저 1회 후, Lotto머신의 숫자 첫번째 상자에 저장
// SW1(PH9) 입력: EXTI9 발생, LCD화면 'EXTI9' 출력, 부저 1회 후, Lotto머신의 숫자 두번째 상자에 저장
// SW6(PH14) 입력: EXTI14 발생, LCD화면 'EXTI14' 출력, 부저 1회 후, Lotto머신의 숫자 세번째 상자에 저장
// SW7(PH15) 입력: EXTI15 발생, LCD화면 'EXTI15' 출력, 부저 1회 후, Lotto머신의 숫자 네번째 상자에 저장

#include "stm32f4xx.h"		// stm32F4 시리즈 MCU의 핵심적인 Hardware(특히 Register) 정의 및 초기화 header file
#include "GLCD.h"		// GLCD 초기화 및 주요 라이브러리 함수 선언

void _GPIO_Init(void);		// GPIO port 초기화 루틴
uint16_t Key_Scan(void);	// Switch array(Key)중 어떤 switch가 눌렸는지 파악하여 출력하는 루틴
void BEEP(void);			// 부저 소리 발생하는 루틴
void DelayMS(unsigned short wMS);	// 시간 지연 함수(ms 단위)
void DelayUS(unsigned short wUS);		// 시간 지연 함수(us 단위)
void DisplayInitScreen(void);	// GLCD 초기 화면 실행 루틴

void _EXTI_Init(void);		// EXTI 초기화

uint8_t LottoCurNo=1, LOTTO_NO[4];	// Lotto머신의 숫자 저장을 위한 변수
int coin = 0;											//현재 진행 확인을 위한 변수
int main(void)
{
	_GPIO_Init();	// GPIO (LED, SW, Buzzer, Navi_SW) 초기화
	LCD_Init();		// GLCD 모듈 초기화
	DelayMS(10);	// GLCD 안정화를 위한 delay 시간(필수 아님)
	DisplayInitScreen();	// GLCD 초기화면
	_EXTI_Init();		// EXTI 초기화
    while(1)
	{
		if(coin >= 1 && coin <= 4)		//동작모드에서의 동작
		{
			LCD_SetTextColor(RGB_RED);								//글자색 빨간색으로 설정
			LCD_DisplayChar(1,9,(LottoCurNo/10) + '0');		//Lotto머신 10의자리 숫자 출력
			LCD_DisplayChar(1,10,(LottoCurNo%10) + '0'); 	//Lotto머신 1의자리 숫자 출력
			DelayMS(10);														//딜레이 10초
			LottoCurNo++;														//Lotto머신 값 1씩 증가
			if(LottoCurNo == 46)												//Lotto머신 값 46일 때 1로 초기화
				LottoCurNo = 1;
		}
		else if(coin == 5)														//4번째 추첨에서의 동작
		{
			for(int i = 0; i < 200; i++)											//동작모드이기에 딜레이 2초동안 Lotto머신 동작
			{
				LCD_SetTextColor(RGB_RED);							//글자색 빨간색으로 설정
				LCD_DisplayChar(1,9,(LottoCurNo/10) + '0');	//Lotto머신 10의자리 숫자 출력
				LCD_DisplayChar(1,10,(LottoCurNo%10) + '0'); //Lotto머신 1의자리 숫자 출력
				DelayMS(10);													//딜레이 10초
				LottoCurNo++;													//Lotto머신 값 1씩 증가
				if(LottoCurNo == 46)											//Lotto머신 값 46일 때 1로 초기화
					LottoCurNo = 1;
			}
			coin++;																	//현재 코인값 6, Lotto번호 증가 중단
			BEEP();																//0.5초 간격 부저 3회 
			DelayMS(500);
			BEEP();
			DelayMS(500);
			BEEP();
		}
	}	
}

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->AHB1ENR 	|= 0x0100;	// RCC_AHB1ENR GPIOI Enable
	RCC->APB2ENR 	|= (1<<14);	// Enable System Configuration Controller Clock
	
	GPIOI->MODER &= ~0xF000;	// GPIOI 6~7 : Input mode (reset state)	
	
	SYSCFG->EXTICR[2] |= 0x0077;	// EXTI8,9에 대한 소스 입력은 GPIOH로 설정
					// EXTI8 <- PH8, EXTI9 <- PH9 
					// EXTICR3(EXTICR[2])를 이용 
					// reset value: 0x0000	
	SYSCFG->EXTICR[3] |= 0x7700;	// EXTI14,15에 대한 소스 입력은 GPIOH로 설정
					// EXTI14 <- PH14, EXTI15 <- PH15 
					// EXTICR4(EXTICR[3])를 이용 
					// reset value: 0x0000	
	SYSCFG->EXTICR[1] |= 0x8800;	// EXTI6,7에 대한 소스 입력은 GPIOI로 설정
					// EXTI6 <- PI6, EXTI7 <- PI7 
					// EXTICR2(EXTICR[1])를 이용 
					// reset value: 0x0000	
	
	EXTI->FTSR |= (15<<6);		// EXTI6, 7, 8, 9: Falling Trigger Enable 
	EXTI->FTSR |= (3<<14);		//EXTI 14, 15: Falling Trigger Enable 
	EXTI->IMR  |= 0x00C0;		// EXTI6, 7 인터럽트 mask (Interrupt Enable) 설정
		
	NVIC->ISER[0] |= ( 1<<23  );		// Enable 'Global Interrupt EXTI6, 7, 8, 9'
	NVIC->ISER[1] |= ( 1<<8  ); 			// Enable 'Global Interrupt EXTI14, 15'
					// Vector table Position 참조
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void) 
{	
	if(EXTI->PR & 0x0040)			// EXTI6 Interrupt Pending(발생) 여부
	{											//NAVI_UP시의 동작
		EXTI->PR |= 0x0040;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		coin = 0;							//코인 초기화하여 Lotto머신 번호 증가 중단
		BEEP();							//0.5초 간격으로 부저 2회
		DelayMS(500);
		BEEP();
		LottoCurNo = 1;				//초기 Lotto머신 번호 1로 설정
		DisplayInitScreen();			//LCD화면 초기상태로 설정
		EXTI->IMR  &= ~0xC300;	//EXTI8, 9, 14, 15 인터럽트 mask (Interrupt Disable) 설정
	}
	else if(EXTI->PR & 0x0080)					// EXTI7 Interrupt Pending(발생) 여부
	{															//NAVI_DOWN시의 동작
		EXTI->PR |= 0x0080;						// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if(coin == 0)										//초기화 상태에서만 동작
		{
			coin++;											//코인 값 현재 1->Lotto 번호 증가 시작
			LCD_SetTextColor(RGB_BLUE);	//글자색 파란색으로 설정
			LCD_DisplayText(2,20,"W");			//LCD에 동작모드 W 표시 
			BEEP();										//부저 2회 동작
			DelayMS(500);
			BEEP();
		}
		EXTI->IMR  |= 0xC300;						//EXTI8, 9, 14, 15  인터럽트 mask (Interrupt Enable) 설정
	}	
	else if(EXTI->PR & 0x0100)												// EXTI8 Interrupt Pending(발생) 여부?
	{																						//SW0 입력시 동작
		EXTI->PR |= 0x0100;													// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if(coin == 1)																	//코인 1일 때 동작
		{
			LOTTO_NO[0] = LottoCurNo;									//Lotto머신 값 배열변수에 저장
			coin++;																		//코인 값 현재 2
			BEEP();																	//부저 1회 동작
			LCD_SetTextColor(RGB_RED);									//글자색 빨간색으로 설정
			LCD_DisplayChar(1,17, '8');										//LCD에 EXTI8 표시
			LCD_DisplayChar(2, 1, (LOTTO_NO[0]/10) + '0');	//Lotto머신 10의자리 숫자 출력
			LCD_DisplayChar(2, 2, (LOTTO_NO[0]%10) + '0');	//Lotto머신 1의자리 숫자 출력
		}
	}
	else if(EXTI->PR & 0x0200)												// EXTI9 Interrupt Pending(발생) 여부?
	{																						//SW1 입력시 동작
		EXTI->PR |= 0x0200;													// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if(coin == 2)																	//코인 2일 때 동작
		{																					
			LOTTO_NO[1] = LottoCurNo;									//Lotto머신 값 배열변수에 저장
			int num=0;																//중복 횟수 저장 변수
				while(LOTTO_NO[1] == LOTTO_NO[0])				//배열변수와의 중복여부 확인 코드
				{
					LOTTO_NO[1]++;												//중복되면 +1해서 저장
					num++;																//숫자 중복될 때마다 +1 증가
					if(LOTTO_NO[1] >= 46)									//배열변수+1값이 46 이상일 시 num값 저장
						LOTTO_NO[1] = num;
				}
			coin++;																		//코인값 현재 3
			BEEP();																	//부저 1회 동작
			LCD_SetTextColor(RGB_RED);									//글자색 빨간색으로 설정
			LCD_DisplayChar(1,17, '9');										//LCD에 EXTI9표시
			LCD_DisplayChar(2, 4, (LOTTO_NO[1]/10) + '0');	//Lotto머신 10의자리 숫자 출력
			LCD_DisplayChar(2, 5, (LOTTO_NO[1]%10) + '0');	//Lotto머신 1의자리 숫자 출력
		}
	}
}
void EXTI15_10_IRQHandler(void)
{
	if(EXTI->PR & 0x4000)														// EXTI14 Interrupt Pending(발생) 여부?
	{																						//SW6 입력시 동작
		EXTI->PR |= 0x4000;													// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if(coin == 3)																	//코인 3일 때 동작
		{																				
			LOTTO_NO[2] = LottoCurNo;									//Lotto머신 값 배열변수에 저장
			int num=0;																//중복 횟수 저장 변수
			while(LOTTO_NO[2] == LOTTO_NO[0] || LOTTO_NO[2] == LOTTO_NO[1])//배열변수와의 중복여부 확인 코드
			{
				LOTTO_NO[2]++;													//중복되면 +1해서 저장
				num++;																	//숫자 중복될 때마다 +1 증가
				if(LOTTO_NO[2] >= 46)										//배열변수+1값이 46 이상일 시 num값 저장
					LOTTO_NO[2] = num;										
			}
			coin++;																		//코인값 현재 4
			BEEP();																	//부저 1회 동작
			LCD_SetTextColor(RGB_RED);									//글자색 빨간색으로 설정
			LCD_DisplayChar(1,17, '1');										//LCD에 EXTI1 표시
			LCD_DisplayChar(1,18, '4');										//LCD에 EXTI 4 표시
			LCD_DisplayChar(2, 7, (LOTTO_NO[2]/10) + '0');	//Lotto머신 10의자리 숫자 출력
			LCD_DisplayChar(2, 8, (LOTTO_NO[2]%10) + '0');	//Lotto머신 10의자리 숫자 출력
		}
	}
	else if(EXTI->PR & 0x8000)												// EXTI15 Interrupt Pending(발생) 여부?
	{																						//SW7 입력시 동작
		EXTI->PR |= 0x8000;													// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		if(coin == 4)																	//코인 4일 때 동작
		{
			LOTTO_NO[3] = LottoCurNo;									//Lotto머신 값 배열변수에 저장
			int num=0;																//중복 횟수 저장 변수
			while(LOTTO_NO[3] == LOTTO_NO[0] || LOTTO_NO[3] == LOTTO_NO[1] || LOTTO_NO[3] == LOTTO_NO[2])//배열변수와의 중복여부 확인 코드
			{
				LOTTO_NO[3]++;													//중복되면 +1해서 저장
				num++;																	//숫자 중복될 때마다 +1 증가
				if(LOTTO_NO[3] >= 46)										//배열변수+1값이 46 이상일 시 num값 저장
					LOTTO_NO[3] = num;
			}
			coin++;																		//코인값 현재 4
			BEEP();																	//부저 1회 동작
			LCD_SetTextColor(RGB_RED);									//글자색 빨간색으로 설정
			LCD_DisplayChar(1,17, '1');										//LCD에 EXTI1 표시
			LCD_DisplayChar(1,18, '5');										//LCD에 EXTI 4 표시
			LCD_DisplayChar(2, 10, (LOTTO_NO[3]/10) + '0');	//Lotto머신 10의자리 숫자 출력
			LCD_DisplayChar(2, 11, (LOTTO_NO[3]%10) + '0');	//Lotto머신 1의자리 숫자 출력
		}
	}
}

/* GLCD 초기화면 표시 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);	// 화면 클리어
	LCD_SetFont(&Gulim7);		// 폰트 : 굴림 7
	LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : YELLOW
	LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
	LCD_DisplayText(0,0,"Lotto 2022130026 LJH");  	// Title
	LCD_SetBackColor(RGB_WHITE);		//글자배경색 : WHITE
	LCD_DisplayText(1,0,"MACHINE:    (EXTI  )");	//머신값 표시 배경 
	LCD_SetTextColor(RGB_RED);	//글자색 : RED
	LCD_DisplayText(2,0," 00 00 00 00 ");	//추첨 상자 00으로 초기화
	LCD_DisplayChar(1,9,(LottoCurNo/10) + '0');	//Lotto머신 10의자리 숫자 출력
	LCD_DisplayChar(1,10,(LottoCurNo%10) + '0');		//Lotto머신 10의자리 숫자 출력
	LCD_DisplayText(1,17,"?");	//LCD에 EXTI? 출력
	LCD_SetTextColor(RGB_BLUE);	//글자색 : BLUE
	LCD_DisplayText(2,20,"S");	//LCD에 초기 S모드 출력
	LCD_SetPenColor(RGB_BLACK);	//펜 색 : BLACK
	LCD_DrawRectangle(3, 21, 18, 12);	//LCD에 첫번째 상자 그림
	LCD_DrawRectangle(21, 21, 18, 12);	//LCD에 두번째 상자 그림
	LCD_DrawRectangle(39, 21, 18, 12);	//LCD에 세번째 상자 그림
	LCD_DrawRectangle(57, 21, 18, 12);	//LCD에 네번째 상자 그림
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
