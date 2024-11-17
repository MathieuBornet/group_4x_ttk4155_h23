/*
 * Programme_test.c
 *
 * Created: 02.09.2024 09:52:13
 * Author : matbo
 */ 
#include <stdio.h>
#include "fonts.h"

#ifndef adressOLEDC
#define adressOLEDC 0x1000
#endif

#ifndef adressOLEDD
#define adressOLEDD 0x1200
#endif

#ifndef lengthOLED
#define lengthOLED 128 //for a display of 128 bits per line
#endif

#ifndef nbrLineOLED
#define nbrLineOLED 8
#endif

#ifndef DISPLAY
#define DISPLAY 1


void printf_test(void);
void displayInit();
void displayWrite(uint8_t line, uint8_t data[lengthOLED]);
void write_c(char data);
void write_d(char data);
void oledReset(void);
void oledPrint(char c);
void oledDiag(void);
void goToColumn(uint8_t column);
void goToLine(uint8_t line);
void setPos(uint8_t line, uint8_t column);

#endif