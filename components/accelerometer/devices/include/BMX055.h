#ifndef INC_BMX055_H
#define INC_BMX055_H

#include "esp_log.h"

#include "../../include/accelerometer.hpp"
#include "BMX055_defs.h"

//*******************************/
//*      CLASS DECLARATION      */
//*******************************/
class BMX055 : public Accel
{
public:
    accel_error_e init(spi_host_device_t spi_bus, gpio_num_t miso, gpio_num_t mosi, gpio_num_t sck, gpio_num_t cs, int32_t speed_mhz) override;
    accel_error_e set_default_config() override;
    accel_error_e get_acc_settings(accel_settings_t &settings) override;
    accel_error_e set_acc_settings(const accel_settings_t &settings) override;
    uint8_t set_range_reg(uint8_t range) override;
    uint8_t get_range_value(void) override;
    uint8_t set_bandwidth_reg(uint8_t bandwidth) override;
    uint8_t get_bandwidth_value(void) override;
    accel_error_e get_int_status(uint8_t &int_status) override;
    accel_error_e read_accelerations_packet(acc_packet_t &dataBuffer) override;

    accel_error_e set_interrupt_source(uint32_t int_source_to_en);
    accel_error_e get_interrupt_source(bmx_int_scr_u *int_en);

    accel_error_e get_power_mode(bmx_power_mode_e *power_mode);
    accel_error_e set_power_mode(bmx_power_mode_e power_mode);

    accel_error_e soft_reset();

    accel_error_e get_int_status(bmx_int_status_t *int_status);
    accel_error_e read_acc_data_DMA(sensor_3D_data_t *accel_data);
    accel_error_e read_acc_data(sensor_3D_data_t *accel_data);

    accel_error_e read_fifo_data();
    accel_error_e get_fifo_frame_count();
    accel_error_e fifo_extract_frames(sensor_3D_data_t *accel_data, uint16_t *acc_index);
    accel_error_e decode_fifo_frames(sensor_3D_data_t *accel_data, uint16_t *acc_index);
    accel_error_e fifo_frame_empty_check(uint16_t *data_index);

private:
    spi_device_handle_t _spi;
    SemaphoreHandle_t _xSpiSemaphore;
    bmx_acc_conf_t _acc_conf;
    bmx_power_mode_e _power_mode;
    bmx_fifo_conf_t _fifo_conf;
    uint8_t _FIFO_data[BMX_FIFO_BUFFER_SIZE] = {0};

    accel_error_e _get_chipID(uint8_t *chipID_value);
    accel_error_e _set_normal_mode();

    accel_error_e _get_accel_conf(bmx_acc_conf_t *acc_conf_struct);
    accel_error_e _set_accel_conf(bmx_acc_conf_t *acc_conf_struct);
    accel_error_e _get_fifo_config(bmx_fifo_conf_t *fifo_conf_struct);
    accel_error_e _set_fifo_config(bmx_fifo_conf_t *fifo_conf_struct);

    accel_error_e _read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
    accel_error_e _write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length);
    accel_error_e _read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);

    int16_t _reg_value_2_range(uint8_t range_reg_value);
    uint8_t _range_2_reg_value(int16_t range);
    int16_t _reg_value_2_bw(uint8_t bw_reg_value);
    uint8_t _bw_2_reg_value(int16_t range);

    void _convert_reg_data_to_accel(sensor_3D_data_t *accel_data, uint8_t *reg_data);
};

#endif