/*
 * state_machine.h
 *
 * Created: 05.11.2024 11:17:36
 *  Author: lapie
 */ 


#include <stdio.h>

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

typedef enum {
	MC_STATE_INIT = 0,
	MC_READY,

} MC_STATES;

MC_STATES change_state(MC_STATES state);

void state_machine(uint8_t* data_in, uint8_t length);

void SM_callback();


#endif /* STATE_MACHINE_H_ */