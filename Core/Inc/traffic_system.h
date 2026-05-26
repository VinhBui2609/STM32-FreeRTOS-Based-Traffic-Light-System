/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : traffic_system.h
  * @brief          : Header for traffic_system.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_TRAFFIC_SYSTEM_H_
#define INC_TRAFFIC_SYSTEM_H_

/* USER CODE BEGIN Includes */
#include "main.h"
#include "cmsis_os.h"
/* USER CODE END Includes */

#define PRESSED		GPIO_PIN_RESET
#define OFF			GPIO_PIN_RESET
#define ON			GPIO_PIN_SET

#define RED(self, x)													\
	HAL_GPIO_WritePin((self)->lightPort, (self)->redPin, (x));			\
	HAL_GPIO_WritePin((self)->lightPort, (self)->greenPin,				\
						((x) == ON ? OFF : ON));						\
	HAL_GPIO_WritePin((self)->lightPort, (self)->yellowPin, 			\
						((x) == ON ? OFF : ON));						\

#define GREEN(self, x)													\
	HAL_GPIO_WritePin((self)->lightPort, (self)->redPin,				\
						((x) == ON ? OFF : ON));						\
	HAL_GPIO_WritePin((self)->lightPort, (self)->greenPin, (x));		\
	HAL_GPIO_WritePin((self)->lightPort, (self)->yellowPin,				\
						((x) == ON ? OFF : ON));						\

#define YELLOW(self, x)													\
	HAL_GPIO_WritePin((self)->lightPort, (self)->redPin,				\
						((x) == ON ? OFF : ON));						\
	HAL_GPIO_WritePin((self)->lightPort, (self)->greenPin,				\
						((x) == ON ? OFF : ON));						\
	HAL_GPIO_WritePin((self)->lightPort, (self)->yellowPin, (x));		\


#define BUTTON_NS	(1U << 0)
#define BUTTON_WE	(1U << 1)


typedef struct TrafficLight {

	GPIO_TypeDef* lightPort;

	uint16_t redPin;
	uint16_t yellowPin;
	uint16_t greenPin;

	GPIO_TypeDef* buttonPort;
	uint16_t buttonPin;

	volatile void (*currentState)(struct TrafficLight* self);
	volatile void (*buttonState)(struct TrafficLight* current, struct TrafficLight* other);

	uint32_t state_start_time;

	uint32_t red_duration;
	uint32_t yellow_duration;
	uint32_t green_duration;

	osTimerId_t stateTimerHandle;
} TrafficLight_t;

extern TrafficLight_t NS;
extern TrafficLight_t WE;

typedef void (*StateFunc)(TrafficLight_t* self);
typedef void (*ButtonFunc)(TrafficLight_t* current, TrafficLight_t* other);

void Init_NS(void);
void Init_WE(void);

void RED_STATE(TrafficLight_t* self);
void GREEN_STATE(TrafficLight_t* self);
void YELLOW_STATE(TrafficLight_t* self);

void RED_ON(TrafficLight_t* self);
void YELLOW_ON(TrafficLight_t* self);
void GRREN_ON(TrafficLight_t* self);

void Pedestrian(TrafficLight_t* current, TrafficLight_t* other);

uint32_t getElapsed(TrafficLight_t* self);
void getStateInfo(TrafficLight_t* self, char** stateName, uint32_t* remainTime);



#endif /* INC_TRAFFIC_SYSTEM_H_ */
