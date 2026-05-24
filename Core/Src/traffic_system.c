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
/* USER CODE END Includes */


void RED_ON(TrafficLight_t* self)
{
	HAL_GPIO_WritePin(self->lightPort, self->redPin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(self->lightPort, self->greenPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->lightPort, self->yellowPin, GPIO_PIN_RESET);
}

void GRREN_ON(TrafficLight_t* self)
{
	HAL_GPIO_WritePin(self->lightPort, self->redPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->lightPort, self->greenPin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(self->lightPort, self->yellowPin, GPIO_PIN_RESET);
}

void YELLOW_ON(TrafficLight_t* self)
{
	HAL_GPIO_WritePin(self->lightPort, self->redPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->lightPort, self->greenPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->lightPort, self->yellowPin, GPIO_PIN_SET);
}




// Get the passed time of the light of a state
uint32_t getElapsed(TrafficLight_t* self) {
	return osKernelGetTickCount() - self->state_start_time;
}


void RED_STATE(TrafficLight_t* self)
{
	RED_ON(self);

	if(getElapsed(self) >= self->red_duration)
	{
		self->currentState = GREEN_STATE;
		self->state_start_time = osKernelGetTickCount();

		self->red_duration = 10000;
	}
}

void GREEN_STATE(TrafficLight_t* self) {
	GRREN_ON(self);

	if(getElapsed(self) >= self->green_duration) {
		self->currentState = YELLOW_STATE;
		self->state_start_time = osKernelGetTickCount();
		self->green_duration = 8000;		// reset the duration of green
	}
}

void YELLOW_STATE(TrafficLight_t* self) {
	YELLOW_ON(self);

	if(getElapsed(self) >= self->yellow_duration) {
		self->currentState = RED_STATE;
		self->state_start_time = osKernelGetTickCount();
		self->yellow_duration = 2000;		// reset the duration of yellow
	}
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
	uint32_t elapsed_time = getElapsed(self);

	if(self->currentState == RED_STATE)
	{
		*remainTime = self->red_duration - elapsed_time;
		*stateName = "RED";
	}
	else if(self->currentState == GREEN_STATE)
	{
		*remainTime = self->green_duration - elapsed_time;
		*stateName = "GREEN";
	}
	else if(self->currentState == YELLOW_STATE)
	{
		*remainTime = self->yellow_duration - elapsed_time;
		*stateName = "YELLOW";
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
