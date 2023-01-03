#ifndef INC_TASKS_DEF_H
#define INC_TASKS_DEF_H

#include "../../main/common_def.h"

#include "../GUI/GUI.h"
#include "../sensors/accelerations.h"
#include "../motor/motor.h"
#include "../app/application.h"

/*********************
 *      DEFINES
 *********************/
#define LV_TICK_PERIOD_MS 1

#define QUEUE_ACC2GUI_LENGTH    MAX_DATA_FRAME_COUNT
#define ITEM_ACC2GUI_SIZE       sizeof( acc_sensor_data )

#define QUEUE_COMMANDS_LENGTH   MAX_COMMAND_COUNT
#define ITEM_COMMANDS_SIZE      sizeof( command_data )

#define ACC_DATA_BUFFER_SIZE    4000

/************************************/
/*      VARIABLES DECLARATIONS      */
/************************************/
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

static StaticQueue_t xStaticQueueAcc2gui;
uint8_t ucQueueAcc2guiStorageArea[QUEUE_ACC2GUI_LENGTH * ITEM_ACC2GUI_SIZE];
QueueHandle_t xQueueAcc2guiHandle;

static StaticQueue_t xStaticQueueAcc2guiCommands;
uint8_t ucQueueAcc2guiCommandsStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueAcc2guiCommandsHandle;

static StaticQueue_t xStaticQueueGui2AccCommand;
uint8_t ucQueueGui2AccCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueGui2AccCommandsHandle;

static StaticQueue_t xStaticQueueComp2SysCommand;
uint8_t ucQueueComp2SysCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueComp2SysCommandsHandle;

static StaticQueue_t xStaticQueueSys2CompCommand;
uint8_t ucQueueSys2CompCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueSys2CompCommandsHandle;

static Motor esc(4, DSHOT300);

static FIFOBuffer<acc_sensor_data> accDataBuffer(ACC_DATA_BUFFER_SIZE);

/*********************
 *      CLASSES
 *********************/


/****************************/
/*  FUNCTION DEFINITIONS    */
/****************************/

#endif