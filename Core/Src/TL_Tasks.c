/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    TL_Tasks.c
  * @brief   Code for Definition for controlling the Tasks for TL_System
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <TL_Tasks.h>
#include <TL_system.h>
#include <stdio.h>
#include <logger.h>
/* USER CODE END Includes */

/* Definition of Tasks ------------------------------------------------------------*/
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


/* Definition of EventFlags ------------------------------------------------------------*/
/* Definitions for buttonEvent */
osEventFlagsId_t buttonEventHandle;
const osEventFlagsAttr_t buttonEvent_attributes = {
  .name = "buttonEvent"
};


extern TrafficLight_t NS, WE;

void StartLightTask(void *argument)
{
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
}


void StartPedestrianTask(void *argument)
{
	char msg[100];
	uint32_t flags;

  /* Infinite loop */
  for(;;)
  {
	  flags = osEventFlagsWait(buttonEventHandle, PEDESTRIAN_NS | PEDESTRIAN_WE |
			  	  	  	  	  	  	  	  	  	  EMERGENCY_NS | EMERGENCY_WE,
												  osFlagsWaitAny, osWaitForever);

	  if(flags & PEDESTRIAN_NS)
	  {
  		  sprintf(msg, "[BUTTON] North-South pedestrian request\r\n");
  		  LOG_Message(msg);

  		  NS.fpt_buttonState = Pedestrian;
  		  NS.fpt_buttonState(&NS, &WE);
	  }

	  if(flags & PEDESTRIAN_WE)
	  {
  		  sprintf(msg, "[BUTTON] West-East pedestrian request\r\n");
  		  LOG_Message(msg);

  		  WE.fpt_buttonState = Pedestrian;
  		  WE.fpt_buttonState(&WE, &NS);
	  }

	  if(flags & EMERGENCY_NS)
	  {
  		  sprintf(msg, "[BUTTON] North-South emergency request\r\n");
  		  LOG_Message(msg);

  		  NS.fpt_buttonState = Emergency;
  		  NS.fpt_buttonState(&NS, &WE);
	  }

	  if(flags & EMERGENCY_WE)
	  {
  		  sprintf(msg, "[BUTTON] West-East emergency request\r\n");
  		  LOG_Message(msg);

  		  WE.fpt_buttonState = Emergency;
  		  WE.fpt_buttonState(&WE, &NS);
	  }

  }
}


void StartLoggerTask(void *argument)
{
	  char buffer[100];

/* Infinite loop */
	  for(;;)
	  {
		  getStateInfo(&NS);
		  getStateInfo(&WE);

		  sprintf(buffer, "NS: %s %lus | WE: %s %lus\r\n", NS.stateName, (NS.remainTime + 999) / 1000,
				  	  	  	  	  	  	  	  	  	  	   WE.stateName, (WE.remainTime + 999) / 1000);
		  LOG_Message(buffer);

		  osDelay(1000);
	  }
}



void TL_Task_Create()
{
	/* Create the thread(s) */
	/* creation of LightTask */
	LightTaskHandle = osThreadNew(StartLightTask, NULL, &LightTask_attributes);

	/* creation of PedestrianTask */
	PedestrianTaskHandle = osThreadNew(StartPedestrianTask, NULL, &PedestrianTask_attributes);

	/* creation of LoggerTask */
	LoggerTaskHandle = osThreadNew(StartLoggerTask, NULL, &LoggerTask_attributes);

}


void TL_Event_Create()
{
	/* Create the event(s) */
	/* creation of buttonEvent */
	buttonEventHandle = osEventFlagsNew(&buttonEvent_attributes);
}


/* stateTimerCallback function */
void stateTimerCallback(void *argument)
{
  /* USER CODE BEGIN stateTimerCallback */
	TrafficLight_t* tlHandle = (TrafficLight_t*)argument;

    switch(tlHandle->currentState)
    {
        case RED:
        	tlHandle->red_duration = DEF_RED;
            CHANGE_STATE(tlHandle, GREEN);
            break;

        case GREEN:
        	tlHandle->green_duration = DEF_GREEN;
            CHANGE_STATE(tlHandle, YELLOW);
            break;

        case YELLOW:
        	tlHandle->yellow_duration = DEF_YELLOW;
            CHANGE_STATE(tlHandle, RED);
            break;
    }

  /* USER CODE END stateTimerCallback */
}

