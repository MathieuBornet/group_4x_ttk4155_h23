/*
 * encoder.c
 *
 * Created: 04.11.2024 14:28:12
 *  Author: lapie
 */

#include "encoder.h"
#include "sam.h"
#include <stdio.h>
#include "uart.h"

#define PCR_EN	28


void setup_encoder()
{
	
	//Peripheral control register
	PMC->PMC_PCR &= ~0x3F;					  // Clear the 6 first bits
	PMC->PMC_PCR |= 33;						  // Set them as TC ID = 29 (Select TC module)
	PMC->PMC_PCR |= (1<<PCR_EN);			  //enable peripheral clock for TC
	PMC->PMC_PCER1 |= (1 << 1);				  // Activation de l'horloge pour TC2, Canal 0
	
	REG_PIOB_WPMR = 0x50494F00; 
	
	PIOC->PIO_ABSR |= PIO_ABSR_P25;			// assign peripheral B
	PIOC->PIO_PUER |= PIO_PUER_P25;			// pull up resistor
	PIOC->PIO_ODR  |= PIO_ODR_P25;			// disable output on the I/O line
	PIOC->PIO_PDR  |= PIO_PDR_P25;			// enable peripheral control on the pin
	
	PIOC->PIO_ABSR |= PIO_ABSR_P26;			// assign peripheral B
	PIOC->PIO_PUER |= PIO_PUER_P26;			// pull up resistor
	PIOC->PIO_ODR  |= PIO_ODR_P26;			// disable output on the I/O line
	PIOC->PIO_PDR  |= PIO_PDR_P26;			// enable peripheral control on the pin
	
	
	TC2->TC_WPMR = 0x54494D00;
	
	REG_TC2_BMR &= ~ TC_BMR_TC0XC0S_Msk;
	REG_TC2_BMR |= 2;
	REG_TC2_BMR |= TC_BMR_QDEN;
	REG_TC2_BMR |= TC_BMR_POSEN;
	
	REG_TC2_CMR0 &= ~TC_CMR_TCCLKS_Msk;
	REG_TC2_CMR0 |= TC_CMR_TCCLKS_XC0;
	REG_TC2_CMR0 &= ~TC_CMR_ETRGEDG_Msk;
	REG_TC2_CMR0 |= TC_CMR_ETRGEDG_RISING;
	REG_TC2_CMR0 &= ~TC_CMR_LDRA_Msk;
	REG_TC2_CMR0 |= TC_CMR_LDRA_RISING;
	REG_TC2_CMR0 |= TC_CMR_ABETRG;
	REG_TC2_CMR0 &= ~TC_CMR_WAVE;
	
	REG_TC2_CCR0 = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

void get_position(motor_settings * motor)
{
	uint16_t index_count =REG_TC2_CV0;
	motor->current_pos=index_count;	
}

void reset_position(void)
{
	REG_TC2_CCR0 |= TC_CCR_SWTRG;
}



