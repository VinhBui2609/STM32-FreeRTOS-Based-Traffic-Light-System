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
#include "traffic_system.h"
#include "main.h"
#include "usart.h"
/* USER CODE END Includes */


/* USER CODE BEGIN PV */
extern uint8_t rxData;
extern osEventFlagsId_t buttonEventHandle;
extern osTimerId_t stateTimerHandle;
/* USER CODE END PV */


// For Interrupt from Serial Terminal (Likely hardware Button)
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



// Get the passed time of the light of a state
uint32_t getElapsed(TrafficLight_t* self)
{
	return osKernelGetTickCount() - self->state_start_time;
}


void RED_STATE(TrafficLight_t* self)
{
	RED(self, ON);
	self->state_start_time = osKernelGetTickCount();
	osTimerStart(self->stateTimerHandle, self->red_duration);
}

void GREEN_STATE(TrafficLight_t* self)
{
	GREEN(self, ON);
	self->state_start_time = osKernelGetTickCount();
	osTimerStart(self->stateTimerHandle, self->green_duration);
}

void YELLOW_STATE(TrafficLight_t* self)
{
	YELLOW(self, ON);
	self->state_start_time = osKernelGetTickCount();
	osTimerStart(self->stateTimerHandle, self->yellow_duration);
}

void Pedestrian(TrafficLight_t* current, TrafficLight_t* other) {
	uint32_t elapsed_green = getElapsed(current);
	uint32_t remain_green = current->green_duration - getElapsed(current);

	/*
	 * Elapsed: 2s
	 * Remain: 6s
	 * --> change green_duration to 4s
	 * --> only need 2s more to getElapsed(self) >= green_duration = 4s
	 */


	if(remain_green > 2000)
	{
		current->green_duration = 2000 + elapsed_green;

		uint32_t elapsed_red = getElapsed(other);
		other->red_duration = 4000 + elapsed_red;
	}

	// if green is less than 2s, do nothing

}

// Input: Light (NS, WE...)
// Output: State name + remaining time of that state
// stateName is ** since its value is a string (an array of characters)
void getStateInfo(TrafficLight_t* self, char** stateName, uint32_t* remainTime) {
	uint32_t elapsed = getElapsed(self);
	uint32_t duration;

	if(self->currentState == RED_STATE)
	{
		duration = self->red_duration;
		*stateName = "RED";
	}
	else if(self->currentState == GREEN_STATE)
	{
		duration = self->green_duration;
		*stateName = "GREEN";
	}
	else if(self->currentState == YELLOW_STATE)
	{
		duration = self->yellow_duration;
		*stateName = "YELLOW";
	}

	if(elapsed >= duration)
	{
		*remainTime = 0;
	}
	else
	{
		*remainTime =
				duration - elapsed;
	}
}



// Initialization of Lights
extern TrafficLight_t NS, WE;

void Init_NS() {
	NS.lightPort = NS_GPIO_Port;
	NS.redPin = NS_RED_Pin;
	NS.greenPin = NS_GREEN_Pin;
	NS.yellowPin = NS_YELLOW_Pin;

	NS.currentState = GREEN_STATE;
	NS.buttonState = Pedestrian;

	NS.state_start_time = osKernelGetTickCount();
	NS.red_duration = 10000;
	NS.green_duration = 8000;
	NS.yellow_duration = 2000;
}


void Init_WE() {
	WE.lightPort = WE_GPIO_Port;
	WE.redPin = WE_RED_Pin;
	WE.greenPin = WE_GREEN_Pin;
	WE.yellowPin = WE_YELLOW_Pin;

	WE.currentState = RED_STATE;
	WE.buttonState = Pedestrian;

	WE.state_start_time = osKernelGetTickCount();
	WE.red_duration = 10000;
	WE.green_duration = 8000;
	WE.yellow_duration = 2000;
}
