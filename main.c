
#include <avr/io.h>	
#include <avr/interrupt.h>	
#include <util/delay.h>	
#include "Lcd.h"	

#define  DIR_L 0
#define  DIR_R 1

unsigned char timer0Cnt=0, mot_cnt=0;
volatile unsigned char dir=DIR_R;	//처음방향은 우측부터
volatile unsigned char Step_flag = 0, buzzer_flag = 0, LCD_flag = 0;
volatile unsigned int piano = 0;
volatile unsigned char getData = 0;

//1-2상 여자 값을 사용
unsigned char Step[]={0x90, 0x80, 0xc0, 0x40, 0x60, 0x20, 0x30, 0x10};

//피아노 음계에 해당하는 PWM주파수
unsigned int DoReMi[8]={523, 587, 659, 698, 783, 880, 987, 1046};
	
void putch(unsigned char data){
	while((UCSR0A & 0x20)==0); //전송되기 전까지 대기
	UDR0=data;
	UCSR0A |= 0x20;
}

unsigned char getch(void)
{
	unsigned char data;
	if((UCSR0A & 0x80)!=0) 
	{
		data=UDR0;
		UCSR0A |= 0x80;
	}
	return data;
}

int main(void)
{
	unsigned char piano=0;
	unsigned char str1[] = " STEP Motor:";
	unsigned char str2[] = " Buzzer:";
	unsigned char str3[] = " OFF ";
	unsigned char str4[] = " ON ";
	unsigned char str5[] = " CW ";
	unsigned char str6[] = " CCW ";
	
	unsigned char RX_data = 0;
	DDRE = 0x0e;
	
	UCSR0A=0x00;
	UCSR0B=0x18;
	UCSR0C=0x06;
	UBRR0H=0x00;
	UBRR0L=0x03;
	
	DDRA=0xff;
	DDRB = 0x80;  //PB7에 PIEZO 연결
	DDRD=0xf0; //D포트에는 모터 연결

	
	//PORTB &= ~0x20; //M1 Disable, DC 모터 정지
	
	LcdInit_4bit();	//LCD를 초기화
	
	TCCR0 = 0x07;
	TCNT0=184;	//256-72=184
	
	TCCR1A |= 0x0a;
	
	TCCR1B |= 0x19;
	
	TCCR1C=0x00;	
	TCNT1=0x0000;	//타이머1 카운터 초기화
	
	TIMSK = 0x01;	//TOIE0 '1'
	TIFR = 0x01;	//TOV0 '1'
	
	EICRB = 0xff;
	
	EIMSK=0xf0;
	EIFR=0xf0;
	sei(); 
	
	 Lcd_Pos(0,0);
	 Lcd_STR(str1);
	 Lcd_Pos(0,12);
	 Lcd_STR(str3);
	 
	 Lcd_Pos(1,0);
	 Lcd_STR(str2);
	 Lcd_Pos(1,10);
	 Lcd_STR(str3);
   
    while (1) 
    {
		getData=getch();
		
		putch(getData);
		
		if(getData=='1'|| getData=='2' ||getData=='3' ||getData=='4')
		{	
			RX_data = getch();
		}
		
		if(RX_data=='1')  // 정지
		{
			Step_flag=0;
			PORTD=0;
			buzzer_flag=0;
			OCR1C=0;
			LCD_flag |= 0x01;
		}
		else if(RX_data=='2') // piezo 출력
		{
			LCD_flag |= 0x02;
			buzzer_flag=1;
		}
		else if(RX_data=='3') // 오른쪽으로 회전
		{
			
			Step_flag=1;
			dir = DIR_R;
			LCD_flag |= 0x04;
		}
		else if(RX_data=='4') // 왼쪽으로 회전
		{
			
			Step_flag=1;
			dir = DIR_L;
			LCD_flag |= 0x08;
		}
			 
		if(LCD_flag)  // lcd 출력
		{
			if(LCD_flag & 0x01)
			{
				Lcd_Pos(0,12);
				Lcd_STR(str3);
				Lcd_Pos(1,10);
				Lcd_STR(str3);
				LCD_flag &= 0x0e;
			}
			if(LCD_flag & 0x02)
			{
				Lcd_Pos(1,10);
				Lcd_STR(str4);
				buzzer_flag=1;
				LCD_flag &= 0x0d;
			}
			if(LCD_flag & 0x04)
			{
				Lcd_Pos(0,12);
				Lcd_STR(str5);
				LCD_flag &= 0x0b;
			}
			if(LCD_flag & 0x08)
			{
				Lcd_Pos(0,12);
				Lcd_STR(str6);
				LCD_flag &= 0x07;
			}
		}
		if(buzzer_flag)
		{
			ICR1 = 7372800/DoReMi[piano];
			
			OCR1C = ICR1/20;
			piano++;
			if(8<piano)piano=0;
			_delay_ms(1000);
		}
    }
}

SIGNAL(TIMER0_OVF_vect)
{
	cli();
	TCNT0=184;
	if(Step_flag)
		timer0Cnt++;	
	
	if(timer0Cnt==2)
	{
		timer0Cnt = 0;
		PORTD=Step[mot_cnt];
		if(dir==DIR_R)
		{
			if(mot_cnt++==7) mot_cnt=0;
		}
		else if(mot_cnt--==0) mot_cnt=7;
	}
	
	sei();
}


