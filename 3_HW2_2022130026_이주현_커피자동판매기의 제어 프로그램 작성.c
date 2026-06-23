
/////////////////////////////////////////////////////////////
// 과제명: HW2. 커피자동판매기의 제어 프로그램 작성
// 과제개요: 코인 주입 후 커피를 선택하면 각 커피의 재료에 맞는 LED가
// 점멸한 후, 커피 출력 종료되면 처음 상태로 전환됨
// 사용한 하드웨어(기능): GPIO 		
// 제출일: 2026. 5. 7
// 제출자 클래스: 목요일반
// 			  학번: 2022130026
// 			  이름: 이주현
///////////////////////////////////////////////////////////////

//*********************************************************************************//
// GPIO(INPUT mode: SWITCH(Key_Scan)) 실습
//	PortH의 PH8~15 핀들을 Input mode로 설정
// 	   * 교육용보드(키트)상의 회로에는 PH8~15에 SW 입력회로가 연결되어 있음
//		PH8: SW0, ....... , PH15: SW7
//	초기상태: Input mode  pin에는 강제로 초기상태 지정할 필요없음
//	실행: PH9(SW1) 및 PH10핀(SW2) 상태를 계속(무한루프) 읽어 '0' 인지 '1'인지를 파악후, LED 출력으로 입력상태를 작업자에게 표시
//
/* type 재지정 */
//	unsigned char == uint8_t
//	unsigned int, short == uint16_t
//	unsigned long == uint32_t

#include "stm32f4xx.h"		// stm32F4 시리즈 MCU의 핵심적인 Hardware(특히 Register) 정의 및 초기화 header file

void _GPIO_Init(void);		// GPIO port 초기화 루틴
uint16_t KEY_Scan(void);	// Switch array(Key)중 어떤 switch가 눌렸는지 파악하여 출력하는 루틴
void BEEP(void);			// 부저 소리 발생하는 루틴
void DelayMS(unsigned short wMS);	// 시간 지연 함수(ms 단위)
void DelayUS(unsigned short wUS);		// 시간 지연 함수(us 단위)
void Cup_LED(void);					//Cup LED 점멸 함수
void Sugar_LED(void);				//Sugar LED 점멸 함수
void Cream_LED(void);				//Cream LED 점멸 함수
void Water_Coffee_LED(void);	//Water, Coffee LED 점멸 함수

int main(void)
{
	_GPIO_Init();	// GPIO (LED(PortG:output), SW(PortH: input), Buzzer(PortI: Output) 초기화
	GPIOG->ODR &= ~0x00FF;	// 초기값: LED0~7 Off  (필수 아님)
	int coin = 0;							//코인 변수 생성
	while(1)
	{
		if((GPIOH->IDR & 0x0100) == 0 && coin == 0) //코인값 0일 때에만 코인 입력 받음
		{
			coin++;										//코인값 +1->코인값 현재 1
			GPIOG->BSRRL = 0x0001; 		//코인 LED ON
			BEEP();									// 음성 알림
		}	
		if(coin == 1)									//코인값 1일 때에만  커피 종류 입력 가능
		{
			switch(KEY_Scan())					//KEY_Scan값을 switch로 받음
			{
				case 0x7F00 : 						//믹스커피, SW7 입력			
					
					GPIOG->ODR |= 0x0080; 	//LED7 (PG7) ON
					BEEP();							// 음성 알림
					DelayMS(1000);				//1초 딜레이
								
					Cup_LED();						//Cup LED 1회 점멸
					Sugar_LED();					//Sugar LED 2회 점멸
					Cream_LED();					//Cream LED 2회 점멸
					Water_Coffee_LED();		//Water/Coffee LED 3회 점멸, 부저 3회
					coin++;								//코인값 +1->코인값 현재 2
					break;
							
				case 0xBF00 : 						//슈가커피, SW6 입력			
					
					GPIOG->ODR |= 0x0040;  //LED6 (PG6) ON
					BEEP();							// 음성 알림
					DelayMS(1000);				//1초 딜레이
								
					Cup_LED();						//Cup LED 1회 점멸
					Sugar_LED();					//Sugar LED 2회 점멸
					Water_Coffee_LED();		//Water/Coffee LED 3회 점멸, 부저 3회
					coin++;								//코인값 +1->코인값 현재 2
					break;
								
				case 0xDF00 : 						//블랙커피, SW5입력			
					GPIOG->ODR |= 0x0020; 	//LED5 (PG5) ON
					BEEP();							// 음성 알림
					DelayMS(1000);				//1초 딜레이
								
					Cup_LED();						//Cup LED 1회 점멸
					Water_Coffee_LED();		//Water/Coffee LED 3회 점멸, 부저 3회
					coin++;								//코인값 +1->코인값 현재 2
					break;
			}
		}
		if(coin == 2)									//코인값 2일 때 종료 사이클 진행
		{
			DelayMS(500);							//커피 싸이클 끝난 후 0.5초 딜레이 있으므로, 1초 딜레이 위해서 0.5초 추가 딜레이
			coin=0;										//코인값 0으로 Reset
			GPIOG->BSRRH=0x0001;			//코인 LED OFF
			GPIOG->ODR &= 0x0000;			//전체 LED OFF
		}
	}
}

void Cup_LED(void)								//Cup LED(LED 4) 1회 점멸
{
	GPIOG->ODR |= 0x0010;					//Cup LED, LED 4 ON
	DelayMS(500);									//딜레이 0.5초
	GPIOG->ODR &= ~0x0010; 				//Cup LED, LED 4 OFF
	DelayMS(500);									// 음성 알림
}

void Sugar_LED(void)							//Sugar LED(LED 3) 2회 점멸
{
	for(int i=0; i<2; i++)							//2회 반복
		{
			GPIOG->ODR |= 0x0008;			//Sugar LED ON
			DelayMS(500);							//딜레이 0.5초
			GPIOG->ODR &= ~0x0008;		//Sugar LED OFF
			DelayMS(500);							//딜레이 0.5초
		}
}

void Cream_LED(void)							//Cream LED(LED 2) 2회 점멸
{
	for(int i=0; i<2; i++)							//2회 반복
		{
			GPIOG->ODR |= 0x0004; 			//Cream LED ON
			DelayMS(500);							//딜레이 0.5초
			GPIOG->ODR &= ~0x0004;		//Cream LED OFF
			DelayMS(500);							//딜레이 0.5초
		}
}

void Water_Coffee_LED(void)				//Water/Coffee LED(LED 1) 3회 점멸, 부저 3회
{
	for(int i=0; i<3; i++)							//3회 반복
		{
			GPIOG->ODR |= 0x0002; 			//Water/Coffee LED, LED 1 ON
			DelayMS(500);							//딜레이 0.5초
			GPIOG->ODR &= ~0x0002;		//Water/Coffee LED, LED 1 OFF
			DelayMS(500);							//딜레이 0.5초
		}
	for(int i=0; i<3; i++)							//3회 반복
		{
			BEEP();									// 음성 알림
			DelayMS(500);							//딜레이 0.5초
		}
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

/* GPIO ( GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer) ) 초기 설정	*/
void _GPIO_Init(void)
{
// GPIO G(0~7)(LED) mode 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC->AHB1ENR.6 : GPIOG Clock Enable (bit#6 : '1')			

	GPIOG->MODER &= 0xFFFF0000;	// GPIOG 0~7 pin의 mode 정의비트(각 핀당 2비트): Clear (0b00)
							// 0b 1111 1111 1111 1111 0000 0000 0000 0000 (0~15번 비트만 '0', 16~31번 비트는 전 상태 유지)
							// mode 설정 2비트가 0b00 인 것은 그 핀이 GPIO input mode(default, 즉 reset후 기본상태임)임을 의미
							// * MODER 레지스터의 0~15번 비트를 모두 '0'으로 clear하는 이유:  
							//	MCU구동 과정중 해당 비트에 원치 않은(쓰레기) 값이 있을 수 있기 떄문에 
							//	해당 비트에 새로운 값을 write하기 전에 쓰레기 값을 모두 지우는 것임.  
	GPIOG->MODER |=  0x00005555;	// GPIOG 0~7 pin의 모드를 GPIO output mode로 정의: Output mode (0b01)						
							// 0b 0000 0000 0000 0000 0101 0101 0101 0101
							// mode 설정 2비트가 0b01 인 것은 그 핀이 GPIO output mode임을 의미

	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7핀의 출력 타입을 Push-pull로 설정 (GP8~15의 출력타입: reset state로 유지)		
							// GPIOG 0~7 핀을  Push-pull 상태로 하기 위해, OTYPER 레지스터의 0~7번 비트를 '0'으로 함
							// ~0x00FF : ~0b0000 0000 1111 1111 = 0b1111 1111 0000 0000 (0~7번 비트만 '0', 8~15번 비트는 전 상태 유지)

	GPIOG->OSPEEDR &= ~0x0000FFFF;	// GPIOG 0~7핀의 출력신호 스피드 설정 : OSPEEDR 해당 bit Clear (0b00)
								// GPIOG 0~7 핀의 출력신호 스피드 설정을 위해 OSPEEDR 레지스터의 0~15번 비트를 '0'으로 함
								// ~0x0000FFFF : ~0b0000 0000 0000 0000 1111 1111 1111 1111 = 0b1111 1111 1111 1111 0000 0000 0000 0000 
								// 			(0~15번 비트만 '0', 16~31번 비트는 전 상태 유지)
	GPIOG->OSPEEDR |=  0x00005555;	// GPIOG 0~7핀의 출력신호 스피드 설정 : Output speed 25MHZ Medium speed 
								// 0b 0000 0000 0000 0000 0101 0101 0101 0101
								// Speed 설정 2비트가 0b01 인 것은 그 핀이 GPIO output speed 25MHZ 로 함을 의미
	// PUPDR : Default (floating) 
    
// GPIO H(8~15)(SW) mode 설정 : Input mode
	RCC->AHB1ENR	|=  0x00000080;	// RCC->AHB1ENR.7 : GPIOH Clock Enable (bit#7 : '1')							
	GPIOH->MODER &= ~0x0000FFFF; // ~0xFFFF0000= 0x0000FFFF
							// GPIOH 8~15 pin의 mode 정의비트(각 핀당 2비트): Clear (0b00) 즉,  Input mode (reset state)	
							// 0b 0000 0000 0000 0000 1111 1111 1111 1111 (16~31번 비트만 '0', 0~15번 비트는 전 상태 유지)
							// mode 설정 2비트가 0b00 인 것은 그 핀이 GPIO input mode(default, 즉 reset후 기본상태임)임을 의미
			
	GPIOH->PUPDR 	&= ~0xFFFF0000; // ~0xFFFF0000= 0x0000FFFF
							// GPIOH 8~15 pin의 핀 내부의 pull up/down 저항 사용 유무 지정(각 핀당 2비트) 
							//  해당 핀의 비트들이 0b00 인 경우: 핀내부가 Floating input (즉, No Pull-up, pull-down, reset state)임을 의미

// GPIO F.9(Buzzer) mode 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000020; 	// RCC->AHB1ENR.5 : GPIOF Clock(bit#5) Enable
	GPIOF->MODER 	&= ~0x000C0000; // GPIOF 9 pin의 mode 정의비트(각 핀당 2비트): Clear (0b00)
							// ~0x000C0000 = ~0b 0000 0000 0000 1100 0000 0000 0000 0000 = 0b 1111 1111 1111 0011 1111 11111 1111 1111 
							//	(18,19번 비트(GPIOF 9 pin의 mode 정의)만 '0', 그외 비트들은 모두 비트는 전 상태 유지)
							// mode 설정 2비트가 0b00 인 것은 그 핀이 GPIO input mode(default, 즉 reset후 기본상태임)임을 의미
	GPIOF->MODER 	|= 0x00040000;	// GPIOF 9 pin의 모드를 GPIO output mode로 정의: Output mode (0b01)						
							// 0b 0000 0000 0000 0100 0000 0000 0000 0000
							// mode 설정 2비트(18,19번 비트(GPIOF 9 pin의 mode 정의)가 0b01 인 것은 그 핀이 GPIO output mode임을 의미
				
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9핀의 출력 타입을 Push-pull로 설정 (그외핀의 출력타입: reset state로 유지)		
							// GPIOF 9핀을 Push-pull 상태로 하기 위해, OTYPER 레지스터의 9번 비트를 '0'으로 함
							// ~0x0200 : ~0b0000 0010 0000 0000 = 0b1111 1101 1111 11111 (9번 비트만 '0', 그외 비트들은 전 상태 유지)
	GPIOF->OSPEEDR &= ~0x000C0000;	// GPIOF 9핀의 출력신호 스피드 설정 : OSPEEDR 해당 bit Clear (0b00)
								// GPIOF 9핀의 출력신호 스피드 설정을 위해 OSPEEDR 레지스터의 18,19번 비트를 '0'으로 함
								// ~0x000C0000 = ~0b 0000 0000 0000 1100 0000 0000 0000 0000 = 0b 1111 1111 1111 0011 1111 11111 1111 1111 
								//	(18,19번 비트(GPIOF 9 pin의 mode 정의)만 '0', 그외 비트들은 모두 비트는 전 상태 유지)
 	GPIOF->OSPEEDR |= 0x00040000;	// GPIOF 9핀의 출력신호 스피드 설정 : Output speed 25MHZ Medium speed 
							// 0b 0000 0000 0000 0100 0000 0000 0000 0000
							// Speed 설정 2비트가 0b01 인 것은 그 핀이 GPIO output speed 25MHZ 로 함을 의미
}	

/* 부저 소리 발생하는 루틴 */
/* Buzzer: Beep for 30 ms 
	소리크기 변경: DelayMS 시간 조정  */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' : Buzzer on
	DelayMS(5);			// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' : Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000); 	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}

/**************************************************************************
// 보충 설명자료
// 다음은 stm32f4xx.h에 있는 RCC관련 주요 선언문임 
#define PERIPH_BASE           ((uint32_t)0x40000000) // Peripheral base address in the alias region                         

// Peripheral memory map  
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000)

// AHB1 peripherals  
#define GPIOA_BASE	(AHB1PERIPH_BASE + 0x0000)
#define GPIOB_BASE	(AHB1PERIPH_BASE + 0x0400)
#define GPIOC_BASE	(AHB1PERIPH_BASE + 0x0800)
#define GPIOD_BASE	(AHB1PERIPH_BASE + 0x0C00)
#define GPIOE_BASE	(AHB1PERIPH_BASE + 0x1000)
#define GPIOF_BASE	(AHB1PERIPH_BASE + 0x1400)
#define GPIOG_BASE	(AHB1PERIPH_BASE + 0x1800)
#define GPIOH_BASE	(AHB1PERIPH_BASE + 0x1C00)
#define GPIOI_BASE	(AHB1PERIPH_BASE + 0x2000)
#define GPIOJ_BASE	(AHB1PERIPH_BASE + 0x2400)
#define GPIOK_BASE	(AHB1PERIPH_BASE + 0x2800)
#define CRC_BASE	(AHB1PERIPH_BASE + 0x3000)
#define RCC_BASE	(AHB1PERIPH_BASE + 0x3800)
#define FLASH_R_BASE	(AHB1PERIPH_BASE + 0x3C00)
#define SYSCFG_BASE	(APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE	(APB2PERIPH_BASE + 0x3C00)

typedef struct
{
  __IO uint32_t MODER;    // GPIO port mode register,               Address offset: 0x00       
  __IO uint32_t OTYPER;   // GPIO port output type register,        Address offset: 0x04       
  __IO uint32_t OSPEEDR;  // GPIO port output speed register,       Address offset: 0x08       
  __IO uint32_t PUPDR;    // GPIO port pull-up/pull-down register,  Address offset: 0x0C       
  __IO uint32_t IDR;      // GPIO port input data register,         Address offset: 0x10       
  __IO uint32_t ODR;      // GPIO port output data register,        Address offset: 0x14       
  __IO uint16_t BSRRL;    // GPIO port bit set/reset low register,  Address offset: 0x18       
  __IO uint16_t BSRRH;    // GPIO port bit set/reset high register, Address offset: 0x1A       
  __IO uint32_t LCKR;     // GPIO port configuration lock register, Address offset: 0x1C       
  __IO uint32_t AFR[2];   // GPIO alternate function registers,     Address offset: 0x20-0x24  
} GPIO_TypeDef;

#define GPIOA	((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB	((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC	((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD	((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE	((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF	((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG	((GPIO_TypeDef *) GPIOG_BASE)
#define GPIOH	((GPIO_TypeDef *) GPIOH_BASE)
#define GPIOI	((GPIO_TypeDef *) GPIOI_BASE)
#define GPIOJ	((GPIO_TypeDef *) GPIOJ_BASE)
#define GPIOK	((GPIO_TypeDef *) GPIOK_BASE)

*******************************************************/ 