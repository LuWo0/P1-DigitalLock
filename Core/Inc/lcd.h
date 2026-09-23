#ifndef LCD_H
#define LCD_H

#include "main.h"

#define LCD_RS_PORT     GPIOA
#define LCD_RS_PIN      GPIO_PIN_8
#define LCD_E_PORT      GPIOA
#define LCD_E_PIN       GPIO_PIN_9


void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t ch);
void lcd_print(const char *s);
void lcd_set_line(uint8_t line);

#endif
