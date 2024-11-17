/*
 * servo.c
 *
 * Created: 16.10.2024
 * Author : lapie
 */

#include "servo.h"
#include <sam.h>
#include "uart.h"
#include "timer.h"
#include <stdio.h>

#define PB12            12

void enable_servo()
{
    PIOB->PIO_SODR = (1 << PB12); //Set PB12 high
}

void disable_servo()
{
    PIOB->PIO_CODR = (1 << PB12); //Set PB12 low
}

void refresh_servo(uint8_t value)
{
	uint16_t duty_cycle = 900+12*50;

	if (value > 100) //duty cycle between 0-100%
	{
		value = 100;
	}

//use value=50 when value between 45 and 55 (because of the joystick sensitivity)
	if(value>55)
	{
		duty_cycle=900+12*value; //(1220/100)
	}

	if(value<45)
	{
		duty_cycle=900+12*value; //(1220/100)
	}
	
	upd_duty1(duty_cycle);
}
