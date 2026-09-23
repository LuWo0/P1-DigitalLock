#ifndef KEYPAD_H
#define KEYPAD_H

#include "main.h"

#define NUM_OF_ROWS  4
#define NUM_OF_COLS  3

#define STAR    10
#define POUND   11

void keypad_init(void);
int32_t keypad_read(void);
int32_t keypad_getkey(void);

#endif
