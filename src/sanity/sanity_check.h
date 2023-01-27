#ifndef INC_SANITY_CHECK_H
#define INC_SANITY_CHECK_H

#include "../common/common_def.h"

#define USED_ACCEL (USE_BMX055 + USE_ADXL345)
#if USED_ACCEL > 1
#error "Multiple accelerometer device type has been selected. Choose only one type."
#endif

#endif