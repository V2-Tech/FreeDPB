#ifndef INC_ADXL345_H
#define INC_ADXL345_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "ADXL345_def.h"

//*******************************/
//*      CLASS DECLARATION      */
//*******************************/
class ADXL345
{
public:
    ADXL345();
    ~ADXL345();

    adxl_error_e init(spi_device_handle_t spiHandle);
    adxl_error_e get_chipID(uint8_t *chipID_value);
    adxl_error_e get_int_status(adxl_int_status_t *int_status);

    adxl_error_e read_acc_data(adxl_3D_data_t *accel_data);

    adxl_error_e set_accel_conf(adxl_acc_conf_t *acc_conf_struct);
    adxl_error_e get_accel_conf(adxl_acc_conf_t *acc_conf_struct);
    adxl_error_e set_range(uint8_t *range_val);
    adxl_error_e get_range(uint8_t *range_val);
    adxl_error_e set_dataRate(uint8_t *odr_val);
    adxl_error_e get_dataRate(uint8_t *odr_val);

    adxl_error_e set_power_mode(adxl_power_mode_e power_mode);
    adxl_error_e get_power_mode(adxl_power_mode_e *power_mode);

private:
    spi_device_handle_t _spi;
    SemaphoreHandle_t _xSpiSemaphore;
    adxl_acc_conf_t _acc_conf;

    void _convert_reg_data_to_accel(adxl_3D_data_t *accel, uint8_t *data_array);
    adxl_error_e _set_normal_mode(void);
    adxl_error_e _set_standby_mode(void);

    adxl_error_e _read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
    adxl_error_e _read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
    adxl_error_e _write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length);
};

#endif