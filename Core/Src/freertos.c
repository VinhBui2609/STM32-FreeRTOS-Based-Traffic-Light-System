/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "traffic_system.h"
#include "logger.h"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for LightTask */
osThreadId_t LightTaskHandle;
const osThreadAttr_t LightTask_attributes = {
  .name = "LightTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for PedestrianTask */
osThreadId_t PedestrianTaskHandle;
const osThreadAttr_t PedestrianTask_attributes = {
  .name = "PedestrianTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for LoggerTask */
osThreadId_t LoggerTaskHandle;
const osThreadAttr_t LoggerTask_attributes = {
  .name = "LoggerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for buttonEvent */
osEventFlagsId_t buttonEventHandle;
const osEventFlagsAttr_t buttonEvent_attributes = {
  .name = "buttonEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
TrafficLight_t NS, WE;	// NS: North-South
						// WE: West-East

/* USER CODE END FunctionPrototypes */

void StartLightTask(void *argument);
void StartPedestrianTask(void *argument);
void StartLoggerTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  	TL_Init();

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of LightTask */
  LightTaskHandle = osThreadNew(StartLightTask, NULL, &LightTask_attributes);

  /* creation of PedestrianTask */
  PedestrianTaskHandle = osThreadNew(StartPedestrianTask, NULL, &PedestrianTask_attributes);

  /* creation of LoggerTask */
  LoggerTaskHandle = osThreadNew(StartLoggerTask, NULL, &LoggerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of buttonEvent */
  buttonEventHandle = osEventFlagsNew(&buttonEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartLightTask */
/**
  * @brief  Function implementing the LightTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartLightTask */
void StartLightTask(void *argument)
{
  /* USER CODE BEGIN StartLightTask */

    TrafficState_t NS_lastState = -1;
    TrafficState_t WE_lastState = -1;

  /* Infinite loop */
  for(;;)
  {
	/* NS state changed */
	if(NS.currentState != NS_lastState)
	{
		NS_lastState = NS.currentState;
	}

	/* WE state changed */
	if(WE.currentState != WE_lastState)
	{
		WE_lastState = WE.currentState;
	}

	osDelay(10);
  }
  /* USER CODE END StartLightTask */
}

/* USER CODE BEGIN Header_StartPedestrianTask */
/**
* @brief Function implementing the PedestrianTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPedestrianTask */
void StartPedestrianTask(void *argument)
{
  /* USER CODE BEGIN StartPedestrianTask */
	char msg[100];
	uint32_t flags;

  /* Infinite loop */
  for(;;)
  {
	  flags = osEventFlagsWait(buttonEventHandle, BUTTON_NS | BUTTON_WE, osFlagsWaitAny, osWaitForever);

	  if(flags & BUTTON_NS)
	  {
  		  sprintf(msg, "[BUTTON] North-South pedestrian request\r\n");
  		  LOG_Message(msg);

  		  NS.fpt_buttonState(&NS, &WE);
	  }

	  if(flags & BUTTON_WE)
	  {
  		  sprintf(msg, "[BUTTON] West-East pedestrian request\r\n");
  		  LOG_Message(msg);

  		  WE.fpt_buttonState(&WE, &NS);
	  }

  }
  /* USER CODE END StartPedestrianTask */
}

/* USER CODE BEGIN Header_StartLoggerTask */
/**
* @brief Function implementing the LoggerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoggerTask */
void StartLoggerTask(void *argument)
{
  /* USER CODE BEGIN StartLoggerTask */
	  char buffer[100];

  /* Infinite loop */
	  for(;;)
	  {
		  getStateInfo(&NS);
		  getStateInfo(&WE);

		  sprintf(buffer, "NS: %s %lus | WE: %s %lus\r\n", NS.state, (NS.remainTime + 999) / 1000,
				  	  	  	  	  	  	  	  	  	  	   WE.state, (WE.remainTime + 999) / 1000);
		  LOG_Message(buffer);

		  osDelay(1000);
	  }

  /* USER CODE END StartLoggerTask */
}


/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* stateTimerCallback function */
void stateTimerCallback(void *argument)
{
  /* USER CODE BEGIN stateTimerCallback */
	TrafficLight_t* tlHandler = (TrafficLight_t*)argument;

    switch(tlHandler->currentState)
    {
        case RED:
        	tlHandler->red_duration = DEF_RED;
            CHANGE_STATE(tlHandler, GREEN);
            break;

        case GREEN:
        	tlHandler->green_duration = DEF_GREEN;
            CHANGE_STATE(tlHandler, YELLOW);
            break;

        case YELLOW:
        	tlHandler->yellow_duration = DEF_YELLOW;
            CHANGE_STATE(tlHandler, RED);
            break;
    }

  /* USER CODE END stateTimerCallback */
}

/* USER CODE END Application */

