#ifndef INC_TASKS_DEF_H
#define INC_TASKS_DEF_H

#include "../shared/common_def.h"
#include "../shared/shared_data.h"

#include "../GUI/GUI.h"
#include "../app/application.h"

/*********************
 *      DEFINES
 *********************/
#define LV_TICK_PERIOD_MS 1

#define QUEUE_ACC2GUI_LENGTH    MAX_DATA_FRAME_COUNT
#define ITEM_ACC2GUI_SIZE       sizeof( acc_sensor_data )

#define QUEUE_COMMANDS_LENGTH   MAX_COMMAND_COUNT
#define ITEM_COMMANDS_SIZE      sizeof( command_data )

/************************************/
/*      VARIABLES DECLARATIONS      */
/************************************/
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

static StaticQueue_t xStaticQueueComp2SysCommand;
uint8_t ucQueueComp2SysCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueComp2SysCommandsHandle;

static StaticQueue_t xStaticQueueSys2CompCommand;
uint8_t ucQueueSys2CompCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueSys2CompCommandsHandle;

#ifdef USE_BMX055
static BMX055 accel;
#endif

static DPB sys(GPIO_ESC_OUT, DSHOT300, GPIO_OPT_SENSOR, &accel);

static fft_chart_data FFTOuput[2];

/*********************
 *      CLASSES
 *********************/


/****************************/
/*  FUNCTION DEFINITIONS    */
/****************************/

#endif