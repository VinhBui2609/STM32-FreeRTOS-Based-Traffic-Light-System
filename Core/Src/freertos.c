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
/* Definitions for stateTimer */
osTimerId_t stateTimerHandle;
const osTimerAttr_t stateTimer_attributes = {
  .name = "stateTimer"
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

void StartPedestrianTask(void *argument);
void StartLoggerTask(void *argument);
void stateTimerCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

	Init_NS();
	Init_WE();

	NS.stateTimerHandle = osTimerNew(stateTimerCallback, osTimerOnce, &NS, &stateTimer_attributes);
  	WE.stateTimerHandle = osTimerNew(stateTimerCallback, osTimerOnce, &WE, &stateTimer_attributes);

	NS.currentState(&NS);
	WE.currentState(&WE);

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of stateTimer */
//  stateTimerHandle = osTimerNew(stateTimerCallback, osTimerOnce, NULL, &stateTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
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

  		  NS.buttonState(&NS, &WE);
	  }

	  if(flags & BUTTON_WE)
	  {
  		  sprintf(msg, "[BUTTON] West-East pedestrian request\r\n");
  		  LOG_Message(msg);

  		  WE.buttonState(&WE, &NS);
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
	  char* NS_state;
	  char* WE_state;

	  uint32_t NS_remain;
	  uint32_t WE_remain;

	  char buffer[100];

  /* Infinite loop */
	  for(;;)
	  {
		  getStateInfo(&NS, &NS_state, &NS_remain);
		  getStateInfo(&WE, &WE_state, &WE_remain);

		  sprintf(buffer, "NS: %s %lus | WE: %s %lus\r\n", NS_state, (NS_remain + 999) / 1000,
				  	  	  	  	  	  	  	  	  	  	   WE_state, (WE_remain + 999) / 1000);
		  LOG_Message(buffer);

		  osDelay(1000);
	  }

  /* USER CODE END StartLoggerTask */
}

/* stateTimerCallback function */
void stateTimerCallback(void *argument)
{
  /* USER CODE BEGIN stateTimerCallback */
	TrafficLight_t* self = (TrafficLight_t*)argument;

	if(self->currentState == RED_STATE)			// RED --> GREEN
	{
		self->currentState = GREEN_STATE;
		self->red_duration = 10000;
	}
	else if(self->currentState == GREEN_STATE)	// GREEN --> YELLOW
	{
		self->currentState = YELLOW_STATE;
		self->green_duration = 8000;
	}
	else if(self->currentState == YELLOW_STATE)	// YELLOW --> RED
	{
		self->currentState = RED_STATE;
		self->yellow_duration = 2000;
	}

	self->currentState(self);
  /* USER CODE END stateTimerCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

