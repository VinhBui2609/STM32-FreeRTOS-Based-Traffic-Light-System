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
#ifndef INC_TL_TASKS_H_
#define INC_TL_TASKS_H_

/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

extern osThreadId_t LightTaskHandle;
extern const osThreadAttr_t LightTask_attributes;

extern osThreadId_t PedestrianTaskHandle;
extern const osThreadAttr_t PedestrianTask_attributes;

extern osThreadId_t LoggerTaskHandle;
extern const osThreadAttr_t LoggerTask_attributes;

extern osEventFlagsId_t buttonEventHandle;
extern const osEventFlagsAttr_t buttonEvent_attributes;

void TL_Task_Create();
void TL_Event_Create();

void StartLightTask(void *argument);
void StartPedestrianTask(void *argument);
void StartLoggerTask(void *argument);

#endif /* INC_TL_TASKS_H_ */
