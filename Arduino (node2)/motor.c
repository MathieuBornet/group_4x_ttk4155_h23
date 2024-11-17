/*
 * motor.c
 *
 * Created: 05.11.2024 09:49:32
 *  Author: lapie
 */

#include "motor.h"
#include <sam.h>
#include "uart.h"
#include "timer.h"
#include "encoder.h"
#include <stdio.h>
#include <stdlib.h>

#define DUTY_MAX 1000
#define PC23 23

uint16_t duty_cycle;


uint8_t init_motor(motor_settings *motor)
{
	reset_position();

	return 1;
}

void refresh_motor(motor_settings *motor, uint8_t ratio)
{
	if (motor->speed > 55)
	{		
		if (motor->pos < 2800)
		{
			motor->pos += (motor->speed - 49)/ratio;
		}
	}

	if (motor->speed < 45)
	{
		
		if (motor->pos > 0)
		{
			motor->pos -= (49 - motor->speed)/ratio;
		}
	}

	if (motor->dir)
	{
		PIOC->PIO_CODR = (1 << PC23); // Set PC23 low
	}
	else
	{
		PIOC->PIO_SODR = (1 << PC23); // Set PC23 high
	}

	if (motor->duty >= DUTY_MAX || motor->duty <= -DUTY_MAX)
	{
		motor->duty = DUTY_MAX;
	}

	if (motor->duty > 0)
	{
		duty_cycle = motor->duty;
	}else{
		duty_cycle = -(motor->duty);
	}

	upd_duty0(duty_cycle);
}
