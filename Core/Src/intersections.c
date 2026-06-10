/*
 * intersections.c
 *
 *  Created on: 03.06.2026
 *      Author: Thien Bui
 */
/* -------------------------------------- INCLUDES -------------------------------------- */
#include "intersections.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* ------------------------------------- VARIABLES -------------------------------------- */
static const IntersectionArgs_t g_intersection_arg_list[MAX_NODES] = {
    { "ABC-x-CDE", 2U, {RED_TIME,GREEN_TIME,YELLOW_TIME} }
};
static const char 		*state_name[] = { "RED   ", "GREEN ", "YELLOW" };
static uint32_t 		default_duration[3] = {RED_TIME,GREEN_TIME,YELLOW_TIME};
static Intersection_t 	g_intersection_list[MAX_NODES];
static UartRx_t		 	g_uart;
uint8_t					rx_data[2];

/* -------------------------------- FUNCTION CALLBACKS ---------------------------------- */
/**
 * @brief  UART DMA receive‑complete callback for command input.
 *
 * This callback is triggered whenever two bytes have been received over UART2
 * via DMA. The received bytes are stored into the UART task context and the
 * pedestrian event flag is set to notify the UART processing task that a new
 * command is available. After handling the data, the DMA reception is restarted
 * so that the system continues listening for the next command without gaps.
 *
 * @param huart  Pointer to the UART handle that generated the callback.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)
	{
		g_uart.flag_data[0] = rx_data[0];
		g_uart.flag_data[1] = rx_data[1];
		osEventFlagsSet(g_uart.button_handle, PEDESTRIAN);

		// Restart the Interrupt reception
		HAL_UART_Receive_DMA(&huart2, rx_data, 2);
	}
}

/* ------------------------------ FUNCTION DECLARATIONS -------------------------------- */
/**
 * @brief  Initialize all system components for the traffic controller.
 *
 * This function sets up the entire traffic‑light system before the RTOS
 * scheduler starts. It initializes every intersection by creating their
 * threads, timers, and internal state, and then initializes the UART task
 * responsible for receiving pedestrian/emergency commands.
 */
void MyTrafficSystem (void) {
	for (uint8_t i = 0; i < MAX_NODES; i++) {
		InitIntersectionTask(&g_intersection_list[i], &g_intersection_arg_list[i]);
	}
	InitUartTask(&g_uart);
}

/**
 * @brief  Initialize a traffic‑light intersection and its RTOS resources.
 *
 * This function prepares a single intersection before the RTOS scheduler starts.
 * It allocates the traffic‑light array, assigns the initial states (first light
 * GREEN, all others RED), copies the default phase durations, creates a one‑shot
 * timer for each light, and records the initial start_time used for remaining‑
 * time calculations. Finally, it creates the Intersection task responsible for
 * running the state machine and handling timing updates.
 *
 * @param node  Pointer to the intersection instance to initialize.
 * @param args  Pointer to the static configuration describing this intersection.
 */
void InitIntersectionTask (Intersection_t *node, const IntersectionArgs_t *args) {
	node->name = args->name;
	node->num_of_lights = args->num_of_lights;
	node->light_list = malloc(sizeof(TrafficLight_t) * node->num_of_lights);

	const osThreadAttr_t node_attributes = {
	  .name = args->name,
	  .stack_size = 256 * 4,
	  .priority = (osPriority_t) osPriorityHigh,
	};
	const osTimerAttr_t timer_attributes = {
	  .name = "state_timer"
	};

	// Default: First light is initially GREEN
	node->light_list[0].state = GREEN;
	for (uint8_t i = 1; i < node->num_of_lights; i++) {
		node->light_list[i].state = RED;
	}
	// Initialize the remaining parameters of traffic lights
	for (uint8_t i = 0; i < node->num_of_lights; i++) {
		memcpy(node->light_list[i].duration, args->duration, sizeof(uint32_t) * 3);
		node->light_list[i].timer_handle = osTimerNew(	StartTimerElapsedCallback,
														osTimerOnce,
														&node->light_list[i],
														&timer_attributes);
		node->light_list[i].start_time = osKernelGetTickCount();
	}
	node->task_handle = osThreadNew(StartIntersectionTask, node, &node_attributes);
}

/**
 * @brief  Initialize the UART reception task and its RTOS event resources.
 *
 * This function creates the event‑flag object used to signal incoming UART
 * commands (e.g., pedestrian or emergency requests), then creates the UART
 * receive thread responsible for waiting on these events and dispatching them
 * to the traffic‑light logic. Finally, it starts the UART DMA reception so that
 * incoming bytes can trigger the HAL UART Rx Complete callback.
 *
 * @param uart  Pointer to the UART task context structure.
 */
void InitUartTask (UartRx_t *uart) {
	const osEventFlagsAttr_t button_attributes = {
	  .name = "Button Event"
	};
	uart->button_handle = osEventFlagsNew(&button_attributes);

	const osThreadAttr_t uart_attributes = {
	  .name = "Uart Event",
	  .stack_size = 256 * 4,
	  .priority = (osPriority_t) osPriorityHigh,
	};
	uart->task_handle = osThreadNew(StartUartReceiveTask, uart, &uart_attributes);

	HAL_UART_Receive_DMA(&huart2, rx_data, 2);
}

/**
 * @brief  RTOS task that manages a single intersection's traffic‑light state.
 *
 * This task starts all per‑light timers using their current phase durations,
 * then enters a 1‑second periodic loop. On each iteration it computes the remaining
 * time of every traffic light based on start_time and the active duration of the
 * current state, formats a status message, and transmits it over UART.
 * The actual state transitions are driven by each light’s timer callback
 * while this task is responsible only for periodic monitoring and reporting.
 *
 * @param arguments  Pointer to the Intersection_t instance associated with this task.
 */
void StartIntersectionTask (void *arguments) {
	Intersection_t *node = (Intersection_t *)arguments;
	for (uint8_t i = 0; i < node->num_of_lights; i++) {
		TrafficLight_t *light = &node->light_list[i];
		osTimerStart(light->timer_handle, light->duration[light->state]);
	}
	while (1) {
		char buffer[100];
		size_t msg_len = snprintf(buffer, sizeof(buffer), "** Node: %s **\n", node->name);
		for (uint8_t i = 0; i < node->num_of_lights; i++) {
			TrafficLight_t *light = &node->light_list[i];
			uint32_t elapsed = osKernelGetTickCount() - light->start_time;
			uint32_t remain = (light->duration[light->state] - elapsed + 999)/1000;
			msg_len += snprintf(msg_len + buffer, sizeof(buffer) - msg_len,
								" |--> Light-%d: %s %ld(s)\n", i,
								state_name[light->state], remain);
		}
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
//		HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buffer, strlen(buffer));
		osDelay(1000);
	}
}

/**
 * @brief  RTOS task that waits for UART‑driven button events and dispatches them.
 *
 * This task blocks on an event‑flag object that is set by the UART Rx Complete
 * callback whenever a pedestrian or emergency command is received. When a flag
 * becomes active, the task decodes the received bytes (node index and light
 * index) and invokes the appropriate request handler.
 *
 * @param arguments  Pointer to the UartRx_t context associated with this task.
 */
void StartUartReceiveTask (void *arguments) {
	UartRx_t *uart = (UartRx_t *)arguments;
	uint32_t flag;
	while (1) {
		flag = osEventFlagsWait(uart->button_handle,
								PEDESTRIAN | EMERGENCY,
								osFlagsWaitAny | osFlagsNoClear,
								osWaitForever);

		if 		(flag & PEDESTRIAN) {
			uint8_t node_idx = uart->flag_data[0];
			uint8_t light_idx = uart->flag_data[1];
			OnPedestrianRequest (node_idx, light_idx);
			osEventFlagsClear(uart->button_handle, PEDESTRIAN);
		}
		else if (flag & EMERGENCY) {
			// TO DO
		} else {
			// TO DO
		}
	}
}

/**
 * @brief  Handle a pedestrian request for a specific traffic light.
 *
 * This function is called when a pedestrian button event is received via UART.
 * It logs the request, checks whether the targeted light is currently GREEN,
 * and if the remaining GREEN time is greater than 2 seconds, it shortens the
 * GREEN phase to 2 seconds. All other lights in the same intersection have
 * their current phase extended to maintain proper sequencing.
 *
 * The function updates:
 *  - the active duration of each affected light
 *  - the start_time used for remaining-time calculations
 *  - the running timers (stop + restart with new duration)
 *
 * If the light is not GREEN or the remaining time is already ≤ 2 seconds,
 * the request is ignored.
 *
 * @param node_idx   Index of the intersection in the global list.
 * @param light_idx  Index of the traffic light within the intersection.
 */
void OnPedestrianRequest (uint8_t node_idx, uint8_t light_idx) {
	Intersection_t *node = &g_intersection_list[node_idx];
	TrafficLight_t *light = &node->light_list[light_idx];
	char buffer[100];

	size_t msg_len = snprintf(buffer, sizeof(buffer),
					"** Node: %s **\n", node->name);
	msg_len += snprintf(msg_len + buffer, sizeof(buffer) - msg_len,
					" |--> Light-%d requests Pedestrian \n", light_idx);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	if (light->state != GREEN) {
		return;		// Nothing to do
	}
	uint32_t elapsed = osKernelGetTickCount() - light->start_time;
	uint32_t remain = light->duration[light->state] - elapsed;

	if (remain > 2000) {
		for (uint8_t i = 0; i < node->num_of_lights; i++) {
			TrafficLight_t *light = &node->light_list[i];
			if (i == light_idx) {
				osTimerStop(light->timer_handle);
				light->duration[GREEN] = 2000;
				light->start_time = osKernelGetTickCount();
				osTimerStart(light->timer_handle, light->duration[GREEN]);
			} else {
				osTimerStop(light->timer_handle);
				// ONLY works for 2 lights
				light->duration[RED] = 2000 + 2000;
				light->start_time = osKernelGetTickCount();
				osTimerStart(light->timer_handle, light->duration[RED]);
			}
		}
	}
}

/**
 * @brief  Timer callback that advances a traffic light to its next state.
 *
 * This callback is invoked when a light’s one‑shot timer expires. It advances
 * the traffic light to the next state in the sequence (RED → GREEN → YELLOW → RED),
 * restores the default duration for the new state, updates the start_time used
 * for remaining‑time calculations, and restarts the timer with the refreshed
 * duration. This ensures that each phase runs for its programmed length unless
 * modified externally (e.g., by a pedestrian request).
 *
 * @param arguments  Pointer to the TrafficLight_t instance whose timer expired.
 */
void StartTimerElapsedCallback (void *arguments) {
	TrafficLight_t *light = (TrafficLight_t *)arguments;
	light->state = (light->state + 1) % 3;
	light->duration[light->state] = default_duration[light->state];
	light->start_time = osKernelGetTickCount();
	osTimerStart(light->timer_handle, light->duration[light->state]);
}

