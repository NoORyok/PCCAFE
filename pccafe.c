/*		     MAIN PROJECT 
		Auto_ PC cafe System

		TEAM.7 : 	20933211 이성재
				 20934113 임재우
				 20933965 장용석		*/ 
  

								

// HEADER FILE
#include<avr/io.h>			// AVR I/O
#include<stdlib.h>			
#include<avr/interrupt.h>		// interrupt
#include<util/delay.h>		
#include"lcd.h"				// lcd library
#include<string.h>			

// constant
#define P1 0				
#define P2 1				 
#define RUN 2				
#define START 3			
#define ISSUE 4			
#define PROCESS 5			
#define BUF_SIZE 17

enum{TEN_MIN,MIN,TEN_SEC,SEC};	


// global variables
unsigned char mot_cnt=0;		// step-motor count
unsigned char led_cnt=0;		// LED count
unsigned char COIN = 0;			// coin count
unsigned int number;			
unsigned int money=0;				


volatile unsigned char GET = 0;		
volatile unsigned char RETURN = 0;		
volatile unsigned char START_button = 0;	
volatile unsigned char timer0cnt= 0;		// LED timer
volatile unsigned char timer2cnt= 0;		// FND timer

short time_s=0, time_m=0;			// sec, min

// Array
char buf1[BUF_SIZE];				// LCD 1 ROW
char buf2[BUF_SIZE];				// LCD 2 ROW
char num_buf[5];				
char data_buf[5]; 				


unsigned char FND_DATA_TBL[]={0x3F,0X06,0X5B,0X4F,0X66,0X6D,0X7D,0X27,0X7F,0x6F}; 		// FND digit 
unsigned char mot_tbl[] = {0x80,0xc0,0x40,0x60,0x20,0x30,0x10,0x90};				// 1~2상 여자 방식
unsigned char LED_Data[] = {0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00};			


// LCD message
unsigned char message[6][BUF_SIZE] = {	"< Main Project >",						
									" Auto PC Cafe!! ",		
									"Insert a coin!! ",
									"Check out code!!",
									"   Thank you !! ",
									"Have a good time" 								
									};
// Hyper-terminal  Message
unsigned char P1_m[]="\n\r**************************************"					
 					 	  "\n\r       <  PC 호환 프로그램  >  "
						  "\n\r**************************************"
  					 	  "\n\r   인증번호를 입력하시기 바랍니다  \n\r"
					 	  "**************************************\n\r : ";
// Hyper-terminal  Message
unsigned char P2_m[] ="\n\r**************************************"					
 					 	  "\n\r    인증번호가 일치하지 않습니다  "
						  "\n\r**************************************"
  					 	  "\n\r   다시 입력하시기 바랍니다 (4자리)  \n\r"
					 	  "**************************************\n\r : ";



// Interrupt and Overflow
SIGNAL(SIG_INTERRUPT0);			// start 
SIGNAL(SIG_INTERRUPT1);			// coin insert
SIGNAL(SIG_INTERRUPT2);			// issue
SIGNAL(SIG_INTERRUPT3);			// return
SIGNAL(SIG_OVERFLOW0);			// timer/counter, LED timer
SIGNAL(SIG_OVERFLOW1);  		// step-motor
SIGNAL(SIG_OVERFLOW2);			// timer/counter, FND timer 

// FND 시간의 위치 출력(위치 , 값)
void FND_print(int t_clk, short data)		
{

	switch(t_clk)										
	{
		case SEC: 									// 1초 단위
			PORTF &= 0xF0;							
			PORTF |= 0x07;							
			break;
		
		case TEN_SEC: 									// 10초 단위
			PORTF &= 0xF0;						
			PORTF |= 0x0B;						
			break;

		case MIN:									// 1분 단위
			PORTF &= 0xF0;							
			PORTF |= 0x0D;							
			break;						

		case TEN_MIN: 									// 10분 단위
			PORTF &= 0xF0;						
			PORTF |= 0x0E;							
			break;

		default:										
			PORTF &= 0xF0;						
			PORTF |= 0x0F;							
			break;

	}
	
	if(t_clk== MIN && time_s%2 ==0)						
		PORTB =	FND_DATA_TBL[data] | 0x80;				
	else	
		PORTB =	FND_DATA_TBL[data];					
}

// LCD-display
void LCD_print(int a)			
{

	switch(a)					
	{
		case RUN:					
		{
			sprintf(buf1,"%s",message[0]);	
			sprintf(buf2,"%s",message[1]);
			break;
		}		

		case START:						// 시작
		{
			sprintf(buf1,"%s",message[2]);
			sprintf(buf2,"COIN=%d          ",money);
			break;
		}

		case ISSUE:						// 인증 번호 발급
		{
			sprintf(buf1,"%s",message[3]);
			sprintf(buf2,"PIN-CODE:%4d       ",number);
			break;
		}

		case PROCESS:						// 정상 처리
		{
			sprintf(buf1,"%s",message[4]);
			sprintf(buf2,"%s",message[5]);
			break;
		}	

	}
	
	lcdGotoXY(0,0);							// LCD 1 ROW
	lcdPrintData(buf1,16);					

	lcdGotoXY(0,1);							// LCD 2 ROW
	lcdPrintData(buf2,16);					

}

// coin-return function
void RETURN_money()					
{

	EIMSK = 0x00;							// Interrupt Disable 
	timer0cnt = 0;			

	while(1)
	{
		PORTC = 0x00;						

		if( timer0cnt == 100 )					// 1sec delay
		{
			money -= 500;					// money-down
			timer0cnt = 0;					// count = 0
		}
	
		LCD_print(START);					
	
		if( money == 0 )			
		{
		
			while(1)				
			{
				LCD_print(START);			//  시작

				if(	timer0cnt == 100 )		
				{
					timer0cnt = 0;			
					break;
				}
			}

				COIN = 0;				
				RETURN = 0;				
				EIMSK = 0x0E;				// INTERRUPT1,2,3 Enable
				break;
		}	
	}	
	
}

//문자열을 Hyper-Terminal로 출력 
void PUT_char(unsigned char data)	

{
	while((UCSR0A & 0x20) == 0); 				
									
	UDR0 = data;				 			// 송신 레지스터에 data를 할당

 	UCSR0A |= 0x20;						

}

// Hyper-terminal로부터 데이터를 수신
unsigned char GET_char()		
{
	unsigned char data;
	while((UCSR0A & 0x80)==0);					

	data=UDR0;							// 수신 레지스터 값을 데이터에 저장
	UCSR0A |= 0x80;						

 	return data;							
}


// 인증 번호 발급
void GET_number()								
{		
	srand(TCNT0);							
	number = rand()%9000+1000; 					// 범위 : 1000 ~ 9999
	sprintf(num_buf,"%d",number);					// int -> string
	
	LCD_print(ISSUE);						
}

// 번호 입력
void INPUT_number()						
{
	int j=0;							
	unsigned char Tmp = 0;				

 	while(1)
 	{
 
  		Tmp = GET_char();					
  		PUT_char(Tmp);					
 
  		if(Tmp == '\r'|| j > 3 ) 				// 데이터를 초과하거나엔터를 입력할 경우
		{
    		break;						
  		}
		
  		data_buf[j] = Tmp;					// 입력된 데이터를 버퍼로 저장
		_delay_ms(10);					
		j++;							// 버퍼 주소 증가
		
 	}
		data_buf[j] = '\0';					// 문자열 끝 '\0'
}

// Hyper-Terminal 출력
void PC_print(int b)						
{
	int i = 0;

	switch(b)							
	{
		case P1:						// 입력 요구
			
			while(P1_m[i]!='\0')				
   				PUT_char(P1_m[i++]);			
				break;
			
		case P2:						// 재입력 요구 
						
			while(P2_m[i]!='\0')			
   				PUT_char(P2_m[i++]);	
				break;
	}

}

// PC 이용 시간 카운터
void TIME_count()						
{
	time_m = COIN;							// 금액에 따른 이용 시간 할당	

	TIMSK = 0x44; 							// OVERFLOW1, 2 Enable
	
	while(1)
	{


		FND_print(MIN, time_m%10);				// 1분 단위 표시
		_delay_ms(1);					

		FND_print(TEN_MIN, time_m/10);				// 10분 단위 표시

		FND_print(SEC, time_s%10);			
		_delay_ms(1);					

		FND_print(TEN_SEC, time_s/10);		
		_delay_ms(1);	
		
		if( time_m < 0 )				 
		{ 
				TIMSK = 0x00;				// OVERFLOW1 Disable
				FND_print(4,0);				// Timer Disable
				break;				
		}
			
	}

}

//Hyper-Terminal 실행
void PC_program()								
{
	EIMSK = 0x00;							// Interrupt Disable
	
	int c = P1;								

	while(1)
	{	
		PC_print(c);							

		INPUT_number();						// 인증 번호 입력

		PUT_char('\r');							
	   	PUT_char('\n');							

		if( strcmp(num_buf,data_buf) == 0 )			// 번호 일치
		
			LCD_print(PROCESS);				// 정상 처리 표시 
			TIME_count();					// 이용 시간
			break;
		}
		
		else								
			c = P2;							
	}
 
} 

// 프로그램 시작 함수
void START_program()							
{

	COIN = 0;								
	money = 0;

	while(1)	
	{

		EIMSK = 0x0E;							// INTERRPUT1, 2, 3 Enable
		time_m = 0,time_s = 0,timer2cnt = 0;	// 시간 변수 초기화
		
		TIMSK = 0x01;							// OVERFLOW0 Enable

		LCD_print(START);						// 시작

		// 금액 반환
		if( money != 0 && RETURN == 1)				
		{	
			RETURN_money();					
			led_cnt = 0;
		}
		
		 					
		if( timer0cnt == 150 )						
		{
			led_cnt++;						// 1.5초 경과 시 LED 감소
			timer0cnt= 0;						
		}  
		      
		PORTC = LED_Data[led_cnt];					// 제한시간 표시
		    
		 // 시간 만료
		if(led_cnt == 8)						
		{ 	
			while(money != 0 )					// 잔여 금액이 있을 경우
			{
				RETURN_money();					// 금액 반환
			}

			lcdClear();						
			break;							

		}
		
		
		if( COIN != 0  && GET == 1 )
		{
				TIMSK = 0x00;
				PORTC = 0x00;					
				GET_number();						
				PC_program();						
				RETURN = 0,GET=0,COIN=0,money=0,led_cnt=0; 	
		}
	}
}


int main()	
{
	// PORT Settings
	DDRB = 0xFF;				// FND
	DDRC = 0xFF;				// LED
	DDRD = 0xF0;				// STEP MOTOR
	DDRE = 0xFE;				// UART
	DDRF = 0x0F;				// FND Control

	// Extenal Interrput settings
	EIMSK = 0x01;				
	EIFR = 0x0F;				
	EICRA= 0xFF;				// Positive-Edge Trigger
	
	// UART Settings
	UCSR0A = 0x00;
 	UCSR0B = 0x18;			 	// Rx, Tx enable
 	UCSR0C = 0x06;			 	// asynchro, No Parity bit, 1 Stop bit
	UBRR0H = 0x00;
	UBRR0L = 0x03; 				//7.3728 MHz, 115200 bps	 
	


	// 8 BIT timer/count0
	TCCR0 = 0x07;					// 'Prescaler : 1024
	TCNT0 = 0xFF - 72;				// (1 / (7372800/1024)) * 72 = 0.01s

	// 8 BIT timer/count2
	TCCR2 = 0x05;					// '일반'모드, Prescaler : 1024  ( * 타이머/카운터2는 다른 값 ) 
	TCNT2 = 0xFF - 72; 				// (1 / (7372800/1024)) * 72 = 0.01s

	// 16 BIT timer/count1
	TCCR1A = 0x00;					
	TCCR1B = 0x05;					// Prescaler : 1024 
	TCCR1C = 0x00;
	TCNT1 = 0xFFFF - 30;			

	TIMSK = 0x00;					// OVERFLOW Disable 
	TIFR  = 0x45;					 
	

	// LCD initialization
	lcdInit();								
	
	sei();						// INTERRUPT Enable  

	LCD_print(RUN);		

	
	while(1)
	{
		if( START_button == 1)		
		{
			START_button = 0;		
			START_program();		
		}

	}
		
	return 0;
	
}

// Interrput Service Routines
SIGNAL(SIG_INTERRUPT0)				// 시작
{
	cli();
	_delay_ms(500);
	START_button = 1;		
	sei();
}

SIGNAL(SIG_INTERRUPT1)				// 동전 투입
	cli();
	_delay_ms(500);				// 채터링 방지 delay
	COIN++;					
	money += 500;				

	sei();
}

SIGNAL(SIG_INTERRUPT2)				// 인증 번호 발급
{
	cli();
	_delay_ms(500);
	if ( COIN != 0 )
	{
		GET = 1;		
	}

	sei();
}

SIGNAL(SIG_INTERRUPT3)				// 반환
{
	cli();
	_delay_ms(500);
	RETURN = 1;		

	sei();
}


SIGNAL(SIG_OVERFLOW0)				// 시간지연 및 LED 타이머 
{
	cli();
	TCNT0 = 0xFF - 72 ;
	timer0cnt++;

	sei();
}

SIGNAL(SIG_OVERFLOW1)				// 스텝 모터 가동
{
	cli();
	TCNT1 = 0xFFFF - 30;

	PORTD = mot_tbl[mot_cnt];		// 스텝 모터 실행

	if(mot_cnt++ == 7 )			// 1-2상 여자 방식
		mot_cnt=0;
			
	sei();
}

SIGNAL(SIG_OVERFLOW2)				// 이용 시간 표시(FND)
{
	cli();
	TCNT2 = 0xFF - 72;			// 0.01sec
	timer2cnt++;					
	
	if(timer2cnt ==100)				
    {
			
		if(time_s == 0)				
        {
        	time_m--;				
			time_s=59;		
		}
		else
			time_s--;		
	 	
		timer2cnt = 0;			
	}
		
	sei();

}