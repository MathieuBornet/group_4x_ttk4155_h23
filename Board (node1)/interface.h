/*
 * interface.h
 *
 * Created: 27.09.2024 10:19:44
 *  Author: matbo
 */ 
#include <avr/io.h>
#include <util/delay.h>	//To use delays

#ifndef INTERFACE_H_
#define INTERFACE_H_
#define adressAdc 0x1400
#define margin 20
#define middleX 160
#define middleY 156

enum JoystickPos{LEFT,RIGHT,UP,DOWN,NEUTRAL};
struct joystick
{
	uint8_t posX;
	uint8_t posY;
	enum JoystickPos pos;
};

uint8_t leftSlider(void);
uint8_t rightSlider(void);
uint8_t ADC(uint8_t channel);
struct joystick getJoystick(void);
uint8_t normX(uint8_t x);
uint8_t normY(uint8_t y);
uint8_t joystickDir(uint8_t posDX,uint8_t posDY);

void print_table(const unsigned char (*tableAddr)[15], uint8_t _position, uint8_t l1,uint8_t movable);
uint8_t screen(struct joystick joystick_data,const unsigned char (*tableAddr)[15], uint8_t l1,uint8_t movable,uint8_t position);

#endif /* INTERFACE_H_ */