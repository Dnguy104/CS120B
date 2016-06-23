
#include "io.c"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/sleep.h>
//#include "usart.h"




void red_data(unsigned char data) {
	int i;
	for (i = 7; i >= 0 ; i--) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x80;
		// set SER = next bit of data to be sent.
		PORTB |= (((data >> i) & 0x01) << 4);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x40;
	}
	// set RCLK = 1. Rising edge copies data from the “Shift” register to the
	//“Storage” register
	PORTB |= 0x20;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}

void blue_data(unsigned char data) {
	int i;
	for (i = 7; i >= 0 ; i--) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x04;
	}
	// set RCLK = 1. Rising edge copies data from the “Shift” register to the
	//“Storage” register
	PORTB |= 0x02;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}



void green_data(unsigned char data) {
	int i;
	for (i = 7; i >= 0 ; i--) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x80;
		// set SER = next bit of data to be sent.
		PORTC |= (((data >> i) & 0x01) << 4);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x40;
	}
	// set RCLK = 1. Rising edge copies data from the “Shift” register to the
	//“Storage” register
	PORTC |= 0x20;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}

void serial_data(unsigned char data) {
	int i;
	for (i = 7; i >= 0 ; --i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x08;
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x04;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTC |= 0x02;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}


void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {

	return (b ? x | (0x01 << k) : x & ~(0x01 << k));

}

unsigned char GetBit(unsigned char x, unsigned char k) {

	return ((x & (0x01 << k)) != 0);

}
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerISR() {
	TimerFlag = 1;
}

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B 	= 0x0B;	// bit3 = 1: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: prescaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A 	= 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register

	TIMSK1 	= 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

void TimerOff() {
	TCCR1B 	= 0x00; // bit3bit2bit1bit0=0000: timer off
}


// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; 			// Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { 	// results in a more efficient compare
		TimerISR(); 				// Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}


unsigned long int findGCD (unsigned long int a,
unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

typedef struct task
{
	unsigned long elapsedTime;
	unsigned int state;
	unsigned long period;
	int (*TickFct)(int);
} task;

typedef struct Player
{
	unsigned char x;
	unsigned char y;
} Player;

Player player1;
Player player2;
Player bomb1;
Player bomb11;
Player bomb2;
Player bomb22;

unsigned char player1win = 0;
unsigned char player2win = 0;

enum control1_States
{ control1wait, control1left, control1right} control1_States;
// Monitors button connected to PA0. When the button is
// pressed, shared variable "pause" is toggled.

unsigned char left = 0;
unsigned char right = 0;
unsigned char up = 0;
unsigned char down = 0;

uint16_t input1 = 0x00;
uint16_t input2 = 0x00;
uint16_t input3 = 0x00;
uint16_t input4 = 0x00;
int matrix[8][8];

unsigned char bombdisplayx1 ;
unsigned char bombdisplayy1 ;
unsigned char bombdisplayx ;
unsigned char bombdisplayy ;
unsigned char bombdisplay1x ;
unsigned char bombdisplay1y ;
unsigned char bombdisplay11x ;
unsigned char bombdisplay11y ;
unsigned char matrixindex = 0;
char maplist[] = {0x7E,0xA5,0xFF,0xA5,0xA5,0xFF,0xA5,0x7E};

void matrixinit()
{
	for (unsigned char i = 0; i < 8; ++i)
	{
		for (unsigned char j = 0; j < 8; ++j)
		{
			matrix[i][j] = 2;
		}
	}
	matrix[0][0] = 3;
	matrix[0][7] = 3;
	matrix[1][1] = 3;
	matrix[1][3] = 3;
	matrix[1][4] = 3;
	matrix[1][6] = 3;
	matrix[3][1] = 3;
	matrix[3][3] = 3;
	matrix[3][4] = 3;
	matrix[3][6] = 3;
	matrix[4][1] = 3;
	matrix[4][3] = 3;
	matrix[4][4] = 3;
	matrix[4][6] = 3;
	matrix[6][1] = 3;
	matrix[6][3] = 3;
	matrix[6][4] = 3;
	matrix[6][6] = 3;

	matrix[7][0] = 3;
	matrix[7][7] = 3;
}

static task task1, task2, task3, task33, task4, task5, task6, task7, task8, task9, task10, task11, task12;
task *tasks[] = { &task1, &task2, &task3, &task33, &task4, &task5, &task6, &task7, &task8, &task9, &task10,
&task11, &task12};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

int control1(int state)
{
	Set_A2D_Pin(2);
	delay_ms(2);
	input1 = ADC;
	switch(state)
	{
		case control1wait:
		if (((input1 < 200 ) && (player1.x > 0)) && (matrix[player1.x -1][player1.y] == 2))
		{

			if (matrix[player1.x][player1.y] != 4)
			{
				matrix[player1.x][player1.y] = 2;
			}
			matrix[player1.x-1][player1.y] = 1;
			--player1.x;
			state = control1left;
		} else if (((input1 > 800 ) && (player1.x <= 7)) && (matrix[player1.x +1][player1.y] == 2))
		{
			if (matrix[player1.x][player1.y] != 4)
			{
				matrix[player1.x][player1.y] = 2;
			}
			matrix[player1.x+1][player1.y] = 1;
			++player1.x;
			state = control1right;
		} else
		{
			state = control1wait;
		}
		break;
		
		case control1left:
		if (input1 >400 && input1 < 600)
		{
			state = control1wait;
		} else
		{
			state = control1left;
		}
		break;
		
		case control1right:
		if (input1 >400 && input1 < 600)
		{
			state = control1wait;
		} else
		{
			state = control1right;
		}
		break;
		
		default:
		state = control1wait;
	}
	return state;
}
enum control2_States
{ control2wait, control2left, control2right} control2_States;

int control2(int state) //character movement x player 2
{
	Set_A2D_Pin(5);
	delay_ms(2);
	input2 = ADC;
	switch(state)
	{
		case control2wait:
		if (((input2 > 800 ) && (player2.x > 0)) && (matrix[player2.x -1][player2.y] == 2))
		{

			if (matrix[player2.x][player2.y] != 4)
			{
				matrix[player2.x][player2.y] = 2;
			}
			matrix[player2.x-1][player2.y] = 0;
			--player2.x;
			state = control2left;
		} else if (((input2 < 200 ) && (player2.x <= 7)) && (matrix[player2.x +1][player2.y] == 2))
		{
			if (matrix[player2.x][player2.y] != 4)
			{
				matrix[player2.x][player2.y] = 2;
			}
			matrix[player2.x+1][player2.y] = 0;
			++player2.x;
			state = control2right;
		} else
		{
			state = control2wait;
		}
		break;
		
		case control2left:
		if (input2 >400 && input2 < 600)
		{
			state = control2wait;
		} else
		{
			state = control2left;
		}
		break;
		
		case control2right:
		if (input2 >400 && input2 < 600)
		{
			state = control2wait;
		} else
		{
			state = control2right;
		}
		break;
		
		default:
		state = control2wait;
	}
	return state;
}


enum control4_States
{ control4wait, control4up, control4down} control4_States;


enum control3_States
{ control3wait, control3up, control3down} control3_States;


int control4(int state)
{
	Set_A2D_Pin(1);
	delay_ms(2);
	input3 = ADC;
	switch(state)
	{
		case control4wait:
		if (((input3 > 800 ) && (player1.y < 7)) && (matrix[player1.x][player1.y+1] == 2))
		{
			if (matrix[player1.x][player1.y] != 4)
			{
				matrix[player1.x][player1.y] = 2;
			}
			matrix[player1.x][player1.y+1] = 1;
			++player1.y;
			state = control4up;
		}
		else if (((input3 < 200 ) && (player1.y > 0)) && (matrix[player1.x][player1.y-1] == 2))
		{
			if (matrix[player1.x][player1.y] != 4)
			{
				matrix[player1.x][player1.y] = 2;
			}
			matrix[player1.x][player1.y-1] = 1;
			--player1.y;
			state = control4down;
		}
		break;
		
		case control4up:
		if (input3 >400 && input3 < 600)
		{
			state = control4wait;
		}
		break;
		
		case control4down:
		if (input3 >400 && input3 < 600)
		{
			state = control4wait;
		}
		break;
		
		default:
		state = control4wait;
	}
	
	
	return state;
}


int control3(int state)
{
	Set_A2D_Pin(4);
	delay_ms(2);
	input4 = ADC;
	switch(state)
	{
		case control3wait:
		if (((input4 > 800 ) && (player2.y > 0)) && (matrix[player2.x][player2.y-1] == 2))
		{

			if (matrix[player2.x][player2.y] != 4)
			{
				matrix[player2.x][player2.y] = 2;
			}
			matrix[player2.x][player2.y-1] = 0;
			--player2.y;
			state = control3up;
		}
		else if (((input4 < 200 ) && (player2.y < 7)) && (matrix[player2.x][player2.y+1] == 2))
		{
			if (matrix[player2.x][player2.y] != 4)
			{
				matrix[player2.x][player2.y] = 2;
			}
			matrix[player2.x][player2.y+1] = 0;
			++player2.y;
			state = control3down;
		}
		break;
		
		case control3up:
		if (input4 >400 && input4 < 600)
		{
			state = control3wait;
		}
		break;
		
		case control3down:
		if (input4 >400 && input4 < 600)
		{
			state = control3wait;
		}
		break;
		
		default:
		state = control3wait;
	}
	
	
	return state;
}

enum outdisplay1_States
{ outdisplay1wait, mapdisplay } outdisplay1_States;

unsigned char mapindex = 0;
unsigned char colorcount = 0;
unsigned char colorcount1 = 0;
int outdisplay1(int state)
{
	
	switch(state)
	{
		case outdisplay1wait:
		state = mapdisplay;
		
		break;
		
		case mapdisplay:
		for (mapindex = 0; mapindex < 8 ; mapindex++ )
		{
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << mapindex);
			green_data(maplist[mapindex]);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		break;
		
		default:
		state = outdisplay1wait;
	}


	return state;
}


enum outdisplay2_States
{ outdisplay2wait,
	playerprint1,
	playerprint2,
} outdisplay2_States;

int outdisplay2(int state)
{
	switch(state)
	{
		case outdisplay2wait:
		state = playerprint1;
		
		break;
		
		case playerprint1:
		red_data(0xFF);
		blue_data(0xFF);
		green_data(0xFF);
		serial_data(1 << player2.x);
		blue_data(~(1 << player2.y));
		delay_ms(3);
		state = playerprint2;
		break;
		
		case playerprint2:
		red_data(0xFF);
		blue_data(0xFF);
		green_data(0xFF);
		serial_data(1 << player1.x);
		red_data(~(1 << player1.y));
		delay_ms(3);
		state = playerprint1;
		break;
		

		default:
		state = outdisplay2wait;
	}


	return state;
}

enum bombfunc3_States
{
	starttt, bombclick, bombre
} bombfunc3_States;

enum bombfunc2_States
{
	startt, bombclick1, bombre1
} bombfunc2_States;

unsigned char bomb1x = 0;
unsigned char bomb2x = 0;

unsigned char bomb1_1 = 0;
unsigned char bomb1_2 = 0;
unsigned char bomb2_1 = 0;
unsigned char bomb2_2 = 0;
int bombfunc3(int state)
{
	switch(state)
	{
		case starttt:
		bomb1x = 0;
		bomb1_1 = 0;
		bomb1_2 = 0;
		state = bombclick;
		case bombclick:
		if ( ~PINA & 0x08 && ( bomb1x < 2) )
		{
			if( bomb1_1 == 0)
			{
				matrix[player2.x][player2.y] = 4;
				bomb1.x = player2.x;
				bomb1.y = player2.y;
				bomb1_1 = 1;
				bomb1x++;
			}
			else if ( bomb1_2 == 0)
			{
				matrix[player2.x][player2.y] = 4;
				bomb11.x = player2.x;
				bomb11.y = player2.y;
				bomb1_2 = 1;
				bomb1x++;
			}
			state = bombre;
		}
		
		break;
		
		case bombre:
		if (!(~PINA & 0x08))
		{
			state = bombclick;
		}
		break;

		default:
		state = starttt;
	}


	return state;
}

int bombfunc2(int state)
{
	switch(state)
	{
		case startt:
		bomb2x = 0;
		bomb2_1 = 0;
		bomb2_2 = 0;
		state = bombclick1;
		case bombclick1:
		if ( ~PINA & 0x01 && ( bomb2x < 2) )
		{
			if( bomb2_1 == 0)
			{
				matrix[player1.x][player1.y] = 4;
				bomb2.x = player1.x;
				bomb2.y = player1.y;
				bomb2_1 = 1;
				bomb2x++;
			}
			else if ( bomb2_2 == 0)
			{
				matrix[player1.x][player1.y] = 4;
				bomb22.x = player1.x;
				bomb22.y = player1.y;
				bomb2_2 = 1;
				bomb2x++;
			}
			state = bombre1;
		}
		
		break;
		
		case bombre1:
		if (!(~PINA & 0x01))
		{
			state = bombclick1;
		}
		break;

		default:
		state = startt;
	}


	return state;
}

enum bombon1_States
{
	bombstart, bombon, bombdown, bombup, bombexplode, bombexplode1
} bomb_on1_States;

enum bombon2_States
{
	bombstart2, bombonn, bombdown2, bombup2, bombexplode2, bombexplode12
} bomb_on2_States;

int bombon1(int state)
{
	static unsigned char bombtime1;
	static unsigned char bombtime11;
	static unsigned char temp;
	
	switch(state)
	{
		case bombstart:
		bombtime1 = 0;
		bombtime11 = 0;
		temp = 0;
		bombdisplayx = 0;
		bombdisplayy = 0;
		state = bombon;
		break;
		
		case bombon:
		if (bomb1_1 == 1)
		{
			state = bombdown;
		}
		break;
		
		case bombdown:
		bombtime1++;
		if ( bombtime1 < 75) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb1.x);
			blue_data(~(1 << bomb1.y));
			red_data(~(1 << bomb1.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime1 = 0;
			state = bombup;
		}
		break;
		
		case bombup:
		bombtime1++;
		if ( bombtime1 < 75)
		{
		}
		else
		{
			bombtime11++;
			if ( bombtime11 > 2)
			{
				temp = bomb1.x;
				bombdisplayx |= 1 << bomb1.x;
				while (matrix[temp-1][bomb1.y] != 3 && temp > 0 )
				{
					if (matrix[temp-1][bomb1.y] != 3 )
					{
						bombdisplayx |= (1 << (temp-1));
					}
					temp--;
				}
				temp = bomb1.x;
				bombdisplayx |= 1 << bomb1.x;
				while (matrix[temp+1][bomb1.y] != 3 && temp < 7 )
				{
					if (matrix[temp+1][bomb1.y] != 3 )
					{
						bombdisplayx |= (1 << (temp+1));
					}
					temp++;
				}
				
				temp = bomb1.y;
				bombdisplayy |= 1 << bomb1.y;
				while (matrix[bomb1.x][temp-1] != 3 && temp > 0 )
				{
					if (matrix[bomb1.x][temp-1] != 3 )
					{
						bombdisplayy |= (1 << (temp-1));
					}
					temp--;
				}
				temp = bomb1.y;
				bombdisplayy |= 1 << bomb1.y;
				while (matrix[bomb1.x][temp+1] != 3 && temp < 7 )
				{
					if (matrix[bomb1.x][temp+1] != 3 )
					{
						bombdisplayy |= (1 << (temp+1));
					}
					temp++;
				}
				
				temp = 0;
				bombtime1 = 0;
				bombtime11 = 0;
				
				state = bombexplode;
			}
			else {
				bombtime1 = 0;
				state = bombdown;
			}
		}
		break;
		
		case bombexplode:
		bombtime1++;
		if ( bombtime1 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb1.x);
			red_data(~(bombdisplayy));
			delay_ms(1);
			//blue_data(~(bombdisplayy));
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplayx);
			//blue_data(~(1 << bomb1.y));
			red_data(~(1 << bomb1.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime1 = 0;
			state = bombexplode1;
		}
		
		if ( ( ( bombdisplayx & (1 << (player1.x) ) ) && (bomb1.y == player1.y)) ||
		( (bombdisplayy & (1 << (player1.y)) ) &&  (bomb1.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplayx & (1 << (player2.x) ) ) && (bomb1.y == player2.y)) ||
		( (bombdisplayy & (1 << (player2.y)) ) &&  (bomb1.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		
		case bombexplode1:
		bombtime1++;
		if ( bombtime1 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb1.x);
			blue_data(~(bombdisplayy));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplayx);
			blue_data(~(1 << bomb1.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime11++;
			if ( bombtime11 > 2)
			{
				bombtime1 = 0;
				bombtime11 = 0;
				bomb1_1 = 0;
				bombdisplayx = 0;
				bombdisplayy = 0;
				bomb1x--;
				matrix[bomb1.x][bomb1.y] = 2;
				state = bombon;
			}
			else {
				bombtime1 = 0;
				state = bombexplode;
			}
		}
		
		if ( ( ( bombdisplayx & (1 << (player1.x) ) ) && (bomb1.y == player1.y)) ||
		( (bombdisplayy & (1 << (player1.y)) ) &&  (bomb1.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplayx & (1 << (player2.x) ) ) && (bomb1.y == player2.y)) ||
		( (bombdisplayy & (1 << (player2.y)) ) &&  (bomb1.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		

		default:
		state = bombstart;
	}


	return state;
}

int bombon2(int state)
{
	static unsigned char bombtime2;
	static unsigned char bombtime22;
	static unsigned char temp1;
	
	switch(state)
	{
		case bombstart2:
		bombtime2 = 0;
		bombtime22 = 0;
		temp1 = 0;
		bombdisplayx1 = 0;
		bombdisplayy1 = 0;
		state = bombonn;
		break;
		
		case bombonn:
		if (bomb1_2 == 1)
		{
			state = bombdown2;
		}
		break;
		
		case bombdown2:
		bombtime2++;
		if ( bombtime2 < 75) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb11.x);
			blue_data(~(1 << bomb11.y));
			red_data(~(1 << bomb11.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime2 = 0;
			state = bombup2;
		}
		break;
		
		case bombup2:
		bombtime2++;
		if ( bombtime2 < 75)
		{
		}
		else
		{
			bombtime22++;
			if ( bombtime22 > 2)
			{
				temp1 = bomb11.x;
				bombdisplayx1 |= 1 << bomb11.x;
				while (matrix[temp1-1][bomb11.y] != 3 && temp1 > 0 )
				{
					if (matrix[temp1-1][bomb11.y] != 3 )
					{
						bombdisplayx1 |= (1 << (temp1-1));
					}
					temp1--;
				}
				temp1 = bomb11.x;
				bombdisplayx1 |= 1 << bomb11.x;
				while (matrix[temp1+1][bomb11.y] != 3 && temp1 < 7 )
				{
					if (matrix[temp1+1][bomb11.y] != 3 )
					{
						bombdisplayx1 |= (1 << (temp1+1));
					}
					temp1++;
				}
				
				temp1 = bomb11.y;
				bombdisplayy1 |= 1 << bomb11.y;
				while (matrix[bomb11.x][temp1-1] != 3 && temp1 > 0 )
				{
					if (matrix[bomb11.x][temp1-1] != 3 )
					{
						bombdisplayy1 |= (1 << (temp1-1));
					}
					temp1--;
				}
				temp1 = bomb11.y;
				bombdisplayy1 |= 1 << bomb11.y;
				while (matrix[bomb11.x][temp1+1] != 3 && temp1 < 7 )
				{
					if (matrix[bomb11.x][temp1+1] != 3 )
					{
						bombdisplayy1 |= (1 << (temp1+1));
					}
					temp1++;
				}
				
				temp1 = 0;
				bombtime2 = 0;
				bombtime22 = 0;
				
				state = bombexplode2;
			}
			else {
				bombtime2 = 0;
				state = bombdown2;
			}
		}
		break;
		
		case bombexplode2:
		bombtime2++;
		if ( bombtime2 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb11.x);
			red_data(~(bombdisplayy1));
			delay_ms(1);
			//blue_data(~(bombdisplayy));
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplayx1);
			//blue_data(~(1 << bomb1.y));
			red_data(~(1 << bomb11.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime2 = 0;
			state = bombexplode12;
		}
		
		
		if ( ( ( bombdisplayx1 & (1 << (player1.x) ) ) && (bomb11.y == player1.y)) ||
		( (bombdisplayy1 & (1 << (player1.y)) ) &&  (bomb11.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplayx1 & (1 << (player2.x) ) ) && (bomb11.y == player2.y)) ||
		( (bombdisplayy1 & (1 << (player2.y)) ) &&  (bomb11.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		
		case bombexplode12:
		bombtime2++;
		if ( bombtime2 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb11.x);
			blue_data(~(bombdisplayy1));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplayx1);
			blue_data(~(1 << bomb11.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime22++;
			if ( bombtime22 > 2)
			{
				bombtime2 = 0;
				bombtime22 = 0;
				bomb1_2 = 0;
				bombdisplayx1 = 0;
				bombdisplayy1 = 0;
				bomb1x--;
				matrix[bomb11.x][bomb11.y] = 2;
				state = bombonn;
			}
			else {
				bombtime2 = 0;
				state = bombexplode2;
			}
			
		}
		if ( ( ( bombdisplayx1 & (1 << (player1.x) ) ) && (bomb11.y == player1.y)) ||
		( (bombdisplayy1 & (1 << (player1.y)) ) &&  (bomb11.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplayx1 & (1 << (player2.x) ) ) && (bomb11.y == player2.y)) ||
		( (bombdisplayy1 & (1 << (player2.y)) ) &&  (bomb11.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		

		default:
		state = bombstart2;
	}


	return state;
}

enum bombon11_States
{
	bombs, bombo, bombd, bombu, bombe, bombex
} bomb_on11_States;

enum bombon12_States
{
	bombs1, bombo1, bombd1, bombu1, bombe1, bombex1
} bomb_on12_States;

int bombon12(int state)
{
	static unsigned char bombtime4;
	static unsigned char bombtime44;
	static unsigned char temp4;
	
	switch(state)
	{
		case bombs1:
		bombtime4 = 0;
		bombtime44 = 0;
		temp4 = 0;
		bombdisplay11x = 0;
		bombdisplay11y = 0;
		state = bombo1;
		break;
		
		case bombo1:
		if (bomb2_2 == 1)
		{
			state = bombd1;
		}
		break;
		
		case bombd1:
		bombtime4++;
		if ( bombtime4 < 75) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb22.x);
			blue_data(~(1 << bomb22.y));
			red_data(~(1 << bomb22.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime4 = 0;
			state = bombu1;
		}
		break;
		
		case bombu1:
		bombtime4++;
		if ( bombtime4 < 75)
		{
		}
		else
		{
			bombtime44++;
			if ( bombtime44 > 2)
			{
				temp4 = bomb22.x;
				bombdisplay11x |= 1 << bomb22.x;
				while (matrix[temp4-1][bomb22.y] != 3 && temp4 > 0 )
				{
					if (matrix[temp4-1][bomb22.y] != 3 )
					{
						bombdisplay11x |= (1 << (temp4-1));
					}
					temp4--;
				}
				temp4 = bomb22.x;
				bombdisplay11x |= 1 << bomb22.x;
				while (matrix[temp4+1][bomb22.y] != 3 && temp4 < 7 )
				{
					if (matrix[temp4+1][bomb22.y] != 3 )
					{
						bombdisplay11x |= (1 << (temp4+1));
					}
					temp4++;
				}
				
				temp4 = bomb22.y;
				bombdisplay11y |= 1 << bomb22.y;
				while (matrix[bomb22.x][temp4-1] != 3 && temp4 > 0 )
				{
					if (matrix[bomb22.x][temp4-1] != 3 )
					{
						bombdisplay11y |= (1 << (temp4-1));
					}
					temp4--;
				}
				temp4 = bomb22.y;
				bombdisplay11y |= 1 << bomb22.y;
				while (matrix[bomb22.x][temp4+1] != 3 && temp4 < 7 )
				{
					if (matrix[bomb22.x][temp4+1] != 3 )
					{
						bombdisplay11y |= (1 << (temp4+1));
					}
					temp4++;
				}
				
				temp4 = 0;
				bombtime4 = 0;
				bombtime44 = 0;
				
				state = bombe1;
			}
			else {
				bombtime4 = 0;
				state = bombd1;
			}
		}
		break;
		
		case bombe1:
		bombtime4++;
		if ( bombtime4 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb22.x);
			red_data(~(bombdisplay11y));
			delay_ms(1);
			//blue_data(~(bombdisplayy));
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplay11x);
			//blue_data(~(1 << bomb1.y));
			red_data(~(1 << bomb22.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime4 = 0;
			state = bombex1;
		}
		
		if ( ( ( bombdisplay11x & (1 << (player1.x) ) ) && (bomb22.y == player1.y)) ||
		( (bombdisplay11y & (1 << (player1.y)) ) &&  (bomb22.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplay11x & (1 << (player2.x) ) ) && (bomb22.y == player2.y)) ||
		( (bombdisplay11y & (1 << (player2.y)) ) &&  (bomb22.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		
		case bombex1:
		bombtime4++;
		if ( bombtime4 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb22.x);
			blue_data(~(bombdisplay11y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplay11x);
			blue_data(~(1 << bomb22.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime44++;
			if ( bombtime44 > 2)
			{
				bombtime4 = 0;
				bombtime44 = 0;
				bomb2_2 = 0;
				bombdisplay11x = 0;
				bombdisplay11y = 0;
				bomb2x--;
				matrix[bomb22.x][bomb22.y] = 2;
				state = bombo1;
			}
			else {
				bombtime4 = 0;
				state = bombe1;
			}
		}
		
		if ( ( ( bombdisplay11x & (1 << (player1.x) ) ) && (bomb22.y == player1.y)) ||
		( (bombdisplay11y & (1 << (player1.y)) ) &&  (bomb22.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplay11x & (1 << (player2.x) ) ) && (bomb22.y == player2.y)) ||
		( (bombdisplay11y & (1 << (player2.y)) ) &&  (bomb22.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		

		default:
		state = bombs1;
	}
	
	return state;
}

int bombon11(int state)
{
	static unsigned char bombtime3;
	static unsigned char bombtime33;
	static unsigned char temp3;
	
	switch(state)
	{
		case bombs:
		bombtime3 = 0;
		bombtime33 = 0;
		temp3 = 0;
		bombdisplay1x = 0;
		bombdisplay1y = 0;
		state = bombo;
		break;
		
		case bombo:
		if (bomb2_1 == 1)
		{
			state = bombd;
		}
		break;
		
		case bombd:
		bombtime3++;
		if ( bombtime3 < 75) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb2.x);
			blue_data(~(1 << bomb2.y));
			red_data(~(1 << bomb2.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime3 = 0;
			state = bombu;
		}
		break;
		
		case bombu:
		bombtime3++;
		if ( bombtime3 < 75)
		{
		}
		else
		{
			bombtime33++;
			if ( bombtime33 > 2)
			{
				temp3 = bomb2.x;
				bombdisplay1x |= 1 << bomb2.x;
				while (matrix[temp3-1][bomb2.y] != 3 && temp3 > 0 )
				{
					if (matrix[temp3-1][bomb2.y] != 3 )
					{
						bombdisplay1x |= (1 << (temp3-1));
					}
					temp3--;
				}
				temp3 = bomb2.x;
				bombdisplay1x |= 1 << bomb2.x;
				while (matrix[temp3+1][bomb2.y] != 3 && temp3 < 7 )
				{
					if (matrix[temp3+1][bomb2.y] != 3 )
					{
						bombdisplay1x |= (1 << (temp3+1));
					}
					temp3++;
				}
				
				temp3 = bomb2.y;
				bombdisplay1y |= 1 << bomb2.y;
				while (matrix[bomb2.x][temp3-1] != 3 && temp3 > 0 )
				{
					if (matrix[bomb2.x][temp3-1] != 3 )
					{
						bombdisplay1y |= (1 << (temp3-1));
					}
					temp3--;
				}
				temp3 = bomb2.y;
				bombdisplay1y |= 1 << bomb2.y;
				while (matrix[bomb2.x][temp3+1] != 3 && temp3 < 7 )
				{
					if (matrix[bomb2.x][temp3+1] != 3 )
					{
						bombdisplay1y |= (1 << (temp3+1));
					}
					temp3++;
				}
				
				temp3 = 0;
				bombtime3 = 0;
				bombtime33 = 0;
				
				state = bombe;
			}
			else {
				bombtime3 = 0;
				state = bombd;
			}
		}
		break;
		
		case bombe:
		bombtime3++;
		if ( bombtime3 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb2.x);
			red_data(~(bombdisplay1y));
			delay_ms(1);
			//blue_data(~(bombdisplayy));
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplay1x);
			//blue_data(~(1 << bomb1.y));
			red_data(~(1 << bomb2.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime3 = 0;
			state = bombex;
		}
		
		if ( ( ( bombdisplay1x & (1 << (player1.x) ) ) && (bomb2.y == player1.y)) ||
		( (bombdisplay1y & (1 << (player1.y)) ) &&  (bomb2.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplay1x & (1 << (player2.x) ) ) && (bomb2.y == player2.y)) ||
		( (bombdisplay1y & (1 << (player2.y)) ) &&  (bomb2.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		
		case bombex:
		bombtime3++;
		if ( bombtime3 < 25) {
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(1 << bomb2.x);
			blue_data(~(bombdisplay1y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
			serial_data(bombdisplay1x);
			blue_data(~(1 << bomb2.y));
			delay_ms(1);
			red_data(0xFF);
			blue_data(0xFF);
			green_data(0xFF);
		}
		else
		{
			bombtime33++;
			if ( bombtime33 > 2)
			{
				bombtime3 = 0;
				bombtime33 = 0;
				bomb2_1 = 0;
				bombdisplay1x = 0;
				bombdisplay1y = 0;
				bomb2x--;
				matrix[bomb2.x][bomb2.y] = 2;
				state = bombo;
			}
			else {
				bombtime3 = 0;
				state = bombe;
			}
		}
		if ( ( ( bombdisplay1x & (1 << (player1.x) ) ) && (bomb2.y == player1.y)) ||
		( (bombdisplay1y & (1 << (player1.y)) ) &&  (bomb2.x == player1.x)) )
		{
			player2win = 1;
		}
		if ( ( ( bombdisplay1x & (1 << (player2.x) ) ) && (bomb2.y == player2.y)) ||
		( (bombdisplay1y & (1 << (player2.y)) ) &&  (bomb2.x == player2.x)) )
		{
			player1win = 1;
		}
		break;
		

		default:
		state = bombs;
	}


	return state;
}

enum winfunc_States
{
	winstart, winwait, win1, win2, winreset, win3
} bwinfunc_States;

int winfunc(int state)
{
	static unsigned char output;
	static unsigned char tempi;
	switch(state)
	{
		case winstart:
		player1win = 0;
		player2win = 0;
		output = 0;
		tempi = 0;
		state = winwait;
		break;
		
		case winwait:
		tempi = 0;
		if( player1win == 1 && player2win == 1)
		{
			state = win3;
		}
		if( player1win == 1)
		{
			state = win1;
		}
		else if (player2win == 1)
		{
			state = win2;
		}
		break;
		case win3:
		green_data(0xFF);
		red_data(0xFF);
		blue_data(0xFF);
		serial_data(0xFF);
		for (tempi = 0; tempi < 10 ; tempi++)
		{
			output = ~output;
			blue_data(output);
			red_data(output);
			delay_ms(250);
		}
		matrixinit();
		for( tempi = 0; tempi < numTasks; tempi++)
		{
			tasks[tempi]->state = -1;
		}
		bomb1x = 0;
		bomb2x = 0;

		bomb1_1 = 0;
		bomb1_2 = 0;
		bomb2_1 = 0;
		bomb2_2 = 0;
		bombdisplay1y = 0;
		bombdisplayx = 0;
		bombdisplayy = 0;
		bombdisplayy1 = 0;
		bombdisplayx1 = 0;
		bombdisplay1x = 0;
		bombdisplay11x = 0;
		bombdisplay11y = 0;
		player1.x = 0x02;
		player1.y = 0x02;
		player2.x = 0x05;
		player2.y = 0x05;
		player1win = 0;
		player2win = 0;
		state = winwait;
		break;
		
		
		case win2:
		tempi++;
		if ( tempi > 25)
		{
			green_data(0xFF);
			red_data(0xFF);
			blue_data(0xFF);
			serial_data(0xFF);
			for (tempi = 0; tempi < 10 ; tempi++)
			{
				output = ~output;
				blue_data(output);
				delay_ms(250);
			}
			matrixinit();
			for( tempi = 0; tempi < numTasks; tempi++)
			{
				tasks[tempi]->state = -1;
			}
			bomb1x = 0;
			bomb2x = 0;

			bomb1_1 = 0;
			bomb1_2 = 0;
			bomb2_1 = 0;
			bomb2_2 = 0;
			bombdisplay1y = 0;
			bombdisplayx = 0;
			bombdisplayy = 0;
			bombdisplayy1 = 0;
			bombdisplayx1 = 0;
			bombdisplay1x = 0;
			bombdisplay11x = 0;
			bombdisplay11y = 0;

			player1.x = 0x02;
			player1.y = 0x02;
			player2.x = 0x05;
			player2.y = 0x05;
			player1win = 0;
			player2win = 0;
			state = winwait;
		}
		else
		{
			if( player1win == 1 && player2win == 1)
			{
				
				state = win3;
			}
			
		}
		break;
		
		case win1:
		tempi++;
		if ( tempi > 25)
		{
			green_data(0xFF);
			red_data(0xFF);
			blue_data(0xFF);
			serial_data(0xFF);
			for (tempi = 0; tempi < 10 ; tempi++)
			{
				output = ~output;
				red_data(output);
				delay_ms(250);
			}
			matrixinit();
			for( tempi = 0; tempi < numTasks; tempi++)
			{
				tasks[tempi]->state = -1;
			}
			bomb1x = 0;
			bomb2x = 0;

			bomb1_1 = 0;
			bomb1_2 = 0;
			bomb2_1 = 0;
			bomb2_2 = 0;
			bombdisplay1y = 0;
			bombdisplayx = 0;
			bombdisplayy = 0;
			bombdisplayy1 = 0;
			bombdisplayx1 = 0;
			bombdisplay1x = 0;
			bombdisplay11x = 0;
			bombdisplay11y = 0;

			player1.x = 0x02;
			player1.y = 0x02;
			player2.x = 0x05;
			player2.y = 0x05;
			player1win = 0;
			player2win = 0;
			state = winwait;
		}
		else
		{
			if( player1win == 1 && player2win == 1)
			{
				
				state = win3;
			}
			
		}
		break;
		
		default:
		state = winstart;
	}


	return state;
}




int main(void) {

	//Enable internal pull ups

	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines


	//Recalculate GCD periods for scheduler
	unsigned long int control1_period = 20;
	unsigned long int control4_period = 20;
	unsigned long int outdisplay1_period = 1;
	unsigned long int outdisplay2_period = 1;
	unsigned long int control2_period = 20;
	unsigned long int control3_period = 20;
	unsigned long int bombfunc3_period = 20;
	unsigned long int bombfunc2_period = 20;
	unsigned long int bombon11_period = 1;
	unsigned long int bombon12_period = 1;
	unsigned long int bombon1_period = 1;
	unsigned long int bombon2_period = 1;
	unsigned long int winfunc_period = 1;

	
	// Task 1
	task1.state = -1;
	task1.period = control1_period;
	task1.elapsedTime = control1_period;
	task1.TickFct = &control1;
	// Task 2
	task2.state = -1;
	task2.period = control4_period;
	task2.elapsedTime = control4_period;
	task2.TickFct = &control4;
	//Task 3
	task5.state = -1;
	task5.period = outdisplay1_period;
	task5.elapsedTime = outdisplay1_period;
	task5.TickFct = &outdisplay1;
	task33.state = -1;
	task33.period = outdisplay2_period;
	task33.elapsedTime = outdisplay2_period;
	task33.TickFct = &outdisplay2;
	// Task 4
	task6.state = -1;
	task6.period = bombfunc3_period;
	task6.elapsedTime = bombfunc3_period;
	task6.TickFct = &bombfunc3;
	// 	// Task 5
	task11.state = -1;
	task11.period = bombon1_period;
	task11.elapsedTime = bombon1_period;
	task11.TickFct = &bombon1;
	task8.state = -1;
	task8.period = bombon2_period;
	task8.elapsedTime = bombon2_period;
	task8.TickFct = &bombon2;
	// Task 6
	task4.state = -1;
	task4.period = control2_period;
	task4.elapsedTime = control2_period;
	task4.TickFct = &control2;
	task3.state = -1;
	task3.period = control3_period;
	task3.elapsedTime = control3_period;
	task3.TickFct = &control3;
	task9.state = -1;
	task9.period = bombon11_period;
	task9.elapsedTime = bombon11_period;
	task9.TickFct = &bombon11;
	task10.state = -1;
	task10.period = bombon12_period;
	task10.elapsedTime = bombon12_period;
	task10.TickFct = &bombon12;
	task7.state = -1;
	task7.period = bombfunc2_period;
	task7.elapsedTime = bombfunc2_period;
	task7.TickFct = &bombfunc2;
	task12.state = -1;
	task12.period = winfunc_period;
	task12.elapsedTime = winfunc_period;
	task12.TickFct = &winfunc;
	
	TimerSet(1);
	TimerOn();
	
	A2D_init();
	player1.x = 0x02;
	player1.y = 0x02;
	player2.x = 0x05;
	player2.y = 0x05;
	//unsigned char temp = 0;
	matrixinit();
	//unsigned char tempp = 0;
	//unsigned char stringify[32];
	
	unsigned short i;

	
	while(1)
	{

		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime ==
			tasks[i]->period )
			{
				// Setting next state for task
				tasks[i]->state =
				tasks[i]->TickFct(tasks[i]->state);
				// Reset elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		

		
		while(!TimerFlag);
		TimerFlag = 0;


	}

}




