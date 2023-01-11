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
extern app_steps _app_step;

/************************************/
/*      VARIABLES DECLARATIONS      */
/************************************/

/************************************/
/*      FUNCTION DECLARATIONS       */
/************************************/

static void _vibeTimerCallback(TimerHandle_t pxTimer);
#endif