#ifndef INC_APP_H
#define INC_APP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "../../main/common_def.h"
#include "../GUI/GUI.h"
#include "../sensors/accelerations.h"
#include "../motor/motor.h"

/************************************/
/*      FUNCTION DECLARATIONS       */
/************************************/
uint8_t app_init(Motor *motor, gpio_num_t opto_gpio_num, QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, FIFOBuffer<acc_sensor_data> *pDataBuffer);
void app_loop(void);
void app_exe(command_data command);
void app_start(void);
void app_reset(void);
void app_rec_timer_start(void);

void vibeTimerCallback(TimerHandle_t pxTimer);
#endif