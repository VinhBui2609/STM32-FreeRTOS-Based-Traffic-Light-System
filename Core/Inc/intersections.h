/*
 * intersections.h
 *
 *  Created on: 03.06.2026
 *      Author: Thien Bui
 */

#ifndef INC_INTERSECTIONS_H_
#define INC_INTERSECTIONS_H_

/* -------------------------------------- INCLUDES -------------------------------------- */
#include "user_constant.h"
#include "cmsis_os.h"

/* -------------------------------------- TYPEDEFS -------------------------------------- */
typedef enum {
    PEDESTRIAN = (1U << 0),
    EMERGENCY  = (1U << 1)
} ButtonState_t;

typedef enum {
	RED,
	GREEN,
	YELLOW
} TrafficState_t;

typedef struct {
	TrafficState_t	state;
	uint32_t		duration[3];
	osTimerId_t		timer_handle;
	uint32_t		start_time;
} TrafficLight_t;

typedef struct {
    const char		*name;
    uint8_t			num_of_lights;
    uint32_t		duration[3];
} IntersectionArgs_t;

typedef struct {
	const char		*name;
    osThreadId_t    task_handle;
    uint8_t			num_of_lights;
    TrafficLight_t	*light_list;
} Intersection_t;

typedef struct {
	osThreadId_t		task_handle;
	osEventFlagsId_t	button_handle;
	uint8_t				flag_data[2];
} UartRx_t;

/* -------------------------------- FUNCTION PROTOTYPES --------------------------------- */
void MyTrafficSystem (void);
void InitIntersectionTask (Intersection_t *node, const IntersectionArgs_t *args);
void InitUartTask (UartRx_t *uart);
void StartIntersectionTask (void *arguments);
void StartUartReceiveTask (void *arguments);
void OnPedestrianRequest (uint8_t node_idx, uint8_t light_idx);
void StartTimerElapsedCallback (void *arguments);

#endif /* INC_INTERSECTIONS_H_ */
