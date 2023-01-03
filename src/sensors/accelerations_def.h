#ifndef INC_ACCELERATIONS_DEF_H
#define INC_ACCELERATIONS_DEF_H

#include "../../main/common_def.h"

#ifdef USE_BMX055
#include "drivers/BMX055.h"
#include "drivers/BMX055_defs.h"
#endif

/*********************
 *      DEFINES
 *********************/

/****************************
 *      VARIABLES
 ****************************/
#ifdef USE_BMX055
static BMX055 accel;
uint16_t acc_index;
#endif

static sensor_3D_data accel_data[BMX_FIFO_DATA_FRAME_COUNT] = {{0, 0, 0}};

static QueueHandle_t _xQueueData2GUI = NULL;
static QueueHandle_t _xQueueAcc2Sys = NULL;
static QueueHandle_t _xQueueSys2Acc = NULL;

#endif