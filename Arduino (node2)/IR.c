/*
 * IR.c
 *
 * Created: 16.10.2024
 * Author : lapie
 */
#include <sam.h>
#define ADC_CDR_DATA_Msk		 0x00000FFF
#define PCRE1_PID37				 5


void setup_IR()
{
	//Peripheral control register
	PMC->PMC_PCR &= ~0x3F;					  // Clear the 6 first bits
	PMC->PMC_PCR |= 37;						  // Set them as ADC ID = 37
	PMC->PMC_PCER1 |= (1 << PCRE1_PID37);     // Activate the clock for PWM
	
	ADC->ADC_MR |= (1 << 7); //sets the mode register in freerun mode
	ADC->ADC_CHER |= (1 << 11);  //enables the channel 11, linked to PB18
	ADC->ADC_CR |= (1 << 1);  //launches the first conversion at initialization by putting START at 1

}

uint32_t get_adc()
{
	uint32_t  adc_ch_11 = ADC->ADC_CDR[11] & ADC_CDR_DATA_Msk;
	uint32_t latestData = ADC->ADC_LCDR & ADC_CDR_DATA_Msk;
	return adc_ch_11;
}


