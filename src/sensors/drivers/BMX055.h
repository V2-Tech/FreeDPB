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

class BMX055
{
public:
    bmx_fifo_conf _fifo_conf;

    bmx_error init(spi_device_handle_t spiHandle);

    bmx_error get_chipID(uint8_t *chipID_value);

    bmx_error get_accel_conf(bmx_acc_conf *acc_conf_struct);
    bmx_error set_accel_conf(bmx_acc_conf *acc_conf_struct);
    bmx_error get_fifo_config(bmx_fifo_conf *fifo_conf_struct);
    bmx_error set_fifo_config(bmx_fifo_conf *fifo_conf_struct);

    bmx_error set_interrupt_source(uint32_t int_source_to_en);
    bmx_error get_interrupt_source(bmx_int_scr *int_en);

    bmx_error get_power_mode(bmx_power_mode *power_mode);
    bmx_error set_power_mode(bmx_power_mode power_mode);

    bmx_error soft_reset();

    bmx_error get_int_status(bmx_int_status *int_status);
    bmx_error read_acc_data(sensor_3D_data *accel_data);

    bmx_error read_fifo_data();
    bmx_error get_fifo_frame_count();
    bmx_error fifo_extract_frames(sensor_3D_data *accel_data, uint16_t *acc_index);
    bmx_error decode_fifo_frames(sensor_3D_data *accel_data, uint16_t *acc_index);
    bmx_error fifo_frame_empty_check(uint16_t *data_index);

private:
    spi_device_handle_t _spi;
    SemaphoreHandle_t _xSpiSemaphore;
    bmx_acc_conf _acc_conf;
    bmx_power_mode _power_mode;

    uint8_t _FIFO_data[BMX_FIFO_BUFFER_SIZE] = {0};

    bmx_error _set_normal_mode();

    bmx_error _read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
    bmx_error _write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length);

    bmx_error _read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);

    void _convert_reg_data_to_accel(sensor_3D_data *accel_data, uint8_t *reg_data);
};

#endif