#ifndef INC_MAIN_H
#define INC_MAIN_H

#include "../src/tasks/tasks.h"

/*********************
 *      DEFINES
 *********************/

/*********************
 *      VARIABLES
 *********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
extern "C" {
    void app_main(void);
}

extern void guiTask(void *pvParameter);
extern void accelTask(void *pvParameter);
extern void filterTask(void *pvParameter);
extern void motorTask(void *pvParameter);

#endif