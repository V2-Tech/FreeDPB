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
#ifdef USE_BMX055
    _accel = accel;
#endif
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
            .spics_io_num = ACC_SPI_CS,                    // CS pin
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

Accel::Accel(ADXL345 *accel)
{
#ifdef USE_ADXL345
    _accel = accel;
#endif

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
            .mode = 3, // SPI mode 0
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = ACC_SPI_SPEED * 1000 * 1000, // Clock out at 10 MHz
            .spics_io_num = ACC_SPI_CS,                    // CS pin
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
#ifdef USE_BMX055
    return _accel->get_int_status(int_status);
#endif
    return ESP_FAIL;
}

uint8_t Accel::get_int_status(adxl_int_status_t *int_status)
{
#ifdef USE_ADXL345
    return _accel->get_int_status(int_status);
#endif
    return ESP_FAIL;
}

uint8_t Accel::read_acceleration_data(acc_data_i_t *dataBuffer)
{
#ifdef USE_BMX055
    return _accel->read_acc_data((sensor_3D_data_t *)dataBuffer);
#elif USE_ADXL345
    return _accel->read_acc_data((adxl_3D_data_t *)dataBuffer);
#endif
}

void Accel::get_acc_settings(accel_settings_t *actSettings)
{
#ifdef USE_BMX055
    bmx_acc_conf_t conf;

    _accel->get_accel_conf(&conf);
    *actSettings = regs_to_settings(conf.range, conf.bw, BMX);

#elif USE_ADXL345
    adxl_acc_conf_t conf;

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
        as.range = _bmx_range_2_value_converter(range_reg);
        as.band = _bmx_bw_converter(bw_reg);
        break;
    case ADXL:
        as.range = _adxl_range_2_value_converter(range_reg);
        as.band = _adxl_bw_converter(bw_reg);
        break;
    default:
        break;
    }

    return as;
}

uint8_t Accel::set_range(uint8_t range)
{
#ifdef USE_BMX055
    bmx_acc_conf_t config;
#endif
#ifdef USE_ADXL345
    adxl_acc_conf_t config;
#endif

    if (_accel->get_accel_conf(&config) != ESP_OK)
    {
        return ESP_FAIL;
    }

#ifdef USE_BMX055
    config.range = _bmx_value_2_range_converter(range);
#endif
#ifdef USE_ADXL345
    config.range = _adxl_value_2_range_converter(range);
#endif

    if (_accel->set_accel_conf(&config) != ESP_OK)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

uint8_t Accel::set_bandwidth(uint8_t bandwidth)
{
    return ESP_OK;
}

int16_t Accel::_bmx_range_2_value_converter(uint8_t range_reg)
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

int16_t Accel::_bmx_bw_converter(uint8_t bw_reg)
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

int16_t Accel::_adxl_range_2_value_converter(uint8_t range_reg)
{
    switch (range_reg)
    {
    case ADXL345_RANGE_2_G:
        return 2;
        break;
    case ADXL345_RANGE_4_G:
        return 4;
        break;
    case ADXL345_RANGE_8_G:
        return 8;
        break;
    case ADXL345_RANGE_16_G:
        return 16;
        break;
    default:
        return 0;
        break;
    }
}

int16_t Accel::_adxl_bw_converter(uint8_t bw_reg)
{
    switch (bw_reg)
    {
    case ADXL345_DATARATE_50_HZ:
        return 25;
        break;
    case ADXL345_DATARATE_100_HZ:
        return 50;
        break;
    case ADXL345_DATARATE_200_HZ:
        return 100;
        break;
    case ADXL345_DATARATE_400_HZ:
        return 200;
        break;
    case ADXL345_DATARATE_800_HZ:
        return 400;
        break;
    case ADXL345_DATARATE_1600_HZ:
        return 800;
        break;
    case ADXL345_DATARATE_3200_HZ:
        return 1600;
        break;
    default:
        return 0;
        break;
    }
}

uint8_t Accel::_bmx_value_2_range_converter(uint8_t value)
{
    switch (value)
    {
    case 2:
        return BMX_ACC_RANGE_2G;
        break;
    case 4:
        return BMX_ACC_RANGE_4G;
        break;
    case 8:
        return BMX_ACC_RANGE_8G;
        break;
    case 16:
        return BMX_ACC_RANGE_16G;
        break;
    default:
        return 0;
        break;
    }
}

uint8_t Accel::_adxl_value_2_range_converter(uint8_t value)
{
    switch (value)
    {
    case 2:
        return ADXL345_RANGE_2_G;
        break;
    case 4:
        return ADXL345_RANGE_4_G;
        break;
    case 8:
        return ADXL345_RANGE_8_G;
        break;
    case 16:
        return ADXL345_RANGE_16_G;
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