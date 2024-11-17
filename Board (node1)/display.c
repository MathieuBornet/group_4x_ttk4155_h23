/*
 * Programme_test.c
 *
 * Created: 02.09.2024 09:52:13
 * Author : matbo
 */ 
#include <avr/io.h>
#include "display.h"

void write_c(char data)
{
	volatile char *ext_ram = (char *) adressOLEDC;
	
	*ext_ram = data;
}

void write_d(char data)
{
	volatile char *ext_ram = (char *) adressOLEDD;
	
	*ext_ram = data;
}

void setPos(uint8_t line, uint8_t column)
{
	goToLine(line);
	goToColumn(column);
}

void goToLine(uint8_t line)
{
	write_c(0xB0+line);//Set page address
}

void goToColumn(uint8_t column)
{
	write_c(column&(0x0F));//LSB
	write_c(0x10+((column>>4)&(0x0F)));//MSB
}

void printf_test(void)
{
	printf("Hello world !");
}

void oledPrint(char c)
{
	for(uint8_t j = 0; j < 8; j++)
	{
		write_d(pgm_read_word(&font8[c-32][j%8]));
	}
}

void oledDiag(void)
{
	for(uint8_t i = 0; i < nbrLineOLED;i++)
	{
		goToLine(i);
		goToColumn(8*i);
		for(uint8_t k=0; k<8;k++)
			write_d(pgm_read_word(&font8['E'-32][k]));
	}
}

void oledReset(void)
{
	setPos(0,0);
	for(uint8_t i = 0; i < nbrLineOLED;i++)
	{
		setPos(i,0);
		for(uint8_t j = 0; j < lengthOLED; j++)
		{
			write_d(0x00);
		}
	}
}

void displayWrite(uint8_t line, uint8_t data[lengthOLED])
{
	goToLine(line);
	for(uint8_t i=0;i<lengthOLED;i++)
		write_d(data[i]);
}

void displayInit()
{
	write_c(0xae); // display off
	write_c(0xa1); //segment remap
	write_c(0xda); //common pads hardware: alternative
	write_c(0x12);
	write_c(0xc8); //common output scan direction:com63~com0
	write_c(0xa8); //multiplex ration mode:63
	write_c(0x3f);
	write_c(0xd5); //display divide ratio/osc. freq. mode
	write_c(0x80);
	write_c(0x81); //contrast control
	write_c(0x50);
	write_c(0xd9); //set pre-charge period
	write_c(0x21);
	write_c(0x20); //Set Memory Addressing Mode
	write_c(0x02);			//->to page mode
	write_c(0xdb); //VCOM deselect level mode
	write_c(0x30);
	write_c(0xad); //master configuration
	write_c(0x00);
	write_c(0xa4); //out follows RAM content
	write_c(0xa6); //set normal display
	write_c(0xaf); // display on
}

