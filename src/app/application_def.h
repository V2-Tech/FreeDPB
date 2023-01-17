#ifndef INC_APP_DEF_H
#define INC_APP_DEF_H

#include "../shared/common_def.h"
#include "../shared/shared_data.h"

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

/************************************/
/*      VARIABLES DECLARATIONS      */
/************************************/
inline const char *TAG = "DPB";

/************************************/
/*      FUNCTION DECLARATIONS       */
/************************************/

#endif