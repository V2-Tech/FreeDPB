#ifndef INC_ACCELERATIONS_H
#define INC_ACCELERATIONS_H

#include "../shared/common_def.h"
#include "../shared/shared_data.h"

#ifdef USE_BMX055
#include "drivers/BMX055.h"
#include "drivers/BMX055_defs.h"
#endif

/*********************
 *      DEFINES
 *********************/
#ifdef USE_BMX055
#define MAX_DATA_FRAME_COUNT BMX_FIFO_DATA_FRAME_COUNT
#endif

#define MAX_COMMAND_COUNT 10

/****************************
 *  FUNCTIONS DECLARATIONS  *
 ****************************/
class Accel
{
public:
    Accel(BMX055 *accel);

    uint8_t get_int_status(bmx_int_status *int_status);
    uint8_t read_acceleration_data(acc_data_i_t *dataBuffer);

private:
    BMX055 *_accel;
    spi_device_handle_t _spi;

protected:
    uint8_t __set_default_config();
};

#endif