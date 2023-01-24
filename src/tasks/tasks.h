#ifndef INC_TASKS_H
#define INC_TASKS_H

#include "../shared/common_def.h"
#include "../shared/shared_data.h"

#include "../sensors/accelerations.h"
#include "../motor/motor.h"
#include "../app/application.h"
#include "../GUI/GUI.h"

//************************/
//*      DEFINES         */
//************************/
#define LV_TICK_PERIOD_MS 1

#define QUEUE_ACC2GUI_LENGTH    MAX_DATA_FRAME_COUNT
#define ITEM_ACC2GUI_SIZE       sizeof( acc_sensor_data )

#define QUEUE_COMMANDS_LENGTH   MAX_COMMAND_COUNT
#define ITEM_COMMANDS_SIZE      sizeof( command_data_t )

//*******************************/
//*     FUNCTIONS DECLARATION   */
//*******************************/
void guiTask(void *pvParameter);
void lv_tick_task(void *arg);

void accelTask(void *pvParameter);
void senseTask(void *pvParameter);

void application(void *pvParameter);


#endif