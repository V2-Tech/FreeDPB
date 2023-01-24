#ifndef INC_MAIN_H
#define INC_MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "./tasks/tasks.h"

/*********************
 *      DEFINES
 *********************/

/*********************
 *      VARIABLES
 *********************/
TaskHandle_t guiTaskHandle;
TaskHandle_t accelTaskHandle;
TaskHandle_t senseTaskHandle;
TaskHandle_t appTaskHandle;

/**********************
 *  STATIC PROTOTYPES
 **********************/
extern "C" {
    void app_main(void);
}

extern void guiTask(void *pvParameter);
extern void accelTask(void *pvParameter);
extern void senseTask(void *pvParameter);
extern void application(void *pvParameter);

#endif