/*
 * servo.h
 *
 * Created: 16.10.2024
 * Author : lapie
 */

#include <stdint.h>

#ifndef SERVO_H
#define SERVO_H

void enable_servo();
void disable_servo();
void refresh_servo(uint8_t value);

#endif
