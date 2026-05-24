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
} TrafficLight;

extern TrafficLight NS;
extern TrafficLight WE;

typedef void (*StateFunc)(TrafficLight* self);
typedef void (*ButtonFunc)(TrafficLight* current, TrafficLight* other);

void Init_NS(void);
void Init_WE(void);

void RED_STATE(TrafficLight* self);
void GREEN_STATE(TrafficLight* self);
void YELLOW_STATE(TrafficLight* self);


void Pedestrian(TrafficLight_t* current, TrafficLight_t* other);

uint32_t getElapsed(TrafficLight* self);
void getStateInfo(TrafficLight* self, char** stateName, uint32_t* remainTime);



#endif /* INC_TRAFFIC_SYSTEM_H_ */
