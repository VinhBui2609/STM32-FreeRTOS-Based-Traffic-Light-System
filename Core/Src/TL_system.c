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
uint32_t getElapsed(TrafficLight_t* tlHandler)
{
	return osKernelGetTickCount() - tlHandler->state_start_time;
}


void CHANGE_STATE(TrafficLight_t* tlHandler, TrafficState_t state)
{
	uint32_t duration = 0;

	tlHandler->currentState = state;
	tlHandler->state_start_time = osKernelGetTickCount();

	switch(state)
	{
		case RED:
			duration = tlHandler->red_duration;
			break;

		case GREEN:
			duration = tlHandler->green_duration;
			break;

		case YELLOW:
			duration = tlHandler->yellow_duration;
			break;
	}

	osTimerStart(tlHandler->stateTimerHandle, duration);
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
	 * --> only need 2s more to getElapsed(tlHandler) >= green_duration = 4s
	 */


    if(remain_green > 2000)
    {
        uint32_t elapsed_red = getElapsed(other);

        current->green_duration = elapsed_green + PEDES_GREEN;
        other->red_duration = elapsed_red + PEDED_RED;

        /* Restart current timer */
        osTimerStop(current->stateTimerHandle);
        osTimerStart(current->stateTimerHandle, PEDES_GREEN);

        /* Restart other timer */
        osTimerStop(other->stateTimerHandle);
        osTimerStart(other->stateTimerHandle, PEDED_RED);
    }

	// if green is less than 2s, do nothing

}

// Input: Light (NS, WE...)
// Output: State name + remaining time of that state
// stateName is ** since its value is a string (an array of characters)
void getStateInfo(TrafficLight_t* tlHandler)
{
	uint32_t elapsed = getElapsed(tlHandler);
	uint32_t duration = 0;

    switch(tlHandler->currentState)
    {
        case RED:
            duration = tlHandler->red_duration;
            tlHandler->stateName = "RED";
            break;

        case GREEN:
            duration = tlHandler->green_duration;
            tlHandler->stateName = "GREEN";
            break;

        case YELLOW:
            duration = tlHandler->yellow_duration;
            tlHandler->stateName = "YELLOW";
            break;
    }

	if(elapsed >= duration)
	{
		tlHandler->remainTime = 0;
	}
	else
	{
		tlHandler->remainTime = duration - elapsed;
	}
}



/* Initialization of Lights */
TrafficLight_t NS, WE;

void Init_Light(TrafficLight_t* tlHandler)
{
	tlHandler->fpt_buttonState = Pedestrian;

	tlHandler->state_start_time = osKernelGetTickCount();
	tlHandler->red_duration = DEF_RED;
	tlHandler->green_duration = DEF_GREEN;
	tlHandler->yellow_duration = DEF_YELLOW;
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
		 	 case 'n':
		 		 osEventFlagsSet(buttonEventHandle, BUTTON_NS);
		 		 break;

		 	 case 'w':
		 		 osEventFlagsSet(buttonEventHandle, BUTTON_WE);
		 		 break;
		 }

		 // Restart the Interrupt reception
		 HAL_UART_Receive_DMA(&huart2, &rxData, 1);
	 }
}
