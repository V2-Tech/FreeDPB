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
            .mosi_io_num = HSPI_MOSI,
            .miso_io_num = HSPI_MISO,
            .sclk_io_num = HSPI_SCLK,
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
            .clock_speed_hz = 10 * 1000 * 1000, // Clock out at 10 MHz
            .spics_io_num = HSPI_SS,            // CS pin
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

uint8_t Accel::get_int_status(bmx_int_status *int_status)
{
    return _accel->get_int_status(int_status);
}

uint8_t Accel::read_acceleration_data(acc_data_i_t *dataBuffer)
{
    return _accel->read_acc_data((sensor_3D_data *)dataBuffer);
}

uint8_t Accel::__set_default_config()
{
    ESP_LOGI(TAG, "Initalizing accelerometer's registers with app default values");
    return _accel->init(_spi);
}

// uint8_t acceleration_update(void)
// {
// #ifdef USE_BMX055
//     bmx_int_status int_status;
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
//     bmx_fifo_conf _fifo_conf;
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
//     bmx_fifo_conf _fifo_conf;
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
//     bmx_fifo_conf _fifo_conf;
// 
//     ret += accel.get_fifo_config(&_fifo_conf);
// 
//     _fifo_conf.fifo_mode_select = BMX_MODE_BYPASS;
//     ret += accel.set_fifo_config(&_fifo_conf);
// #endif
// 
//     return ret;
// }


