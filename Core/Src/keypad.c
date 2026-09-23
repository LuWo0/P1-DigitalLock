#include "keypad.h"

typedef struct {
	GPIO_TypeDef *port;
	uint32_t pin;
} keypad_pin_t;

static const keypad_pin_t col_pin[NUM_OF_COLS] = {
	{ GPIOB, GPIO_PIN_8 },
	{ GPIOB, GPIO_PIN_9 },
	{ GPIOC, GPIO_PIN_7 },
};

static const keypad_pin_t row_pin[NUM_OF_ROWS] = {
	{ GPIOA, GPIO_PIN_6 },
	{ GPIOA, GPIO_PIN_7 },
	{ GPIOB, GPIO_PIN_6 },
	{ GPIOC, GPIO_PIN_1 },
};

static const int keymap[NUM_OF_ROWS][NUM_OF_COLS] = {
	{ 1, 2, 3 },
	{ 4, 5, 6 },
	{ 7, 8, 9 },
	{ STAR, 0, POUND },
};

static uint32_t pin_index(uint32_t pin) {
	uint32_t i = 0;
	while (((pin >> i) & 1U) == 0U) {
		i++;
	}
	return i;
}

static void pin_output(GPIO_TypeDef *port, uint32_t pin) {
	uint32_t i = pin_index(pin);
	port->MODER &= ~(3U << (i * 2U));
	port->MODER |= (1U << (i * 2U));
	port->OTYPER &= ~pin;
}

static void pin_input_pulldown(GPIO_TypeDef *port, uint32_t pin) {
	uint32_t i = pin_index(pin);
	port->MODER &= ~(3U << (i * 2U));
	port->PUPDR &= ~(3U << (i * 2U));
	port->PUPDR |= (2U << (i * 2U));
}

static void cols_all_high(void) {
	for (int32_t c = 0; c < NUM_OF_COLS; c++) {
		col_pin[c].port->BSRR = col_pin[c].pin;
	}
}

static int32_t row_is_high(int32_t r) {
	return (row_pin[r].port->IDR & row_pin[r].pin) != 0U;
}

static int32_t any_row_high(void) {
	for (int32_t r = 0; r < NUM_OF_ROWS; r++) {
		if (row_is_high(r)) {
			return 1;
		}
	}
	return 0;
}

void keypad_init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN
			| RCC_AHB2ENR_GPIOCEN;

	for (int32_t r = 0; r < NUM_OF_ROWS; r++) {
		pin_input_pulldown(row_pin[r].port, row_pin[r].pin);
	}
	for (int32_t c = 0; c < NUM_OF_COLS; c++) {
		pin_output(col_pin[c].port, col_pin[c].pin);
	}
	// All columns high so any press pulls its row high
	cols_all_high();
}

int32_t keypad_read(void) {
	if (!any_row_high()) {
		return -1;
	}

	int32_t key = -1;

	for (int32_t c = 0; c < NUM_OF_COLS; c++) {
		for (int32_t i = 0; i < NUM_OF_COLS; i++) {
			col_pin[i].port->BRR = col_pin[i].pin;
		}
		col_pin[c].port->BSRR = col_pin[c].pin;
		HAL_Delay(1);

		for (int32_t r = 0; r < NUM_OF_ROWS; r++) {
			if (row_is_high(r)) {
				key = keymap[r][c];
			}
		}
	}

	cols_all_high();
	return key;
}

int32_t keypad_getkey(void) {
	int32_t key;

	do {
		key = keypad_read();
	} while (key < 0);

	HAL_Delay(20);
	key = keypad_read();

	while (any_row_high()) {
	}

	HAL_Delay(20);
	return key;
}
