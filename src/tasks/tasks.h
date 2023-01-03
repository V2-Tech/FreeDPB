#ifndef INC_TASKS_H
#define INC_TASKS_H

#include "../../main/common_def.h"

#include "../GUI/GUI.h"
#include "../sensors/accelerations.h"
#include "../motor/motor.h"
#include "../app/application.h"

/*********************
 *      CLASSES
 *********************/


/****************************/
/*  FUNCTION DEFINITIONS    */
/****************************/
void guiTask(void *pvParameter);
void lv_tick_task(void *arg);

void accelTask(void *pvParameter);
void senseTask(void *pvParameter);

void application(void *pvParameter);
#endif