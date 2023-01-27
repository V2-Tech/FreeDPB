#ifndef INC_BMX055_H
#define INC_BMX055_H

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

#include "BMX055_defs.h"

//*******************************/
//*      CLASS DECLARATION      */
//*******************************/
class BMX055
{
public:
    bmx_fifo_conf_t _fifo_conf;

    bmx_error_e init(spi_device_handle_t spiHandle);

    bmx_error_e get_chipID(uint8_t *chipID_value);

    bmx_error_e get_accel_conf(bmx_acc_conf_t *acc_conf_struct);
    bmx_error_e set_accel_conf(bmx_acc_conf_t *acc_conf_struct);
    bmx_error_e get_fifo_config(bmx_fifo_conf_t *fifo_conf_struct);
    bmx_error_e set_fifo_config(bmx_fifo_conf_t *fifo_conf_struct);

    bmx_error_e set_interrupt_source(uint32_t int_source_to_en);
    bmx_error_e get_interrupt_source(bmx_int_scr_u *int_en);

    bmx_error_e get_power_mode(bmx_power_mode_e *power_mode);
    bmx_error_e set_power_mode(bmx_power_mode_e power_mode);

    bmx_error_e soft_reset();

    bmx_error_e get_int_status(bmx_int_status_t *int_status);
    bmx_error_e read_acc_data(sensor_3D_data_t *accel_data);

    bmx_error_e read_fifo_data();
    bmx_error_e get_fifo_frame_count();
    bmx_error_e fifo_extract_frames(sensor_3D_data_t *accel_data, uint16_t *acc_index);
    bmx_error_e decode_fifo_frames(sensor_3D_data_t *accel_data, uint16_t *acc_index);
    bmx_error_e fifo_frame_empty_check(uint16_t *data_index);

private:
    spi_device_handle_t _spi;
    SemaphoreHandle_t _xSpiSemaphore;
    bmx_acc_conf_t _acc_conf;
    bmx_power_mode_e _power_mode;

    uint8_t _FIFO_data[BMX_FIFO_BUFFER_SIZE] = {0};

    bmx_error_e _set_normal_mode();

    bmx_error_e _read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
    bmx_error_e _write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length);

    bmx_error_e _read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);

    void _convert_reg_data_to_accel(sensor_3D_data_t *accel_data, uint8_t *reg_data);
};

#endif