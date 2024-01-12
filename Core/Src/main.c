/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

int Debuns = 0;
int select = 0;
int sin_index = 0;
int ten_mili_second = 0;
int flag = 0;
int threshold = 0;
int buffer_index = 0;
int base_sense = 0, current_sense = 0;
int warncount = 0, uart_flag = 1, is_critical = 0, counter = 0, time_flag = 1;
uint8_t dim, light, buzzer, input[15], rx_byte;
char timeFormat[8];
char second[2] = { '0', '0' };
char minute[2] = { '0', '0' };
char hours[2] = { '0', '0' };
int ss = 0, mm = 0, hh = 0;

static const uint16_t sine_table[50] = { 200, 214, 225, 234, 240, 242, 242, 240,
		234, 225, 214, 200, 184, 173, 164, 158, 156, 156, 158, 164, 173, 184,
		200, 216, 227, 236, 241, 242, 242, 241, 236, 227, 216, 200, 185, 174,
		165, 159, 157, 157, 159, 165, 174, 185, 200, 215, 226, 235, 240, 242,
		242 };

static const uint16_t square[4] = { 400, 400, 0, 0 };

static const uint16_t triangle[11] = { 0, 50, 100, 150, 200, 250, 300, 350, 400,
		0, 0 };
typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} pin_type;

typedef struct {
	pin_type digit_activators[4];
	pin_type BCD_input[4];
	uint32_t digits[4];
	uint32_t number;
} seven_segment_type;

seven_segment_type seven_segment = { .digit_activators = { { .port = GPIOC,
		.pin = GPIO_PIN_3 }, { .port = GPIOC, .pin = GPIO_PIN_2 }, { .port =
GPIOC, .pin = GPIO_PIN_1 }, { .port = GPIOC, .pin = GPIO_PIN_0 } }, .BCD_input =
		{ { .port = GPIOC, .pin = GPIO_PIN_6 }, { .port = GPIOC, .pin =
		GPIO_PIN_7 }, { .port = GPIOC, .pin = GPIO_PIN_8 }, { .port =
		GPIOC, .pin = GPIO_PIN_9 } }, .digits = { 0, 0, 0, 0 }, .number = 0 };

void seven_segment_display_decimal(uint32_t n) {
	if (n < 10) {
		HAL_GPIO_WritePin(seven_segment.BCD_input[0].port,
				seven_segment.BCD_input[0].pin,
				(n & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(seven_segment.BCD_input[1].port,
				seven_segment.BCD_input[1].pin,
				(n & 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(seven_segment.BCD_input[2].port,
				seven_segment.BCD_input[2].pin,
				(n & 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(seven_segment.BCD_input[3].port,
				seven_segment.BCD_input[3].pin,
				(n & 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

void seven_segment_deactivate_digits(void) {
	for (int i = 0; i < 4; ++i) {
		HAL_GPIO_WritePin(seven_segment.digit_activators[i].port,
				seven_segment.digit_activators[i].pin, GPIO_PIN_RESET);
	}
}

void seven_segment_activate_digit(uint32_t d) {
	if (d < 4) {
		HAL_GPIO_WritePin(seven_segment.digit_activators[d].port,
				seven_segment.digit_activators[d].pin, GPIO_PIN_SET);
	}
}

void seven_segment_set_num(uint32_t n) {

	seven_segment.number = n;
	for (uint32_t i = 0; i < 4; ++i) {
		seven_segment.digits[3 - i] = n % 10;
		n /= 10;
	}
}

void seven_segment_refresh(void) {
	static uint32_t state = 0;
	static uint32_t last_time = 0;
	static uint32_t second_time = 0;
	static int integer = 0;

	if (HAL_GetTick() - last_time > 4) {

		seven_segment_display_decimal(seven_segment.digits[state]);
		seven_segment_deactivate_digits();

		if (current_sense < threshold) {
			if (select == state) {
				if (integer == 0) {
					HAL_GPIO_WritePin(
							seven_segment.digit_activators[select].port,
							seven_segment.digit_activators[select].pin, 1);
					seven_segment_activate_digit(state);
				}
				if (HAL_GetTick() - second_time > 500) {
					integer = !integer;
					second_time = HAL_GetTick();
				}
			} else {
				seven_segment_activate_digit(state);
			}
		} else {
			if (integer == 0) {
				HAL_GPIO_WritePin(seven_segment.digit_activators[select].port,
						seven_segment.digit_activators[select].pin, 1);
				seven_segment_activate_digit(state);
			}
			if (HAL_GetTick() - second_time > 500) {
				integer = !integer;
				second_time = HAL_GetTick();
			}
		}

		seven_segment_display_decimal(seven_segment.digits[state]);
		state = (state + 1) % 4;
		last_time = HAL_GetTick();

	}

}

//void seven_segment_refresh(void) {
//	static uint32_t state = 0;
//	static uint32_t last_time = 0;
//	static uint32_t second_time = 0;
//	static int integer = 0;
//
//	if (HAL_GetTick() - last_time > 4) {
//
//		seven_segment_display_decimal(seven_segment.digits[state]);
//		seven_segment_deactivate_digits();
//
//		if (select == state) {
//			if (integer == 0) {
//				HAL_GPIO_WritePin(seven_segment.digit_activators[select].port,
//						seven_segment.digit_activators[select].pin, 1);
////				if (current_sense < threshold)
//				seven_segment_activate_digit(state);
//			}
//			if (HAL_GetTick() - second_time > 500) {
//				integer = !integer;
//				second_time = HAL_GetTick();
//			}
//		} else {
//			seven_segment_activate_digit(state);
//		}
//
//		seven_segment_display_decimal(seven_segment.digits[state]);
//		state = (state + 1) % 4;
//		last_time = HAL_GetTick();
//
//	}
//
//}

void seven_refresh() {
	static uint32_t second_time = 0;
	static int on = 0;
	if (HAL_GetTick() - second_time > 500 && !on) {
		second_time = HAL_GetTick();
		seven_segment_refresh();
		on = 1;
	} else if (HAL_GetTick() - second_time > 500 && on) {
		second_time = HAL_GetTick();
		seven_segment_deactivate_digits();
		on = 0;
	}
}

void programInit() {

	seven_segment_set_num(0);

}

void programLoop() {
	seven_segment_refresh();
}

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */

TIM_HandleTypeDef *pwm_timer = &htim3;
uint32_t pwm_channel = TIM_CHANNEL_2;

void PWM_Start() {
	HAL_TIM_PWM_Start(pwm_timer, pwm_channel);
}

void PWM_Change_Tone(uint16_t pwm_freq, uint16_t volume) {
	if (pwm_freq == 0 || pwm_freq > 20000) {
		__HAL_TIM_SET_COMPARE(pwm_timer, pwm_channel, 0);
	} else {
		const uint32_t internal_clock_freq = HAL_RCC_GetSysClockFreq();
		const uint16_t prescaler = 1 + internal_clock_freq / pwm_freq / 60000;
		const uint32_t timer_clock = internal_clock_freq / prescaler;
		const uint32_t period_cycles = timer_clock / pwm_freq;
		const uint32_t pulse_width = volume * period_cycles / 1000 / 2;

		pwm_timer->Instance->PSC = prescaler - 1;
		pwm_timer->Instance->ARR = period_cycles - 1;
		pwm_timer->Instance->EGR = TIM_EGR_UG;
		__HAL_TIM_SET_COMPARE(pwm_timer, pwm_channel, pulse_width);
	}
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC3_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

RTC_TimeTypeDef myTime;
RTC_DateTypeDef myDate;
char timeStr[20];
void TurnOnLed() {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 1);
}
void TurnOffLed() {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 0);
}

void printTime() {
	int size = sprintf(timeStr, "%2d:%2d:%2d\n", myTime.Hours, myTime.Minutes,
			myTime.Seconds);
	HAL_UART_Transmit(&huart1, timeStr, size, 1000);
}

void checkwarn() {

	if (current_sense >= threshold) {
		if (is_critical == 0) {
			is_critical = 1;
			warncount++;
		}
		//warncount++;
		TurnOffLed();
		seven_segment_set_num(current_sense);
		if (counter % 2 == 0) {

		}
//			seven_segment_set_num(0);
//		seven_refresh();
		is_critical = 1;
	} else {
		is_critical = 0;
		seven_segment.digits[0] = dim;
		seven_segment.digits[1] = light;
		seven_segment.digits[2] = buzzer;
		seven_segment.digits[3] = warncount;
	}

//	if (flag2 == 1 && current_sense * 20 < threshold + base_sense) {
//
//	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_13 && HAL_GetTick() - Debuns > 300) {
		if (flag) {
			Debuns = HAL_GetTick();
			select = (select + 1) % 3;

		} else {
			TurnOffLed();
			flag = !flag;
			threshold = threshold + base_sense;
			programInit();
		}

	} else if (GPIO_Pin == GPIO_PIN_14 && HAL_GetTick() - Debuns > 300) {
		Debuns = HAL_GetTick();
		if (select == 0) {
			seven_segment.digits[select] = (seven_segment.digits[select] + 1)
					% 10;
			dim = seven_segment.digits[select];

			if (seven_segment.digits[select] != 0) {
				HAL_UART_Transmit(&huart1, "\n[INFO] DimStep increased\n", 26,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] DimStep increased\n", 26,
				HAL_MAX_DELAY);
			} else {
				HAL_UART_Transmit(&huart1, "\n[INFO] DimStep Decreased\n", 26,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] DimStep Decreased\n", 26,
				HAL_MAX_DELAY);
			}
		} else if (select == 1) {
			seven_segment.digits[select] = (seven_segment.digits[select] + 1)
					% 5;
			light = seven_segment.digits[select];
			if (seven_segment.digits[select] != 0) {
				HAL_UART_Transmit(&huart1, "\n[INFO] LIGHTS increased\n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] LIGHTS increased\n", 25,
				HAL_MAX_DELAY);
			} else {
				HAL_UART_Transmit(&huart1, "\n[INFO] LIGHTS Decreased\n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] LIGHTS Decreased\n", 25,
				HAL_MAX_DELAY);
			}

		} else {
			seven_segment.digits[select] = (seven_segment.digits[select] + 1)
					% 4;
			buzzer = seven_segment.digits[select];
			if (seven_segment.digits[select] != 0) {
				HAL_UART_Transmit(&huart1, "\n[INFO] WAVE increased\n", 23,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] WAVE increased\n", 23,
				HAL_MAX_DELAY);
			} else {
				HAL_UART_Transmit(&huart1, "\n[INFO] WAVE Decreased\n", 23,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] WAVE Decreased\n", 23,
				HAL_MAX_DELAY);
			}
		}
		printTime();
	} else if (GPIO_Pin == GPIO_PIN_15 && HAL_GetTick() - Debuns > 300) {
		Debuns = HAL_GetTick();
		int k = seven_segment.digits[select] - 1;
		if (select == 1) {
			if (k < 0) {
				seven_segment.digits[select] = 4;
				light = seven_segment.digits[select];
				HAL_UART_Transmit(&huart1, "\n[INFO] LIGHT INCREASED \n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] LIGHT INCREASED \n", 25,
				HAL_MAX_DELAY);
			} else {
				seven_segment.digits[select] = k;
				light = seven_segment.digits[select];
				HAL_UART_Transmit(&huart1, "\n[INFO] LIGHT Decreased \n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] LIGHT Decreased \n", 25,
				HAL_MAX_DELAY);
			}

		} else if (select == 2) {
			if (k < 0) {
				seven_segment.digits[select] = 3;
				buzzer = seven_segment.digits[select];
				HAL_UART_Transmit(&huart1, "\n[INFO] WAVE INCREASED \n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] WAVE INCREASED \n", 25,
				HAL_MAX_DELAY);
			} else {
				seven_segment.digits[select] = k;
				buzzer = seven_segment.digits[select];
				HAL_UART_Transmit(&huart1, "\n[INFO] WAVE Decreased \n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] WAVE Decreased \n", 25,
				HAL_MAX_DELAY);
			}
		} else {
			if (k < 0) {
				seven_segment.digits[select] = 9;
				dim = seven_segment.digits[select];
				HAL_UART_Transmit(&huart1, "\n[INFO] DIMSTEP INCREASED \n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] DIMSTEP INCREASED \n", 25,
				HAL_MAX_DELAY);
			} else {
				seven_segment.digits[select] = k;
				dim = seven_segment.digits[select];
				HAL_UART_Transmit(&huart1, "\n[INFO] DIMSTEP Decreased \n", 25,
				HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, "\n[INFO] DIMSTEP Decreased \n", 25,
				HAL_MAX_DELAY);
			}
		}
		printTime();
	}

}

void uart_rx_enable_it(void) {
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {

		input[buffer_index++] = rx_byte;
		HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

		if (rx_byte == '\n') {

			if (time_flag) {
				HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_13);
				HAL_RTC_GetTime(&hrtc, &myTime, RTC_FORMAT_BIN);
				hours[0] = input[0];
				hours[1] = input[1];

				minute[0] = input[3];
				minute[1] = input[4];

				second[0] = input[6];
				second[1] = input[7];

				for (int i = 0; i < 8; i++)
					timeFormat[i] = input[i];
				HAL_UART_Transmit(&huart1, input, 10, HAL_MAX_DELAY);

//				myTime.Hours = atoi(hours);
				myTime.Hours = (hours[0] - '0') * 10 + (hours[1] - '0');
//				myTime.Minutes = atoi(minute);
				myTime.Minutes = (minute[0] - '0') * 10 + (minute[1] - '0');
//				myTime.Seconds = atoi(second);
				myTime.Seconds = (second[0] - '0') * 10 + (second[1] - '0');

				HAL_RTC_SetTime(&hrtc, &myTime, RTC_FORMAT_BIN);
				if (input[2] == ':')
					time_flag = 0;

			}

//			if (time_flag == 0)
//				HAL_UART_Transmit(&huart1, input, 10, HAL_MAX_DELAY);

			buffer_index = 0;
			if (strncmp(input, "[DIMSTEP]:", 10) == 0) {
				if (input[10] - 48 >= 0 && input[10] - 48 <= 9) {
					dim = input[10] - 48;
					if (seven_segment.digits[0] > input[10] - 48) {
						int differ = seven_segment.digits[0] - (input[10] - 48);
						char message[60];
						sprintf((char*) message, "\nDimStep Deceased : {%d}\n",
								differ);
						HAL_UART_Transmit(&huart1, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						HAL_UART_Transmit(&huart2, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						seven_segment.digits[0] = input[10] - 48;

					} else if (seven_segment.digits[0] < input[10] - 48) {
						int differ = (input[10] - 48) - seven_segment.digits[0];
						char message[60];
						sprintf((char*) message, "\nDimStep Increased : {%d}\n",
								differ);
						HAL_UART_Transmit(&huart1, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						HAL_UART_Transmit(&huart2, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						seven_segment.digits[0] = input[10] - 48;
					}
				} else {
					HAL_UART_Transmit(&huart1, "\nOut of range!\n", 15,
					HAL_MAX_DELAY);
					HAL_UART_Transmit(&huart2, "\nOut of range!\n", 15,
					HAL_MAX_DELAY);
				}

			} else if (strncmp(input, "[LIGHTS]:", 9) == 0) {
				if (input[9] - 48 >= 0 && input[9] - 48 <= 4) {
					light = input[9] - 48;
					if (seven_segment.digits[1] > input[9] - 48) {
						int differ = seven_segment.digits[1] - (input[9] - 48);
						char message[60];
						sprintf((char*) message, "\nLights Deceased : {%d}\n",
								differ);
						HAL_UART_Transmit(&huart1, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						HAL_UART_Transmit(&huart2, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						seven_segment.digits[1] = input[9] - 48;

					} else if (seven_segment.digits[1] < input[9] - 48) {
						int differ = (input[9] - 48) - seven_segment.digits[1];
						char message[60];
						sprintf((char*) message, "\nLights Increased : {%d}\n",
								differ);
						HAL_UART_Transmit(&huart1, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						HAL_UART_Transmit(&huart2, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						seven_segment.digits[1] = input[9] - 48;
					}
				} else {
					HAL_UART_Transmit(&huart1, "\nOut of range!\n", 15,
					HAL_MAX_DELAY);
					HAL_UART_Transmit(&huart2, "\nOut of range!\n", 15,
					HAL_MAX_DELAY);
				}

			} else if (strncmp(input, "[WARNNUM]:", 10) == 0) {
				if (input[10] - 48 >= 0 && input[10] - 48 <= 3) {
					buzzer = input[10] - 48;
					if (seven_segment.digits[2] > input[10] - 48) {
						int differ = seven_segment.digits[2] - (input[10] - 48);
						char message[60];
						sprintf((char*) message, "\nWarnNum Deceased : {%d}\n",
								differ);
						HAL_UART_Transmit(&huart1, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						HAL_UART_Transmit(&huart2, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						seven_segment.digits[2] = input[10] - 48;

					} else if (seven_segment.digits[2] < input[10] - 48) {
						int differ = (input[10] - 48) - seven_segment.digits[2];
						char message[60];
						sprintf((char*) message, "\nWarnNum Increased : {%d}\n",
								differ);
						HAL_UART_Transmit(&huart1, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						HAL_UART_Transmit(&huart2, (uint8_t*) message,
								strlen((char*) message),
								HAL_MAX_DELAY);
						seven_segment.digits[2] = input[10] - 48;
					}
				} else {
					HAL_UART_Transmit(&huart1, "\nOut of range!\n", 15,
					HAL_MAX_DELAY);
					HAL_UART_Transmit(&huart2, "\nOut of range!\n", 15,
					HAL_MAX_DELAY);
				}

			} else {
				HAL_UART_Transmit(&huart1,
						"\n[ERR] Not valid Value – When working with UART\n",
						48,
						HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,
						"\n[ERR] Not valid Value – When working with UART\n",
						48,
						HAL_MAX_DELAY);
			}
			printTime();
		}

	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {

		HAL_ADC_Start_IT(&hadc1);
		HAL_ADC_Start_IT(&hadc3);
		counter++;
		if (flag) {

			if (buzzer == 1 && is_critical) {
				sin_index = (sin_index + 1) % 50;
				PWM_Change_Tone(sine_table[sin_index], 50);
			} else if (buzzer == 2 && is_critical) {
				sin_index = (sin_index + 1) % 4;
				PWM_Change_Tone(square[sin_index], 50);
			} else if (buzzer == 3 && is_critical) {
				sin_index = (sin_index + 1) % 11;
				PWM_Change_Tone(triangle[sin_index], 50);
			} else {
				PWM_Change_Tone(0, 10);
			}

			int intensity = 100 * seven_segment.digits[0]
					+ ten_mili_second * 10;

			if (seven_segment.digits[1] == 1) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
				__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
			} else if (seven_segment.digits[1] == 2) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
				__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
			} else if (seven_segment.digits[1] == 3) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, intensity);
				__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
			} else if (seven_segment.digits[1] == 4) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, intensity);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, intensity);
				__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, intensity);
			} else {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
				__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
			}
		}

	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1) {

		ten_mili_second = HAL_ADC_GetValue(&hadc1);

		if (!flag) {
			threshold = 20 * ten_mili_second;
			seven_segment_set_num(threshold + base_sense);
//			unsigned char data[100] = "SALAM";
//			int n = sprintf(data, "%d\n", threshold + base_sense);
//			HAL_UART_Transmit(&huart1, data, n, 1000);
		} else {

			double y = ten_mili_second / 62.0;
			if (seven_segment.digits[0] == 0)
				ten_mili_second = 0;
			else
				ten_mili_second = ((y) * 19) - 9;
		}

//		unsigned char data[100] = "SALAM";
//		int n = sprintf(data, "%d\n", ten_mili_second);
//		HAL_UART_Transmit(&huart1, data, n, 1000);
	}
	if (hadc->Instance == ADC3) {
		if (!flag) {
			base_sense = HAL_ADC_GetValue(&hadc3);
			TurnOnLed();
		} else {
			current_sense = HAL_ADC_GetValue(&hadc3);

//			unsigned char data[100] = "SALAM";
//			int n = sprintf(data, "current_sense: %d\n", current_sense);
//			HAL_UART_Transmit(&huart1, data, n, 1000);

			checkwarn();

		}

	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_PCD_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_ADC3_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);
//	HAL_TIM_B
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_ADC_Start_IT(&hadc1);
	HAL_ADC_Start_IT(&hadc3);
	uart_rx_enable_it();
	programInit();
	PWM_Start();
//
	myTime.Hours = 4;
	myTime.Minutes = 3;
	myTime.Seconds = 32;
	HAL_RTC_SetTime(&hrtc, &myTime, RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc, &myTime, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &myDate, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &myDate, RTC_FORMAT_BIN);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		programLoop();

		HAL_RTC_GetTime(&hrtc, &myTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &myDate, RTC_FORMAT_BIN);

		HAL_UART_Transmit(&huart2, "reza\n", 5, HAL_MAX_DELAY);

//		char stringToSend[] = "Hello from microcontroller!\r\n";
//		HAL_UART_Transmit(&huart1, stringToSend, strlen(stringToSend), HAL_MAX_DELAY);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_TIM1
                              |RCC_PERIPHCLK_ADC12|RCC_PERIPHCLK_ADC34;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.Adc34ClockSelection = RCC_ADC34PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_6B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc3.Init.Resolution = ADC_RESOLUTION_10B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc3, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 39;
  hrtc.Init.SynchPrediv = 999;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 47;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 2400-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 47;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin
                           MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = DRDY_Pin|MEMS_INT3_Pin|MEMS_INT4_Pin|MEMS_INT1_Pin
                          |MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
                           LD7_Pin LD9_Pin LD10_Pin LD8_Pin
                           LD6_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC6 PC7 PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
