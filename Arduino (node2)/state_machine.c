/*
 * state_machine.C
 *
 * Created: 05.11.2024 11:17:36
 *  Author: lapie
 */

#include "state_machine.h"
#include <sam.h>
#include "uart.h"
#include <stdio.h>
#include "motor.h"
#include "servo.h"
#include "can.h"
#include "encoder.h"
#include "IR.h"
#include "PI.h"

#define PB25 25

/*
 * Speed control pid constants
 */
#define P 1		// Proportional
#define I 0.01	// Integral
#define DT 0.02 // 50Hz

motor_settings motor;
MC_STATES MC_state = MC_STATE_INIT;
uint8_t ready_flag = 0;
PI_controller pi;
uint8_t data[8];
uint8_t data_length;
int16_t error = 0;
uint8_t ratio = 1; // 1, 2, 4
float output = 0;
CanMsg msg_tx;

void refresh_solenoid(uint8_t state);

MC_STATES change_state(MC_STATES state)
{
	MC_state = state;
	return state;
}

void state_machine(uint8_t *data_in, uint8_t length)
{
	for (uint8_t i = 0; i < length; i++)
	{
		data[i] = data_in[i];
	}

	data_length = length;

	switch (MC_state)
	{
	case MC_STATE_INIT:
		msg_tx.id = 1;
		msg_tx.length = 3;
		msg_tx.byte[0] = 0;
		msg_tx.byte[1] = 0;
		msg_tx.byte[2] = 0;
		if (ready_flag)
		{
			MC_state = MC_READY;
		}
		break;

	case MC_READY:

		break;

	default:
		MC_state = MC_STATE_INIT;
		break;
	}
}

void SM_callback()
{
	switch (MC_state)
	{
	case MC_STATE_INIT:
		PI_init(&pi, P, I, DT);
		if (init_motor(&motor))
		{
			ready_flag = 1;
		}
		break;

	case MC_READY:
		for (uint8_t i = 0; i < data_length; i++)
		{
			switch (i)
			{
			case 0:
			
				break;

			case 1:
				motor.speed = data[i];
				break;

			case 2:
				refresh_servo(data[i]);
				break;

			case 3:
				refresh_solenoid(data[i]);
				break;

			case 4:
				ratio = data[i] + 1;
				break;

			default:
				break;
			} // end switch
	 } // end for
		
		error = motor.pos - motor.current_pos; 		// get error
		
		if(error>0)
		{
			motor.dir=1;
		}else{
			motor.dir=0;
		}
		
		
		motor.duty = PI_run(&pi, error);		   	// PI control
				
		refresh_motor(&motor,ratio);				//update dutycycle
		
		get_position(&motor); 						// get new position
		
		printf("\n position:");
		printf("%d", motor.pos);
		printf("\n current pos:");
		printf("%d",motor.current_pos);
		
		
		if (get_adc() < 1500)						//Test IR
		{
			msg_tx.byte[0] = 1;
			msg_tx.byte[1] = 1;
			msg_tx.byte[2] = 1;
			can_tx(msg_tx);
			can_printmsg(msg_tx);
			printf("\n perdu \n");
		}

		break;

	default:
		MC_state = MC_STATE_INIT;
		break;
	}
}

void refresh_solenoid(uint8_t state)
{
	if (state)
	{
		PIOB->PIO_CODR = (1 << PB25); // Set PD4 low
	}
	else
	{
		PIOB->PIO_SODR = (1 << PB25); // Set PD4 high
	}
}

