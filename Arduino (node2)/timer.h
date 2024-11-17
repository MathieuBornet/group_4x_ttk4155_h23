/*
 * Timer.h
 *
 * Created: 04.11.2024 09:59:33
 *  Author: lapie
 */ 


#ifndef TIMER_H_
#define TIMER_H_

void setup_pwm();
void setup_timer_int();
void upd_duty1(uint16_t duty_cycle);
void upd_duty0(uint16_t duty_cycle);

#endif /* TIMER_H_ */