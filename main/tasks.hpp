#ifndef INC_TASKS_H
#define INC_TASKS_H

#include "common_def.h"
#include "DPBshared.hpp"

#include "FreeDPB/DPB.hpp"
#include "GUI/GUI.h"
#include "accelerometer.hpp"
#include "RPMTracker.hpp"
#include "motor.hpp"

//************************/
//*      DEFINES         */
//************************/
#define LV_TICK_PERIOD_MS 1

#define QUEUE_COMMANDS_LENGTH   10
#define ITEM_COMMANDS_SIZE      sizeof( command_data_t )

//*******************************/
//*     FUNCTIONS DECLARATION   */
//*******************************/
void guiTask(void *pvParameter);
void lv_tick_task(void *arg);

void accelTask(void *pvParameter);
void sensorTask(void *pvParameter);

void application(void *pvParameter);


#endif