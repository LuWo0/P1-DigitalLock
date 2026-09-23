#include "main.h"
#include "lcd.h"
#include "keypad.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

void show_screen(const char *line1, const char *line2);
void clear_input(void);
void show_locked(void);
void show_unlocked(void);
void set_led(int locked);
void show_set_pin(void);
void show_confirm(void);
void show_saved(void);
void add_digit(int32_t key);
void unlock_animation(void);

typedef enum {
	LOCKED, UNLOCKED, SET_PIN, CONFIRM,
} state_t;

static state_t state = LOCKED;
static uint32_t curr_pin = DEFAULT_PIN;
static uint32_t new_pin = 0;
static uint32_t input = 0;
static uint8_t digits = 0;
static char typed[5];

int main(void) {

	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();

	lcd_init();
	keypad_init();

	BSP_LED_Init(LED_GREEN);

	/* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

	typed[0] = '\0';
	BSP_LED_On(LED_GREEN);
	show_locked();
	set_led(LOCKED);

	while (1) {
		int32_t key = keypad_getkey();

		switch (state) {

		case LOCKED:
			if (key == STAR) {
				clear_input();
				show_locked();
				set_led(LOCKED);
				state = LOCKED;
			} else if (key >= 0 && key <= 9 && digits < PIN_LEN) {
				add_digit(key);
				show_locked();
				if (digits == PIN_LEN) {
					if (input == curr_pin) {
						state = UNLOCKED;
						clear_input();
						set_led(UNLOCKED);
						unlock_animation();
						show_unlocked();
					} else {
						clear_input();
						show_locked();
					}
				}
			}
			break;

		case UNLOCKED:
			if (key == POUND) {
				clear_input();
				state = SET_PIN;
				show_set_pin();
			} else {
				clear_input();
				state = LOCKED;
				set_led(state);
				show_locked();
			}
			break;

		case SET_PIN:
			if (key == STAR) {
				clear_input();
				show_set_pin();
			} else if (key >= 0 && key <= 9 && digits < PIN_LEN) {
				add_digit(key);
				show_set_pin();
				if (digits == PIN_LEN) {
					new_pin = input;
					clear_input();
					state = CONFIRM;
					show_confirm();
				}
			}
			break;

		case CONFIRM:

			if (key == STAR) {
				clear_input();
				show_confirm();
			} else if (key >= 0 && key <= 9 && digits < PIN_LEN) {
				add_digit(key);
				show_confirm();
				if (digits == PIN_LEN) {
					if (input == new_pin) {
						curr_pin = new_pin;
							clear_input();
							state = LOCKED;
							set_led(LOCKED);
							show_saved();
							HAL_Delay(1000);
							show_locked();
					} else {
						clear_input();
						state = SET_PIN;
						show_set_pin();
					}
				}
			}
			break;
		default:
			break;
		}

	}
	/* USER CODE END 3 */
}

void show_screen(const char *line1, const char *line2) {
	lcd_command(0x01); // Clear
	HAL_Delay(2);

	lcd_set_line(0);
	lcd_print(line1);
	lcd_set_line(1);
	lcd_print(line2);

}

void clear_input(void) {
	input = 0;
	digits = 0;
	typed[0] = '\0';
}

void show_locked(void) {
	show_screen("LOCKED", "ENTER KEY ");
	lcd_print(typed);
}

void show_unlocked(void) {
	show_screen("UNLOCKED", "PRESS KEY 2 LOCK");
}

void show_set_pin(void) {
	show_screen("NEW PIN", "");
	lcd_print(typed);
}

void show_confirm(void) {
	show_screen("CONFIRM", "");
	lcd_print(typed);
}

void show_saved(void){
	show_screen("NEW PIN SAVED", "");
}

void set_led(int locked) {
	if (locked == LOCKED) {
		BSP_LED_On(LED_GREEN);
	} else {
		BSP_LED_Off(LED_GREEN);
	}
}

void add_digit(int32_t key) {
	typed[digits] = (char) (key + 0x30);
	digits++;
	typed[digits] = '\0';
	input = input * 10 + key;
}

void unlock_animation(void) {
	const char *frames[] = {
		"[||||||||]",
		"[||||||  ]",
		"[||||    ]",
		"[||      ]",
		"[        ]",
	};

	lcd_command(0x0C); /* display on, cursor off */
	for (int i = 0; i < 5; i++) {
		show_screen("UNLOCKING", frames[i]);
		HAL_Delay(150);
	}
	lcd_command(0x0F); /* cursor / blink back on */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
	GPIO_InitStruct.Pin = USART_TX_Pin | USART_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
