/*
 * Timer.c
 *
 * Created: 04.11.2024 09:59:21
 *  Author: lapie
 */ 

#include <sam.h>
#include "uart.h"
#include <stdio.h>
#include "timer.h"
#include "state_machine.h"

#define PB12            12
#define PB13            13
#define PCR_EN          28
#define PCRE1_PID36     4
#define WPRG            2
#define WPKEY_P         24
#define WPKEY_W         16
#define WPKEY_M         8
#define CPRE            0
#define CALG            8
#define CPOL            9
#define CPRD            0
#define PREA            8
#define DIVA            0
#define CHID0			0
#define CHID1           1
#define FCHID1	        16

//MASK
#define PWM_CLK_DIVA_MASK   (0xFF << DIVA)         // Mask DIVA
#define PWM_CLK_PREA_MASK   (0xF << PREA)           // Mask PREA


void setup_pwm() {
	
	printf("setting pwm");
	printf("\n");	
	
	//Peripheral control register
	PMC->PMC_PCR &= ~0x3F;					  // Clear the 6 first bits
	PMC->PMC_PCR |= 36;						  // Set them as PWM ID = 36 (Select PWM module)
	PMC->PMC_PCR |= (1<<PCR_EN);			  //enable peripheral clock for PWM
	PMC->PMC_PCER1 |= (1 << PCRE1_PID36);     // Activate the clock for PWM

	//Write protect command
	REG_PWM_WPCR &= ~PWM_WPCR_WPCMD_Msk;
	REG_PWM_WPCR |= PWM_WPCR_WPCMD(1);
	REG_PWM_WPCR |= PWM_WPCR_WPRG0;
	REG_PWM_WPCR |= PWM_WPCR_WPRG1;
	REG_PWM_WPCR |= PWM_WPCR_WPRG2;
	REG_PWM_WPCR |= PWM_WPCR_WPRG3;
	REG_PWM_WPCR &= ~PWM_WPCR_WPKEY_Msk;
	REG_PWM_WPCR |= PWM_WPCR_WPKEY('PWM');
	
	
	/*
	*
	*SERVO - PB13 - Channel 1
	*
	*/
	
	PIOB->PIO_OER |= (1 << PB13);										// Set PB13 as an output
	REG_PIOB_ABSR |= PIO_ABSR_P13;										// Select PWM for PB13 (peripheral B)
	REG_PIOB_PDR |= PIO_PDR_P13;									    // Set PWM pin to an output
	
	REG_PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(42);					// Set the PWM clock rate to 2MHz (84MHz/42)
	REG_PWM_CMR1 = PWM_CMR_CALG | PWM_CMR_CPRE_CLKA | PWM_CMR_CPOL;     // Enable dual slope PWM and set the clock source as CLKA
	REG_PWM_CPRD1 = 20000;												// Set the PWM frequency 2MHz/(2 * 20000) = 50Hz
	REG_PWM_CDTY1 = 1500;												// Set the PWM duty cycle to 1500 - centre the servo
	REG_PWM_ENA = PWM_ENA_CHID1;
	
	NVIC_EnableIRQ(PWM_IRQn);
	PWM->PWM_IER1 = (0b1 << CHID1);
	
	
	/*
	*
	*MOTOR - PB12 - Channel 0
	*
	*/
		
	PIOB->PIO_OER |= (1 << PB12);										// Set PB12 as an output
	REG_PIOB_ABSR |= PIO_ABSR_P12;										// Select PWM for PB12 (peripheral B)
	REG_PIOB_PDR |= PIO_PDR_P12;									    // Set PWM pin to an output
	
	REG_PWM_CLK |= PWM_CLK_PREB(0) | PWM_CLK_DIVB(2);
	REG_PWM_CMR0 = PWM_CMR_CALG | PWM_CMR_CPRE_CLKB | PWM_CMR_CPOL;     // Enable dual slope PWM and set the clock source as CLKB
	REG_PWM_CPRD0 = 1000;												// Set the PWM frequency 10.5kHz
	REG_PWM_CDTY0 = 0;												
	REG_PWM_ENA = PWM_ENA_CHID0;
	
}



void PWM_Handler() {
	
	if (PWM->PWM_ISR1 & PWM_ISR1_CHID1) {
		
		SM_callback();
		
		PWM->PWM_ISR1;
	}
}

void upd_duty1(uint16_t duty_cycle) 
{
	REG_PWM_CDTYUPD1 = duty_cycle; 
}

void upd_duty0(uint16_t duty_cycle)
{
	REG_PWM_CDTYUPD0 = duty_cycle;
}