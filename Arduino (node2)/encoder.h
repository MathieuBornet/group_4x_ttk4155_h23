/*
 * encoder.h
 *
 * Created: 04.11.2024 14:28:12
 *  Author: lapie
 */ 

#include "motor.h"

#ifndef ENCODER_H_
#define ENCODER_H_


void setup_encoder(void);
void get_position(motor_settings * motor);
void reset_position(void);

#endif /* ENCODER_H_ */