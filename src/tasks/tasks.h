#ifndef INC_TASKS_H
#define INC_TASKS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "driver/gpio.h"

#include "../../main/common_def.h"
#include "../GUI/GUI.h"
#include "../sensors/accelerations.h"
#include "../motor/motor.h"

/*********************
 *      DEFINES
 *********************/
#define LV_TICK_PERIOD_MS 1
#define GPIO_LED_ERROR GPIO_NUM_2

#define QUEUE_ACC2GUI_LENGTH    MAX_DATA_FRAME_COUNT
#define ITEM_ACC2GUI_SIZE       sizeof( acc_sensor_data )

#define QUEUE_COMMANDS_LENGTH   MAX_COMMAND_COUNT
#define ITEM_COMMANDS_SIZE      sizeof( command_data )

/*********************
 *      VARIABLES
 *********************/


/*********************
 *      CLASSES
 *********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
void guiTask(void *pvParameter);
void lv_tick_task(void *arg);

void accelTask(void *pvParameter);
void filterTask(void *pvParameter);

void motorTask(void *pvParameter);
#endif