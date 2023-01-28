#ifndef INC_ACCELERATIONS_H
#define INC_ACCELERATIONS_H

#include "../common/common_def.h"
#include "../shared/shared_data.h"

#include "drivers/BMX055.h"
#include "drivers/BMX055_defs.h"
#include "drivers/ADXL345.h"
#include "drivers/ADXL345_def.h"

//**********************/
//*      DEFINES       /
//*********************/
#ifdef USE_BMX055
#define MAX_DATA_FRAME_COUNT BMX_FIFO_DATA_FRAME_COUNT
#endif

#define MAX_COMMAND_COUNT 10

//*******************************/
//*         TYPE DEFINES        */
//*******************************/
struct accel_settings_t
{
    uint16_t range;      // G
    uint16_t sampleRate; // sample/sec
};

enum acc_model_e
{
    BMX,
    ADXL
};

//*******************************/
//*      CLASS DECLARATION       /
//*******************************/
class Accel
{
public:
    Accel(BMX055 *accel);
    Accel(ADXL345 *accel);

    uint8_t set_default_config(void);
    uint8_t get_int_status(bmx_int_status_t *int_status);
    uint8_t get_int_status(adxl_int_status_t *int_status);
    uint8_t read_acceleration_data(acc_data_i_t *dataBuffer);

    void get_acc_settings(accel_settings_t *actSettings);
    accel_settings_t regs_to_settings(uint8_t range_reg, uint8_t bw_reg, acc_model_e device_model);

private:
#ifdef USE_BMX055
    BMX055 *_accel;
#elif USE_ADXL345
    ADXL345 *_accel;
#endif
    spi_device_handle_t _spi;

    int16_t bmx_range_converter(uint8_t range_reg);
    int16_t bmx_bw_converter(uint8_t bw_reg);
    int16_t adxl_range_converter(uint8_t range_reg);
    int16_t adxl_bw_converter(uint8_t bw_reg);

protected:
};

#endif