/*		     MAIN PROJECT 
		Auto_ PC cafe System

		TEAM.7 : 	20933211 �̼���
				 20934113 �����
				 20933965 ��뼮		*/ 
  

								

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
unsigned char mot_tbl[] = {0x80,0xc0,0x40,0x60,0x20,0x30,0x10,0x90};				// 1~2�� ���� ���
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
 					 	  "\n\r       <  PC ȣȯ ���α׷�  >  "
						  "\n\r**************************************"
  					 	  "\n\r   ������ȣ�� �Է��Ͻñ� �ٶ��ϴ�  \n\r"
					 	  "**************************************\n\r : ";
// Hyper-terminal  Message
unsigned char P2_m[] ="\n\r**************************************"					
 					 	  "\n\r    ������ȣ�� ��ġ���� �ʽ��ϴ�  "
						  "\n\r**************************************"
  					 	  "\n\r   �ٽ� �Է��Ͻñ� �ٶ��ϴ� (4�ڸ�)  \n\r"
					 	  "**************************************\n\r : ";



// Interrupt and Overflow
SIGNAL(SIG_INTERRUPT0);			// start 
SIGNAL(SIG_INTERRUPT1);			// coin insert
SIGNAL(SIG_INTERRUPT2);			// issue
SIGNAL(SIG_INTERRUPT3);			// return
SIGNAL(SIG_OVERFLOW0);			// timer/counter, LED timer
SIGNAL(SIG_OVERFLOW1);  		// step-motor
SIGNAL(SIG_OVERFLOW2);			// timer/counter, FND timer 

// FND �ð��� ��ġ ���(��ġ , ��)
void FND_print(int t_clk, short data)		
{

	switch(t_clk)										
	{
		case SEC: 									// 1�� ����
			PORTF &= 0xF0;							
			PORTF |= 0x07;							
			break;
		
		case TEN_SEC: 									// 10�� ����
			PORTF &= 0xF0;						
			PORTF |= 0x0B;						
			break;

		case MIN:									// 1�� ����
			PORTF &= 0xF0;							
			PORTF |= 0x0D;							
			break;						

		case TEN_MIN: 									// 10�� ����
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

		case START:						// ����
		{
			sprintf(buf1,"%s",message[2]);
			sprintf(buf2,"COIN=%d          ",money);
			break;
		}

		case ISSUE:						// ���� ��ȣ �߱�
		{
			sprintf(buf1,"%s",message[3]);
			sprintf(buf2,"PIN-CODE:%4d       ",number);
			break;
		}

		case PROCESS:						// ���� ó��
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
				LCD_print(START);			//  ����

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

//���ڿ��� Hyper-Terminal�� ��� 
void PUT_char(unsigned char data)	

{
	while((UCSR0A & 0x20) == 0); 				
									
	UDR0 = data;				 			// �۽� �������Ϳ� data�� �Ҵ�

 	UCSR0A |= 0x20;						

}

// Hyper-terminal�κ��� �����͸� ����
unsigned char GET_char()		
{
	unsigned char data;
	while((UCSR0A & 0x80)==0);					

	data=UDR0;							// ���� �������� ���� �����Ϳ� ����
	UCSR0A |= 0x80;						

 	return data;							
}


// ���� ��ȣ �߱�
void GET_number()								
{		
	srand(TCNT0);							
	number = rand()%9000+1000; 					// ���� : 1000 ~ 9999
	sprintf(num_buf,"%d",number);					// int -> string
	
	LCD_print(ISSUE);						
}

// ��ȣ �Է�
void INPUT_number()						
{
	int j=0;							
	unsigned char Tmp = 0;				

 	while(1)
 	{
 
  		Tmp = GET_char();					
  		PUT_char(Tmp);					
 
  		if(Tmp == '\r'|| j > 3 ) 				// �����͸� �ʰ��ϰų����͸� �Է��� ���
		{
    		break;						
  		}
		
  		data_buf[j] = Tmp;					// �Էµ� �����͸� ���۷� ����
		_delay_ms(10);					
		j++;							// ���� �ּ� ����
		
 	}
		data_buf[j] = '\0';					// ���ڿ� �� '\0'
}

// Hyper-Terminal ���
void PC_print(int b)						
{
	int i = 0;

	switch(b)							
	{
		case P1:						// �Է� �䱸
			
			while(P1_m[i]!='\0')				
   				PUT_char(P1_m[i++]);			
				break;
			
		case P2:						// ���Է� �䱸 
						
			while(P2_m[i]!='\0')			
   				PUT_char(P2_m[i++]);	
				break;
	}

}

// PC �̿� �ð� ī����
void TIME_count()						
{
	time_m = COIN;							// �ݾ׿� ���� �̿� �ð� �Ҵ�	

	TIMSK = 0x44; 							// OVERFLOW1, 2 Enable
	
	while(1)
	{


		FND_print(MIN, time_m%10);				// 1�� ���� ǥ��
		_delay_ms(1);					

		FND_print(TEN_MIN, time_m/10);				// 10�� ���� ǥ��

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

//Hyper-Terminal ����
void PC_program()								
{
	EIMSK = 0x00;							// Interrupt Disable
	
	int c = P1;								

	while(1)
	{	
		PC_print(c);							

		INPUT_number();						// ���� ��ȣ �Է�

		PUT_char('\r');							
	   	PUT_char('\n');							

		if( strcmp(num_buf,data_buf) == 0 )			// ��ȣ ��ġ
		
			LCD_print(PROCESS);				// ���� ó�� ǥ�� 
			TIME_count();					// �̿� �ð�
			break;
		}
		
		else								
			c = P2;							
	}
 
} 

// ���α׷� ���� �Լ�
void START_program()							
{

	COIN = 0;								
	money = 0;

	while(1)	
	{

		EIMSK = 0x0E;							// INTERRPUT1, 2, 3 Enable
		time_m = 0,time_s = 0,timer2cnt = 0;	// �ð� ���� �ʱ�ȭ
		
		TIMSK = 0x01;							// OVERFLOW0 Enable

		LCD_print(START);						// ����

		// �ݾ� ��ȯ
		if( money != 0 && RETURN == 1)				
		{	
			RETURN_money();					
			led_cnt = 0;
		}
		
		 					
		if( timer0cnt == 150 )						
		{
			led_cnt++;						// 1.5�� ��� �� LED ����
			timer0cnt= 0;						
		}  
		      
		PORTC = LED_Data[led_cnt];					// ���ѽð� ǥ��
		    
		 // �ð� ����
		if(led_cnt == 8)						
		{ 	
			while(money != 0 )					// �ܿ� �ݾ��� ���� ���
			{
				RETURN_money();					// �ݾ� ��ȯ
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
	TCCR2 = 0x05;					// '�Ϲ�'���, Prescaler : 1024  ( * Ÿ�̸�/ī����2�� �ٸ� �� ) 
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
SIGNAL(SIG_INTERRUPT0)				// ����
{
	cli();
	_delay_ms(500);
	START_button = 1;		
	sei();
}

SIGNAL(SIG_INTERRUPT1)				// ���� ����
	cli();
	_delay_ms(500);				// ä�͸� ���� delay
	COIN++;					
	money += 500;				

	sei();
}

SIGNAL(SIG_INTERRUPT2)				// ���� ��ȣ �߱�
{
	cli();
	_delay_ms(500);
	if ( COIN != 0 )
	{
		GET = 1;		
	}

	sei();
}

SIGNAL(SIG_INTERRUPT3)				// ��ȯ
{
	cli();
	_delay_ms(500);
	RETURN = 1;		

	sei();
}


SIGNAL(SIG_OVERFLOW0)				// �ð����� �� LED Ÿ�̸� 
{
	cli();
	TCNT0 = 0xFF - 72 ;
	timer0cnt++;

	sei();
}

SIGNAL(SIG_OVERFLOW1)				// ���� ���� ����
{
	cli();
	TCNT1 = 0xFFFF - 30;

	PORTD = mot_tbl[mot_cnt];		// ���� ���� ����

	if(mot_cnt++ == 7 )			// 1-2�� ���� ���
		mot_cnt=0;
			
	sei();
}

SIGNAL(SIG_OVERFLOW2)				// �̿� �ð� ǥ��(FND)
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