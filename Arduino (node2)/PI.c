/*
 * servo.c
 *
 * Created: 16.10.2024
 * Author : lapie
 */

#include "PI.h"
#include "sam.h"
#include <stdio.h>
#include "motor.h"

/*
 * Sets up PI controller, inits pi struct with relevant parameters
 */
void PI_init(PI_controller* controller, float kp, float ki, float dt)
{

	controller->KP = kp;
	controller->KI = ki;

	controller->integral_error = 0;
	controller->prev_error = 0;
	controller->dt = dt;

}

/*
 * Runs PI controller
 */
float PI_run(PI_controller * controller, float current_error)
{


	controller->integral_error += current_error * controller->dt;
	controller->prev_error = current_error;

	float output = controller->KP * current_error + 1.0/100000 * controller->KI * controller->integral_error;

	return output;
}

