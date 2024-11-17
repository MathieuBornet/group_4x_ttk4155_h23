/*
 * motor.h
 *
 * Created: 05.11.2024 09:49:32
 *  Author: lapie
 */


#include <stdio.h>

#ifndef MOTOR_H_
#define MOTOR_H_

typedef struct
{
	uint8_t dir;			//direction from error
	uint8_t speed;			//speed from joystick
	float duty;				//duty from PI
	int16_t pos;			//position from speed
	int16_t current_pos;	//position from encoder
} motor_settings;

uint8_t init_motor(motor_settings * motor);
void refresh_motor(motor_settings *motor, uint8_t ratio);



#endif /* MOTOR_H_ */ 