#include "accelerations.h"

//************************/
//*      VARIABLES       */
//************************/
static const char *TAG = "ACCELERATIONS";

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         FUNCTIONS DEFINITION        /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/

Accel::Accel(BMX055 *accel)
{
    _accel = accel;

    /* Initialize the communication bus */
    spi_bus_config_t buscfg =
        {
            .mosi_io_num = ACC_SPI_MOSI,
            .miso_io_num = ACC_SPI_MISO,
            .sclk_io_num = ACC_SPI_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1};
    spi_device_interface_config_t devcfg =
        {
            .command_bits = 0,
            .address_bits = 8,
            .dummy_bits = 0,
            .mode = 0, // SPI mode 0
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = ACC_SPI_SPEED * 1000 * 1000, // Clock out at 10 MHz
            .spics_io_num = ACC_SPI_CS,            // CS pin
            .queue_size = 1,
            .pre_cb = NULL,
            .post_cb = NULL,
        };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Bus initialized");

    // Attach the Accelerometer to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &_spi));
    ESP_LOGI(TAG, "Device added to HSPI bus");
}

uint8_t Accel::get_int_status(bmx_int_status_t *int_status)
{
    return _accel->get_int_status(int_status);
}

uint8_t Accel::read_acceleration_data(acc_data_i_t *dataBuffer)
{
    return _accel->read_acc_data((sensor_3D_data_t *)dataBuffer);
}

void Accel::get_acc_settings(accel_settings_t *actSettings)
{
#ifdef USE_BMX055
    bmx_acc_conf_t conf;

    _accel->get_accel_conf(&conf);
    *actSettings = regs_to_settings(conf.range, conf.bw, BMX);

#elif USE_ADXL345
    adxl_acc_conf conf;

    _accel->get_accel_conf(&conf);
    *actSettings = regs_to_settings(conf.range, conf.bw, ADXL);
#endif
}

accel_settings_t Accel::regs_to_settings(uint8_t range_reg, uint8_t bw_reg, acc_model_e device_model)
{
    accel_settings_t as;

    switch (device_model)
    {
    case BMX:
        as.range = bmx_range_converter(range_reg);
        as.sampleRate = bmx_bw_converter(bw_reg);
        break;
    case ADXL:
        break;
    default:
        break;
    }

    return as;
}

int16_t Accel::bmx_range_converter(uint8_t range_reg)
{
    switch (range_reg)
    {
    case BMX_ACC_RANGE_2G:
        return 2;
        break;
    case BMX_ACC_RANGE_4G:
        return 4;
        break;
    case BMX_ACC_RANGE_8G:
        return 8;
        break;
    case BMX_ACC_RANGE_16G:
        return 16;
        break;
    default:
        return 0;
        break;
    }
}

int16_t Accel::bmx_bw_converter(uint8_t bw_reg)
{
    switch (bw_reg)
    {
    case BMX_ACC_BW_7_81_HZ:
        return 8;
        break;
    case BMX_ACC_BW_15_63_HZ:
        return 16;
        break;
    case BMX_ACC_BW_31_25_HZ:
        return 31;
        break;
    case BMX_ACC_BW_62_5_HZ:
        return 62;
        break;
    case BMX_ACC_BW_125_HZ:
        return 125;
        break;
    case BMX_ACC_BW_250_HZ:
        return 250;
        break;
    case BMX_ACC_BW_500_HZ:
        return 500;
        break;
    case BMX_ACC_BW_1000_HZ:
        return 1000;
        break;
    default:
        return 0;
        break;
    }
}

uint8_t Accel::set_default_config(void)
{
    ESP_LOGI(TAG, "Initalizing accelerometer's registers with app default values");
    return _accel->init(_spi);
}

// uint8_t acceleration_update(void)
// {
// #ifdef USE_BMX055
//     bmx_int_status_t int_status;
// #endif
//
//     accel.get_int_status(&int_status);
//     if (int_status.int_status_1 & BMX_INT_1_ASSERTED_FIFO_WM)
//     {
//         acceleration_read_data();
//     }
//
//     return 0;
// }
//
// uint8_t acceleration_read_data()
// {
// #ifdef USE_BMX055
//     /* Read fifo data */
//     accel.read_fifo_data();
//
//     /* Convert fifo data into accelerations packets */
//     accel.fifo_extract_frames(accel_data, &acc_index);
//
//     /* Add value to analized buffer */
//     acceleration_send2FIFO();
// #endif
//
//     return ESP_OK;
// }
//
// uint8_t acceleration_send2FIFO()
// {
//     uint8_t idx;
//     acc_sensor_data data;
//     command_data_t command;
//
//     for (idx = 0; idx < acc_index; idx++)
//     {
//         data.accel_data[0] = static_cast<float>(accel_data[idx].x);
//         data.accel_data[1] = static_cast<float>(accel_data[idx].y);
//         data.accel_data[2] = static_cast<float>(accel_data[idx].z);
//
//         _pDataBuffer->push(data);
//
//         if (_pDataBuffer->full())
//         {
//             acceleration_stop_read();
//
//             command.command = APP_CMD;
//             command.value = ANALYSING;
//
//             xQueueSend(_xQueueAcc2Sys, &command, portMAX_DELAY);
//
//             return ESP_OK;
//         }
//     }
//
//     return ESP_OK;
// }
//
// uint8_t acceleration_FIFOFlush(void)
// {
//     uint8_t ret = 0;
//
// #ifdef USE_BMX055
//     bmx_fifo_conf_t _fifo_conf;
//
//     ret += accel.get_fifo_config(&_fifo_conf);
//     ret += accel.set_fifo_config(&_fifo_conf);
//     ret += accel.get_fifo_config(&_fifo_conf);
//
//     if (_fifo_conf.fifo_frame_count != 0)
//     {
//         ret++;
//     }
// #endif
//
// #ifdef APP_DEBUG_MODE
//     printf("%s\n", (_fifo_conf.fifo_frame_count == 0) ? "FIFO flushed" : "FIFO flushing: error occured");
// #endif
//
//     return ret;
// }
//
// uint8_t acceleration_start_read(void)
// {
//     uint8_t ret = 0;
//     command_data_t command;
//     const char *TAG = "acceleration_start_read";
//
// #ifdef USE_BMX055
//     bmx_fifo_conf_t _fifo_conf;
//
//     ESP_LOGI(TAG, "Get actual FIFO configuration");
//     ret += accel.get_fifo_config(&_fifo_conf);
//
//     ESP_LOGI(TAG, "Enabling FIFO data collection ");
//     _fifo_conf.fifo_mode_select = BMX_MODE_FIFO;
//     ret += accel.set_fifo_config(&_fifo_conf);
// #endif
//
//     command.command = APP_CMD;
//     command.value = VIBES_REC;
//
//     xQueueSend(_xQueueAcc2Sys, &command, portMAX_DELAY);
//
//     return ret;
// }
//
// uint8_t acceleration_stop_read(void)
// {
//     uint8_t ret = 0;
//
// #ifdef USE_BMX055
//     bmx_fifo_conf_t _fifo_conf;
//
//     ret += accel.get_fifo_config(&_fifo_conf);
//
//     _fifo_conf.fifo_mode_select = BMX_MODE_BYPASS;
//     ret += accel.set_fifo_config(&_fifo_conf);
// #endif
//
//     return ret;
// }
