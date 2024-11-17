/*
 * main.c
 *
 * Created: 14.10.2024 13:06:00
 * Author : lapie
 */

#include "sam.h"
#include <stdio.h>
#include "uart.h"
#include "servo.h"
#include "can.h"
#include "IR.h"
#include "timer.h"
#include "PI.h"
#include "encoder.h"
#include "motor.h"
#include "state_machine.h"

#define DDB4 4
#define PORTB4 4
#define PB25 25
#define PC23 23
#define F_CPU 84000000
#define Baudrate 9600


void setup();


CanMsg msg;


int main(void)
{

	/* Initialization */
	SystemInit();
	setup();
	uart_init(84000000, 115200);
	setup_pwm();
	setup_IR();
	setup_encoder();
	can_init((CanInit){.brp = 41, .phase1 = 6, .phase2 = 5, .propag = 1}, 0);

	while (1)
	{
		can_rx(&msg);	
		state_machine(&msg.byte, msg.length);
	}

	return 0;
}

void setup()
{
	WDT->WDT_MR = WDT_MR_WDDIS; // disable watchdog

	PMC->PMC_PCER0 = (1 << ID_PIOB);
	PMC->PMC_PCER0 = (1 << ID_PIOC);

	PIOB->PIO_OER = (1 << PB25); // PB25 as output
	PIOB->PIO_PER = (1 << PB25); // PIO control
	
	PIOC->PIO_OER = (1 << PC23); // PB25 as output
	PIOC->PIO_PER = (1 << PC23); // PIO control
}
