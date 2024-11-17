/*
 * PI.h
 *
 * Created: 04.11.2024 14:09:54
 *  Author: lapie
 */ 

#ifndef PI_H_
#define PI_H_


typedef struct
{
	float KP;
	float KI;
	float integral_error;
	float prev_error;
	float dt;
} PI_controller;


void PI_init(PI_controller * controller, float kp, float ki, float dt);
float PI_run(PI_controller * controller, float current_error);


#endif /* PI_H_ */