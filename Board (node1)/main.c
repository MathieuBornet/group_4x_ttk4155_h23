/*
 * main.c
 *
 * Created: 02.09.2024 09:52:13
 * Author : matbo
 */ 

#include <avr/io.h>
#define F_CPU 4910000 //CPU frequency
#include <util/delay.h>	//To use delays
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>
#include "display.h"
#include "interface.h"
#include "can_comm.h"
#include <string.h>  // For strcpy to print the actual settings on the game

//For printf function
#include <stdio.h>
#define BitSet(Port,Bit) (Port|=(1<<Bit))
#define BitClear(Port,Bit) (Port&=~(1<<Bit))
enum States{menu,game,difficulty,player_select,scores,game_over,pause,credits};
enum Difficulty{easy,medium,hard};
enum States ActualState;

//#define adressAdc 0x1400
#define adressSRAM 0x1800
#define adressOLEDC 0x1000
#define adressOLEDD 0x1200

//How many seconds equals a point
#define secPoint 1

//----------------------------------------------------------------
//Uart

#define BAUDRATE 9600    //Desired Baud Rate
#define BAUD_PRESC (((F_CPU / (BAUDRATE * 16UL))) - 1) //UBRR Value calculation formula

#define bufferLength 20
unsigned char reception[bufferLength];
unsigned char newReception;

void UART_send(unsigned char data);

//-------------------------------------------------------------
//Global variables
uint8_t displayData[lengthOLED];
uint16_t scores_value[3] = {0,0,0};
uint8_t chosenDifficulty=easy;
uint8_t player=0;
uint16_t score=0;
uint8_t position =0;

//To remember which state the user was in before break
uint8_t lastState = 12;//init at a random value (just not menu) because used in menu
uint8_t changedState=1;//Is at 1 if we just changed state

// Function for `fdevopen`, call by `printf`
int uart_putchar(char c, FILE *stream) {
	if (c == '\n') {
		UART_send('\r');  // Add \r in front of \n
	}
	UART_send(c);
	return 0;
}

void resetTable(unsigned char table[],uint8_t tableLength)
{
	int i =0;
	for(i = 0;i<tableLength;i++)
	{
		table[i] = 0;
	}
}

unsigned int addTable(unsigned char table[bufferLength], unsigned char element)
{
	int i = 0;
	for(i = 0;i<bufferLength;i++)
	{
		if(table[i] == 0)
		{
			table[i] = element;
			return 0;
		}
	}
	return 1;//No more length
}

void UART_init(void){
	UBRR0L = (uint8_t)(BAUD_PRESC);
	UBRR0H = (uint8_t)(BAUD_PRESC>>8);
	UCSR0B = (1 << RXEN0)|(1 << TXEN0); //Enable transmeter (and his interrupt) and receiver |(1<<RXCIE0)
	UCSR0C = (1 << URSEL0)|(1 << USBS0)|(3 << UCSZ00);
}

void init(void)
{
	//PORTS
	
	// MOSI, SCK and SS as output, MISO as input
	//Inputs
	DDRB = 0b10110000;
	//Outputs
	DDRC = 0x0f; //the fourth msb are for the jtag
	DDRA = 0xff; //Data
	DDRD = 0b11111110; //PD2 = left button | PD3 = right button |PD5 = PWM (ADC) | PD6 = RW | PD7 = RD 
	DDRE = 0b111; //PE1 = ALE (latch)
	
	MCUCR |= (1 << SRE);  // Activate the SRE
	SFIOR = (1<<XMM2);
	
	resetTable(displayData,lengthOLED);
	
	ActualState = menu;
	
	UART_init();         //Call the USART initialization code
	resetTable(reception,bufferLength);
	
	MCUCR |= (1 << ISC11) | (1<<ISC01); //Falling edge interrupt for INT1 and INT0
	GICR |= (1<<INT1) | (1<<INT0);
	
	sei();

	// Configure Timer1 in Fast PWM mode, non-inverted
	// WGM13:0 = 1111 for Fast PWM (mode 3), COM1B1:0 = 10 to Clear OC0B on Compare Match
	TCCR1A |= (1 << WGM10) | (1 << WGM11) ;  // Fast PWM mode
	TCCR1A |= (1 << COM1A0);  // Clear OC1B on Compare Match (non-inverted mode)

	// Set prescaler to 1 (for the highest possible frequency)
	TCCR1B |= (1 << CS10) | (1 << WGM12) | (1 << WGM13);  // Prescaler 1

	// Initialize compare register for 50% duty cycle
	OCR1A = 127;  // 50% duty cycle (127 is half of 255 for an 8-bit timer)
	
	//Timer3: 1s for the score counting
	//Normal port operation, prescaler 1024 (set when activate the timer) and OCR = 2392
	ETIMSK |= (1<<OCIE3A);//The timer is activated when we launch a game
	OCR3A = 0x12B0;
	TCCR3B |= (1<<WGM32);//CTC mode
	
	
	
	// Redirect stdout to UART
	FILE *uart_output = fdevopen(uart_putchar, NULL);
	if (uart_output != NULL) {
		stdout = uart_output;
	}
	
	displayInit();
	_delay_ms(10);
	
	if(can_init(0))
	{
		printf("Error can \n");
	}
	else
	{
		printf("Can init ok\n");
	}
}

unsigned char UART_receive(void){
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void UART_send(unsigned char data){
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

//Not in const because of the print of the config
 unsigned char tableMenu[8][15] ={
	"New game       ",
	"Difficulty     ",
	"Select player  ",
	"Scores         ",
	"Credits        ",
	"###############",
	"Config:        ",
	"Player   D:    "
};

const unsigned char tableDifficulty[3][15] ={
	"Easy           ",
	"Medium         ",
	"Hard           ",
};

const unsigned char tablePlayer[3][15] ={
	"Player 1       ",
	"Player 2       ",
	"Player 3       ",
};

const unsigned char tableCredits[4][15] ={
	"Final project  ",
	"Mathieu        ",
	"Lenaic         ",
	"Philip         ",
};



//Left touch pad
ISR(INT0_vect)
{
	lastState = ActualState;
	if(ActualState == game)
	{
		changedState = 1;
		TCCR3B &= 0b11111000; //Deactivation of the timer
		TCNT3 = 0;
		ActualState = pause;
	}
	else if(ActualState == pause)
	{
		changedState = 1;
		TCCR3B &= 0b11111000; //Deactivation of the timer
		TCNT3 = 0;
		ActualState = game;
	}
}



//Right touch pad
ISR(INT1_vect)
{
	changedState = 1;
	lastState = ActualState;
	if(ActualState == menu)
	{
		switch(position)
		{
			case 0:	ActualState = game;
			break;
			case 1: ActualState = difficulty;
			break;
			case 2: ActualState = player_select;
			break;
			case 3: ActualState = scores;
			break;
			case 4: ActualState = credits;
			break;
			default:
			break;
		}
	}
	else if(ActualState == difficulty)
	{
		ActualState = menu;
		switch(position)
		{
			case 0:	chosenDifficulty = easy;
			break;
			case 1: chosenDifficulty = medium;
			break;
			case 2: chosenDifficulty = hard;
			break;
			default:
			break;
		}
	}
	else if(ActualState == player_select)
	{
		ActualState = menu;
		switch(position)
		{
			case 0:	player = 0;
			break;
			case 1: player = 1;
			break;
			case 2: player = 2;
			break;
			default:
			break;
		}
	}
	else if(ActualState == credits)
	{
		ActualState = menu;
	}
	else if(ActualState == scores)
	{
		ActualState = menu;
	}
	else if(ActualState == game)
	{
		TCCR3B &= 0b11111000; //deactivation of the timer
		TCNT3 = 0;
		score = 0;
		ActualState = menu;
	}
	else if(ActualState == game_over)
	{
		TCCR3B &= 0b11111000; //Deactivation of the timer
		TCNT3 = 0;
		ActualState = menu;
	}
}

//Timer3
ISR(TIMER3_COMPA_vect) {
	score++;
}


//Put a uint8 in three digits (so in decimal) at a position (line,_position)
void addTableDigits(uint8_t nbr,unsigned char (*tableAddr)[15],uint8_t _position,uint8_t line)
{
	//Put that number into digits
	uint8_t nbr_d1 = nbr/100;
	uint8_t nbr_d2 = (nbr/10)%10;
	uint8_t nbr_d3 = nbr%10;
	
	//change into char and put it in the table to print
	tableAddr[line][_position] = nbr_d1+0x30;
	tableAddr[line][_position+1] = nbr_d2+0x30;
	tableAddr[line][_position+2] = nbr_d3+0x30;
}

int main(void)
{
	init();

	while (1)
	{
		switch (ActualState)
		{
			case menu:
				if(lastState != ActualState)
				{
					char playerNbr = player+0x31;
					char difficultyNbr = chosenDifficulty+0x31;
					tableMenu[7][7]=playerNbr;
					tableMenu[7][12]=difficultyNbr;
					lastState = ActualState;
				}
				position = screen(getJoystick(),&tableMenu[0],8,1,position);
				_delay_ms(200);
			break;
			case difficulty:
			{
				position = screen(getJoystick(),&tableDifficulty[0],3,1,position);
				_delay_ms(200);
			}
			break;
			case game:		
			{
				if(lastState != ActualState)
				{
					//Empty the received buffers 
					struct canData emptyCan = {0};
					
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					emptyCan = can_reception(0);
					emptyCan = can_reception(1);
					
					unsigned char tablePlay[1][15] ={"    Playing    "};//In 2D for compatibility with the function print_table
					print_table(&tablePlay[0],0,1,0);
					lastState = ActualState;
					TCCR3B |= (1<<CS32)|(1<<CS30); //used to activate the timer
				}
				struct joystick value_joy = {0};
				value_joy = getJoystick();
				
				struct canData dataCan = {0};
				dataCan.length = 5;
				dataCan.ID = 1;
				dataCan.data[0] = value_joy.pos;
				dataCan.data[1] = value_joy.posX;
				dataCan.data[2] = value_joy.posY;
				dataCan.data[3] = !((PINB&0x02)>>1);//equals 1 if joystick clicked
				dataCan.data[4] = chosenDifficulty;
				
				if(can_send_message(dataCan,0))
					printf("error during send on can \n");
					
				if(1)
				{
					dataCan = can_reception(0);
					if(dataCan.ID == 1 && dataCan.data[0] == 1)
						ActualState = game_over;
					dataCan = can_reception(1);
					if(dataCan.ID == 1 && dataCan.data[0] == 1)
						ActualState = game_over;
				}
					
				
				_delay_ms(25);//To not surcharge CAN
			}			
			break;
			case player_select:
			{
				position = screen(getJoystick(),&tablePlayer[0],3,1,position);
				_delay_ms(200);
			}
			break;
			case scores:
			{
				if(lastState != ActualState)
				{
					unsigned char tableScores[5][15] =
					{
						"     Scores    ",
						"---------------",
						"Player 1       ",
						"Player 2       ",
						"Player 3       "
					};
					
					for(uint8_t i=0;i<3;i++)
					{
						addTableDigits(scores_value[i],&tableScores[0],10,2+i);
					}
					print_table(&tableScores[0],0,5,0);
					lastState = ActualState;
				}
			}
			break;
			case game_over:
			{
				if(lastState != ActualState)
				{
					unsigned char tableGameOver[3][15] =
								{
									"   Game Over   ",
									"---------------",
									"Score =        "
								};
								
					uint8_t real_score = score/secPoint;//Change in one point equals n seconds
					if(real_score > scores_value[player])
						scores_value[player] = real_score;

					addTableDigits(real_score,&tableGameOver[0],8,2);
					print_table(&tableGameOver[0],0,3,0);
					lastState = ActualState;
					TCCR3B &= 0b11111000; //deactivation of the timer
					TCNT3 = 0;
					score = 0;
				}
			}
			break;
			case pause:
			{
				printf("Score = %d",score);
				if(lastState != ActualState)
				{
					unsigned char tableBreak[1][15] ={"     Break     "};//In 2D for compatibility with the function print_table
					print_table(&tableBreak[0],0,1,0);
					lastState = ActualState;
				}
			}
			break;
			case credits:
			{
				if(lastState != ActualState)
				{
					print_table(&tableCredits[0],0,4,0);
					lastState = ActualState;
				}		
			}
			break;
		}
	}
}

