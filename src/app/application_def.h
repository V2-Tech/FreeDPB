#ifndef INC_APP_DEF_H
#define INC_APP_DEF_H

#include "../../main/common_def.h"

#include "../motor/motor.h"

/************************************/
/*      STRUCT DECLARATIONS         */
/************************************/


/************************************/
/*   SHARED VARIABLES DECLARATIONS  */
/************************************/
extern app_status _app_step;

/************************************/
/*      VARIABLES DECLARATIONS      */
/************************************/
static QueueHandle_t _xQueueSysInput;
static QueueHandle_t _xQueueSysOutput;
static FIFOBuffer<acc_sensor_data> *_pDataBuffer;

Motor *_pMotor;

TimerHandle_t _vibeRecTimer;
/************************************/
/*      FUNCTION DECLARATIONS       */
/************************************/
static void IRAM_ATTR opto_isr_handler(void* arg);

#endif