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
inline const char *TAG = "ACCELERATIONS";

static sensor_3D_data accel_data[BMX_FIFO_DATA_FRAME_COUNT] = {{0, 0, 0}};

#endif