#ifndef INC_APP_DEF_H
#define INC_APP_DEF_H

#include "../../main/common_def.h"

#include "../motor/motor.h"


#define INIT_ISR_DONE   1U<<1
#define INIT_ESC_DONE   1U<<2
#define INIT_ACCEL_DONE 1U<<3
#define INIT_RPM_S_DONE 1U<<4

/************************************/
/*      STRUCT DECLARATIONS         */
/************************************/


/************************************/
/*   SHARED VARIABLES DECLARATIONS  */
/************************************/
extern app_steps _app_step;

/************************************/
/*      VARIABLES DECLARATIONS      */
/************************************/
inline const char *TAG = "DPB";

/************************************/
/*      FUNCTION DECLARATIONS       */
/************************************/

static void _vibeTimerCallback(TimerHandle_t pxTimer);
#endif