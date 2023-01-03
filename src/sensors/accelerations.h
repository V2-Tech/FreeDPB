#ifndef INC_ACCELERATIONS_H
#define INC_ACCELERATIONS_H

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

#ifdef USE_BMX055
#include "drivers/BMX055.h"
#include "drivers/BMX055_defs.h"
#endif

/*********************
 *      DEFINES
 *********************/
#ifdef USE_BMX055
#define MAX_DATA_FRAME_COUNT BMX_FIFO_DATA_FRAME_COUNT
#endif

#define MAX_COMMAND_COUNT 10

/****************************
 *  FUNCTIONS DECLARATIONS  *
 ****************************/
uint8_t acceleration_init(QueueHandle_t xQueueAcc2GUI_handle, QueueHandle_t xQueueCommandTo_handle, QueueHandle_t xQueueCommandFrom_handle);
uint8_t acceleration_update(void);
uint8_t acceleration_read_data(void);
uint8_t acceleration_send2gui(void);
uint8_t acceleration_FIFOFlush(void);
uint8_t acceleration_start_read(void);
uint8_t acceleration_stop_read(void);
#endif