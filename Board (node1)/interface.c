/*
 * interface.c
 *
 * Created: 27.09.2024 10:19:21
 *  Author: matbo
 */ 
#include "interface.h"

uint8_t ADC(uint8_t channel)
{
	volatile char *ext_ram = (char *) adressAdc; // Start address for the ADC
	uint8_t config = 0b10000000 | channel;
	
	//Write signal and config
	*ext_ram = config;
	_delay_ms(2);
	
	uint8_t data=0;
	data = *ext_ram;
	return data;
}

uint8_t leftSlider(void)
{
	return ADC(2);
}

uint8_t rightSlider(void)
{
	return ADC(3);
}

struct joystick getJoystick(void)
{
	struct joystick data;
	 
	data.posX = normX(ADC(0));
	data.posY = normY(ADC(1));
	data.pos = joystickDir(data.posX,data.posY);
	
	return data;
}

uint8_t normX(uint8_t x)
{
	uint8_t norm_x=0;
	if(x >= 160)
	{
		norm_x = 50+50.*(x-middleX)/(255-middleX);
	}
	else
		norm_x = 50.*(x)/middleX;
		
	return norm_x;
}

uint8_t normY(uint8_t y)
{
	uint8_t norm_y=0;
	if(y >= 160)
	{
		norm_y = 50+50.*(y-middleY)/(255-middleY);
	}
	else
	norm_y = 50.*(y)/middleY;
		
	return norm_y;
}

uint8_t joystickDir(uint8_t posDX,uint8_t posDY)
{
	enum JoystickPos pos;
	
	if(posDX > 50-margin && posDX < 50+margin)
	{
		if(posDY > 50-margin && posDY < 50+margin)
		{
			pos = NEUTRAL;
			return pos;
		}
	}
	
	if(posDY > 50+margin && (posDY > posDX))
	{
		pos = UP;
	}
	else if(posDY < 50-margin && (posDY < posDX))
	{
		pos = DOWN;
	}
	else if (posDX > 50+margin)
	pos = RIGHT;
	else
	pos = LEFT;
	
	return pos;
}

void print_table(const unsigned char (*tableAddr)[15], uint8_t _position, uint8_t l1,uint8_t movable)
{
	setPos(0,0);
	oledReset();
	if(movable)
	{
		setPos(_position, 0);
		oledPrint('@');
	}
	for(uint8_t i = 0; i < l1; i++)
	{
		setPos(i, 8);
		for(uint8_t j = 0; j < 15; j++)
		oledPrint(tableAddr[i][j]);
	}
}

uint8_t screen(struct joystick joystick_data,const unsigned char (*tableAddr)[15], uint8_t l1,uint8_t movable,uint8_t position)
{
	if(movable)
	{
		uint8_t joy_pos =0;
		joy_pos = joystick_data.pos;
		if (joy_pos == UP)
		{
			if(position > 0)
			position -= 1;
		}
		else if (joy_pos == DOWN)
		{
			if(position < l1-1)
			position +=1;
		}
	}
	
	setPos(0,0);
	oledReset();
	if(movable)
	{
		setPos(position, 0);
		oledPrint('@');
	}
	for(uint8_t i = 0; i < l1; i++)
	{
		setPos(i, 8);
		for(uint8_t j = 0; j < 15; j++)
		oledPrint(tableAddr[i][j]);
	}
	return position;
}