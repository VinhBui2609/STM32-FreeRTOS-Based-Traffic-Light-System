/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    traffic_system.c
  * @brief   Code for Definition for controlling the Light States
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
#include <TL_system.h>
#include "usart.h"
/* USER CODE END Includes */


/* USER CODE BEGIN PV */
extern uint8_t rxData;
extern osEventFlagsId_t buttonEventHandle;
extern osTimerId_t stateTimerHandle;
/* USER CODE END PV */


/* Definitions for Software stateTimer */
osTimerId_t stateTimerHandle;
const osTimerAttr_t stateTimer_attributes = {
  .name = "stateTimer"
};


/* Get the passed time of the light of a state */
uint32_t getElapsed(TrafficLight_t* tlHandle)
{
	return osKernelGetTickCount() - tlHandle->state_start_time;
}


void CHANGE_STATE(TrafficLight_t* tlHandle, TrafficState_t state)
{
	uint32_t duration = 0;

	tlHandle->currentState = state;
	tlHandle->state_start_time = osKernelGetTickCount();

	switch(state)
	{
		case RED:
			duration = tlHandle->red_duration;
			break;

		case GREEN:
			duration = tlHandle->green_duration;
			break;

		case YELLOW:
			duration = tlHandle->yellow_duration;
			break;
	}

	osTimerStart(tlHandle->stateTimerHandle, duration);
}


void ALL_RED(TrafficLight_t* tlHandle)
{
	tlHandle->red_duration = DEF_RED;
	CHANGE_STATE(tlHandle, RED);
}



void Pedestrian(TrafficLight_t* current, TrafficLight_t* other) {

    /* Only work during GREEN */
    if(current->currentState != GREEN)
    {
        return;
    }

    uint32_t elapsed_green = getElapsed(current);
    uint32_t remain_green = current->green_duration - elapsed_green;

	/*
	 * Elapsed: 2s
	 * Remain: 6s
	 * --> change green_duration to 4s
	 * --> only need 2s more to getElapsed(tlHandle) >= green_duration = 4s
	 */


    if(remain_green > 2000)
    {
        uint32_t elapsed_red = getElapsed(other);

        current->green_duration = elapsed_green + PEDES_GREEN;
        other->red_duration = elapsed_red + PEDES_RED;

        /* Restart current timer */
        osTimerStop(current->stateTimerHandle);
        osTimerStart(current->stateTimerHandle, PEDES_GREEN);

        /* Restart other timer */
        osTimerStop(other->stateTimerHandle);
        osTimerStart(other->stateTimerHandle, PEDES_RED);
    }

	// if green is less than 2s, do nothing

}


void Emergency(TrafficLight_t* current, TrafficLight_t* other)
{
	if(current->currentState == RED)
	{
		current->green_duration = DEF_GREEN;
		CHANGE_STATE(current, GREEN);

		ALL_RED(other);
	}
}


// Input: Light (NS, WE...)
// Output: State name + remaining time of that state
// stateName is ** since its value is a string (an array of characters)
void getStateInfo(TrafficLight_t* tlHandle)
{
	uint32_t elapsed = getElapsed(tlHandle);
	uint32_t duration = 0;

    switch(tlHandle->currentState)
    {
        case RED:
            duration = tlHandle->red_duration;
            tlHandle->stateName = "RED";
            break;

        case GREEN:
            duration = tlHandle->green_duration;
            tlHandle->stateName = "GREEN";
            break;

        case YELLOW:
            duration = tlHandle->yellow_duration;
            tlHandle->stateName = "YELLOW";
            break;
    }

	if(elapsed >= duration)
	{
		tlHandle->remainTime = 0;
	}
	else
	{
		tlHandle->remainTime = duration - elapsed;
	}
}



/* Initialization of Lights */
TrafficLight_t NS, WE;

void Init_Light(TrafficLight_t* tlHandle)
{
	tlHandle->fpt_buttonState = NULL;

	tlHandle->state_start_time = osKernelGetTickCount();
	tlHandle->red_duration = DEF_RED;
	tlHandle->green_duration = DEF_GREEN;
	tlHandle->yellow_duration = DEF_YELLOW;
}

void TL_Init()
{
	Init_Light(&NS);
	Init_Light(&WE);

	NS.stateTimerHandle = osTimerNew(stateTimerCallback, osTimerOnce, &NS, &stateTimer_attributes);
	WE.stateTimerHandle = osTimerNew(stateTimerCallback, osTimerOnce, &WE, &stateTimer_attributes);

    CHANGE_STATE(&NS, GREEN);
    CHANGE_STATE(&WE, RED);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	 if(huart->Instance == USART2)
	 {
		 switch(rxData)
		 {
		 	 // North-South Pedestrian button
		 	 case 'n':
		 		 osEventFlagsSet(buttonEventHandle, PEDESTRIAN_NS);
		 		 break;

			 // West-East Pedestrian button
		 	 case 'w':
		 		 osEventFlagsSet(buttonEventHandle, PEDESTRIAN_WE);
		 		 break;

		 	 // North-South Emergency button
		 	 case 'N':
		 		 osEventFlagsSet(buttonEventHandle, EMERGENCY_NS);
		 		 break;

		 	 // West-East Emergency button
		 	 case 'W':
		 		 osEventFlagsSet(buttonEventHandle, EMERGENCY_WE);
		 		 break;

		 }

		 // Restart the Interrupt reception
		 HAL_UART_Receive_DMA(&huart2, &rxData, 1);
	 }
}
