#pragma once

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//*******************************/
//*         TYPE DEFINES        */
//*******************************/
struct acc_packet_t
{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
};

struct accel_settings_t
{
    uint16_t range; // g-forge
    uint16_t band;  // ODR/2
};

enum accel_error_e
{
    ACC_OK,
    ACC_ERR_RD,
    ACC_ERR_WR,
    ACC_ERR_INIT,
    ACC_ERR_DEV_NOT_FOUND,
    ACC_ERR_NULL_POINTER,
    ACC_ERR_INVALID_POWERMODE,
    ACC_ERR_READ_FIFO_CONFIG,
    ACC_ERR_READ_FIFO_DATA,
    ACC_ERR_RD_DMA,
    ACC_ERR_FIFO_FRAME_EMPTY,
    ACC_ERR_INVALID_CONFIG,
    ACC_ERR_NO_NEW_AVAILABLE,
};
//*******************************/
//*      CLASS DECLARATION       /
//*******************************/
class Accel
{
    spi_device_handle_t _spi_bus;

public:
    virtual accel_error_e init(spi_host_device_t spi_bus, gpio_num_t miso, gpio_num_t mosi, gpio_num_t sck, gpio_num_t cs, int32_t speed_mhz) = 0;
    virtual accel_error_e set_default_config(void) = 0;
    virtual accel_error_e get_acc_settings(accel_settings_t &settings) = 0;
    virtual accel_error_e set_acc_settings(const accel_settings_t &settings) = 0;

    virtual accel_error_e get_int_status(uint8_t &int_status) = 0;
    virtual accel_error_e read_accelerations_packet(acc_packet_t &dataBuffer) = 0;

    virtual uint8_t set_range_reg(uint8_t range) = 0;
    virtual uint8_t get_range_value(void) = 0;
    virtual uint8_t set_bandwidth_reg(uint8_t bandwidth) = 0;
    virtual uint8_t get_bandwidth_value(void) = 0;
};