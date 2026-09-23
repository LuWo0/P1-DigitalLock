#include "lcd.h"

static void lcd_gpio_init(void);
static void lcd_set_data(uint8_t data);
static void lcd_pulse_e(void);
static void lcd_write(uint8_t rs, uint8_t data);

/**
 * @brief  LCD GPIO as push-pull outputs. E / RS / data idle low.
 */
static void lcd_gpio_init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

	// PA0, PA1, PA4, PA8 (RS), PA9 (E), PA10
	GPIOA->MODER &= ~((3U << 0) | (3U << 2) | (3U << 8) | (3U << 16)
			| (3U << 18) | (3U << 20));

	GPIOA->MODER |= ((1U << 0) | (1U << 2) | (1U << 8) | (1U << 16)
			| (1U << 18)| (1U << 20));

	GPIOA->OTYPER &= ~(GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |
	GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);

	// PB0, PB4, PB5, PB10
	GPIOB->MODER &= ~((3U << 0) | (3U << 8) | (3U << 10) | (3U << 20));
	GPIOB->MODER |= ((1U << 0) | (1U << 8) | (1U << 10) | (1U << 20));
	GPIOB->OTYPER &= ~(GPIO_PIN_0 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10);

	GPIOA->BRR = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |
	GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
	GPIOB->BRR = GPIO_PIN_0 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10;
}

/**
 * @brief  Drive LCD D0-D7 from one byte.
 */
static void lcd_set_data(uint8_t data) {
	uint32_t a_set = 0;
	uint32_t a_rst = 0
			;
	uint32_t b_set = 0;
	uint32_t b_rst = 0;

	if (data & 0x01) {
		a_set |= GPIO_PIN_0;
	} else {
		a_rst |= GPIO_PIN_0;
	} // D0
	if (data & 0x02) {
		a_set |= GPIO_PIN_1;
	} else {
		a_rst |= GPIO_PIN_1;
	} // D1
	if (data & 0x04) {
		a_set |= GPIO_PIN_4;
	} else {
		a_rst |= GPIO_PIN_4;
	} // D2
	if (data & 0x08) {
		b_set |= GPIO_PIN_0;
	} else {
		b_rst |= GPIO_PIN_0;
	} // D3
	if (data & 0x10) {
		a_set |= GPIO_PIN_10;
	} else {
		a_rst |= GPIO_PIN_10;
	} // D4
	if (data & 0x20) {
		b_set |= GPIO_PIN_5;
	} else {
		b_rst |= GPIO_PIN_5;
	} // D5
	if (data & 0x40) {
		b_set |= GPIO_PIN_4;
	} else {
		b_rst |= GPIO_PIN_4;
	} // D6
	if (data & 0x80) {
		b_set |= GPIO_PIN_10;
	} else {
		b_rst |= GPIO_PIN_10;
	} // D7

	GPIOA->BSRR = a_set | (a_rst << 16);
	GPIOB->BSRR = b_set | (b_rst << 16);
}

/**
 * @brief  Latch on the falling edge of E.
 */
static void lcd_pulse_e(void) {
	LCD_E_PORT->BSRR = LCD_E_PIN;   // E high
	HAL_Delay(1);
	LCD_E_PORT->BRR = LCD_E_PIN;   // E low, LCD captures here
	HAL_Delay(1);
}

/**
 * @brief  Write a command (rs = 0) or character (rs = 1).
 */
static void lcd_write(uint8_t rs, uint8_t data) {
	if (rs) {
		LCD_RS_PORT->BSRR = LCD_RS_PIN;
	} else {
		LCD_RS_PORT->BRR = LCD_RS_PIN;
	}

	lcd_set_data(data);
	lcd_pulse_e();
	HAL_Delay(2);
}

void lcd_command(uint8_t cmd) {
	lcd_write(0, cmd);
}

void lcd_data(uint8_t ch) {
	lcd_write(1, ch);
}

void lcd_print(const char *s) {
	while (*s != '\0') {
		lcd_data((uint8_t) *s);
		s++;
	}
}

// line 0 = first row (0x00), line 1 = second row (0x40)
void lcd_set_line(uint8_t line) {
	lcd_command(line ? 0xC0 : 0x80);
}

/**
 * @brief Initialize the LCD.
 */
void lcd_init(void) {
	lcd_gpio_init();

	HAL_Delay(50);          // wait after power-up

	lcd_command(0x30);      // 8-bit function set
	HAL_Delay(5);
	lcd_command(0x30);
	HAL_Delay(1);
	lcd_command(0x30);

	lcd_command(0x38);      // 8-bit with 2 lines and 5x8 font
	lcd_command(0x08);      // display off
	lcd_command(0x01);      // clear
	lcd_command(0x06);      // increment cursor
	lcd_command(0x0F);      // display on / cursor on / blink on
}
