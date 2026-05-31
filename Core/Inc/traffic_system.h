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
#define BUTTON_NS	(1U << 0)
#define BUTTON_WE	(1U << 1)

#define DEF_RED		10000
#define DEF_GREEN	8000
#define DEF_YELLOW	2000

#define PEDED_RED		4000
#define PEDES_GREEN		2000

typedef enum
{
	RED,
	GREEN,
	YELLOW

} TrafficState_t;


typedef struct TrafficLight {

	TrafficState_t currentState;
	volatile void (*fpt_buttonState)(struct TrafficLight* current, struct TrafficLight* other);

	uint32_t state_start_time;

	uint32_t red_duration;
	uint32_t yellow_duration;
	uint32_t green_duration;

	char* state;
	uint32_t remainTime;

	osTimerId_t stateTimerHandle;

} TrafficLight_t;



void TL_Init();
void Init_Light(TrafficLight_t* tlHandler);

void CHANGE_STATE(TrafficLight_t* tlHandler, TrafficState_t state);

void Pedestrian(TrafficLight_t* current, TrafficLight_t* other);

uint32_t getElapsed(TrafficLight_t* tlHandler);
void getStateInfo(TrafficLight_t* tlHandler);

void stateTimerCallback(void *argument);

#endif /* INC_TRAFFIC_SYSTEM_H_ */
