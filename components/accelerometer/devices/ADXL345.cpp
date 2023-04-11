#include "include/ADXL345.h"

//************************/
//*      VARIABLES       */
//************************/
static const char *TAG = "ADXL345";

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         FUNCTIONS DEFINITION        /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
ADXL345::ADXL345()
{
}

ADXL345::~ADXL345()
{
}

adxl_error_e ADXL345::init(spi_device_handle_t spiHandle)
{
    uint8_t dummy_value;
    uint32_t l_dummy_value;

    _spi = spiHandle;

    _xSpiSemaphore = xSemaphoreCreateMutex();

    if (get_chipID(&dummy_value) != ADXL_OK)
    {
        printf("* DEVICE ID READ FAILED *\n");
        return ADXL_ERR_DEV_NOT_FOUND;
    }

    printf("Chip id : 0x%x\n", dummy_value);
    if (dummy_value != ADXL345_CHIPID)
    {
        return ADXL_ERR_DEV_NOT_FOUND;
    }

    /* Get accelerometer actual configuration */
    if (get_accel_conf(&_acc_conf) != ADXL_OK)
    {
        printf("* ACCEL CONFIG READ FAILED *\n");
        return ADXL_ERR_INIT;
    }

    printf("******************************\n");
    printf("* ACCEL CONFIG DEFAULT VALUE *\n");
    printf("******************************\n");
    printf("Range: 0x0%x\n", _acc_conf.range);
    printf("Bandwidth: 0x0%x\n", _acc_conf.bw);

    /* Set accelerometer configuration required value */
    _acc_conf.bw = ADXL345_DATARATE_3200_HZ;
    _acc_conf.range = ADXL345_RANGE_2_G;

    /* Set accelerometer actual configuration */
    if (set_accel_conf(&_acc_conf) != ADXL_OK)
    {
        printf("* ACCEL CONFIG WRITE FAILED *\n");
        return ADXL_ERR_INIT;
    }

    /* Get accelerometer actual configuration */
    if (get_accel_conf(&_acc_conf) != ADXL_OK)
    {
        printf("* ACCEL CONFIG READ FAILED *\n");
        return ADXL_ERR_INIT;
    }

    printf("******************************\n");
    printf("*   ACCEL CONFIG APP VALUE   *\n");
    printf("******************************\n");
    printf("Range: 0x0%x\n", _acc_conf.range);
    printf("Bandwidth: 0x0%x\n", _acc_conf.bw);

    /* Set power mode to NORMAL */
    if (set_power_mode(ADXL_NORMAL_MODE) != ADXL_OK)
    {
        printf("* POWER MODE WRITE FAILED *\n");
    }

    printf("\nAccelerometer initialization COMPLETED\n");
    return ADXL_OK;
}

adxl_error_e ADXL345::get_chipID(uint8_t *chipID_value)
{
    return _read_regs(ADXL345_REG_CHIPID, chipID_value, 1);
}

adxl_error_e ADXL345::get_accel_conf(adxl_acc_conf_t *acc_conf_struct)
{
    if (get_range(&acc_conf_struct->range) != ADXL_OK)
    {
        return ADXL_ERR_INIT;
    }
    if (get_dataRate(&acc_conf_struct->bw) != ADXL_OK)
    {
        return ADXL_ERR_INIT;
    }
    
    return ADXL_OK;
}

adxl_error_e ADXL345::get_int_status(adxl_int_status_t *int_status)
{
    uint8_t reg_data;

    if (int_status == NULL)
    {
        return ADXL_ERR_NULL_POINTER;
    }

    if (_read_regs(ADXL345_REG_INT_SOURCE, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    int_status->int_status = reg_data;

    return ADXL_OK;
}

adxl_error_e ADXL345::read_acc_data(adxl_3D_data_t *accel_data)
{
    uint8_t reg_data[6];
    adxl_int_status_t status;

    if (accel_data == NULL)
    {
        return ADXL_ERR_NULL_POINTER;
    }
    if (get_int_status(&status) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }
    if (status.data_rdy != 1)
    {
        return ADXL_ERR_RD;
    }
    if (_read_regs_dma(ADXL345_REG_DATAX0, reg_data, 6) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    _convert_reg_data_to_accel(accel_data, reg_data);

    return ADXL_OK;
}

adxl_error_e ADXL345::set_accel_conf(adxl_acc_conf_t *acc_conf_struct)
{
    if (set_range(&acc_conf_struct->range) != ADXL_OK)
    {
        return ADXL_ERR_INIT;
    }
    if (set_dataRate(&acc_conf_struct->bw) != ADXL_OK)
    {
        return ADXL_ERR_INIT;
    }

    return ADXL_OK;
}

adxl_error_e ADXL345::set_range(uint8_t *range_val)
{
    uint8_t reg_data;

    if (_read_regs(ADXL345_REG_DATA_FORMAT, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    reg_data = ADXL_SET_BITS_POS_0(reg_data, ADXL_RANGE, *range_val);

    if (_write_regs(ADXL345_REG_DATA_FORMAT, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    return ADXL_OK;
}

adxl_error_e ADXL345::get_range(uint8_t *range_val)
{
    uint8_t reg_data;

    if (_read_regs(ADXL345_REG_DATA_FORMAT, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    *range_val = ADXL_GET_BITS_POS_0(reg_data, ADXL_RANGE);

    return ADXL_OK;
}

adxl_error_e ADXL345::set_dataRate(uint8_t *odr_val)
{
    uint8_t reg_data;

    if (_read_regs(ADXL345_REG_BW_RATE, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    reg_data = ADXL_SET_BITS_POS_0(reg_data, ADXL_BW, *odr_val);

    if (_write_regs(ADXL345_REG_BW_RATE, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    return ADXL_OK;
}

adxl_error_e ADXL345::get_dataRate(uint8_t *odr_val)
{
    uint8_t reg_data;

    if (_read_regs(ADXL345_REG_BW_RATE, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    *odr_val = ADXL_GET_BITS_POS_0(reg_data, ADXL_BW);

    return ADXL_OK;
}

adxl_error_e ADXL345::set_power_mode(adxl_power_mode_e power_mode)
{
    switch (power_mode)
    {
    case ADXL_NORMAL_MODE:
        _set_normal_mode();
        break;
    case ADXL_STANDBY_MODE:
        _set_standby_mode();
        break;
    case ADXL_SLEEP_MODE:
        // TODO
        break;
    default:
        break;
    }

    return ADXL_OK;
}

adxl_error_e ADXL345::get_power_mode(adxl_power_mode_e *power_mode)
{
    return ADXL_OK;
}

void ADXL345::_convert_reg_data_to_accel(adxl_3D_data_t *accel, uint8_t *data_array)
{
    uint16_t reg_data;

    /* Accel X axis data */
    reg_data = (uint16_t)((data_array[1] << 8) | data_array[0]);
    accel->x = (int16_t)reg_data;

    /* Accel Y axis data */
    reg_data = (uint16_t)((data_array[3] << 8) | data_array[2]);
    accel->y = (int16_t)reg_data;

    /* Accel Z axis data */
    reg_data = (uint16_t)((data_array[5] << 8) | data_array[4]);
    accel->z = (int16_t)reg_data;
}

adxl_error_e ADXL345::_set_normal_mode(void)
{
    uint8_t reg_data = 0x08;

    if (_write_regs(ADXL345_REG_POWER_CTL, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    return ADXL_OK;
}

adxl_error_e ADXL345::_set_standby_mode(void)
{
    uint8_t reg_data = 0x00;

    if (_write_regs(ADXL345_REG_POWER_CTL, &reg_data, 1) != ADXL_OK)
    {
        return ADXL_ERR_RD;
    }

    return ADXL_OK;
}

/*!
 * @details This API reads the data from the given register
 * address of the sensor
 *
 * @param[in] reg_addr  : Register address from where the data is to be read
 * @param[out] data_rd  : Pointer to data buffer to store the read data
 * @param[in] length    : No of bytes of data to be read
 *
 * @return Result of API execution status
 * @return 0 -> Success
 * @return != 0 -> Fail
 */
adxl_error_e ADXL345::_read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length)
{
    esp_err_t ret = ESP_FAIL;

    uint8_t dummy_buffer[length];
    memset(dummy_buffer, 0, sizeof(dummy_buffer[0] * length));

    uint8_t mask;
    if (length > 1)
    {
        mask = ADXL345_SPI_RD_MASK | ADXL345_SPI_MULTI_MASK;
    }
    else
    {
        mask = ADXL345_SPI_RD_MASK;
    }

    spi_transaction_t t =
        {
            .addr = (uint64_t)(reg_addr | mask),
            .length = 8 * length,
            .rxlength = 8 * length,
            .tx_buffer = dummy_buffer, // Dummy data buffer
            .rx_buffer = data_rd,
        };

    if ((length > 0) && (data_rd != NULL))
    {
        if (xSemaphoreTake(_xSpiSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ret = spi_device_polling_transmit(_spi, &t);
            xSemaphoreGive(_xSpiSemaphore);
        }
    }
    else
    {
        return ADXL_ERR_NULL_POINTER;
    }

    if (ret == 0)
    {
        return ADXL_OK;
    }
    else
    {
        return ADXL_ERR_RD;
    }
}

/*!
 * @details This API writes the given data to the register address
 * of the sensor.
 *
 * @param[in] reg_addr : Register address where the reg_data is to be written
 * @param[in] data_wr  : Pointer to data buffer which is to be written
 *                       in the reg_addr of sensor
 * @param[in] length   : No of bytes of data to be written
 *
 * @return Result of API execution status
 * @return 0 -> Success
 * @return != 0 -> Fail
 */
adxl_error_e ADXL345::_write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length)
{
    esp_err_t ret = ESP_FAIL;

    spi_transaction_t t =
        {
            .addr = (uint64_t)(reg_addr & ADXL345_SPI_WR_MASK),
            .length = 8,
            .tx_buffer = data_wr // Buffer of data to write
        };
    // ! there is a bug inside the spi_device_polling_transmit which cause no data transmit when (t.length > 8)
    if ((length > 0) && (data_wr != NULL))
    {
        for (uint32_t i = 0; i < length; i++)
        {
            t.addr = (uint64_t)((reg_addr + i) & ADXL345_SPI_WR_MASK),
            t.tx_buffer = &data_wr[i];
            if (xSemaphoreTake(_xSpiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                ret = spi_device_polling_transmit(_spi, &t);
                xSemaphoreGive(_xSpiSemaphore);
            }
        }
    }
    else
    {
        return ADXL_ERR_NULL_POINTER;
    }

    if (ret == 0)
    {
        return ADXL_OK;
    }
    else
    {
        return ADXL_ERR_WR;
    }
}

/*!
 * @details This API reads the data from the given register
 * address of the sensor without interrupt the execution of the task.
 *
 * @param[in] reg_addr  : Register address from where the data is to be read
 * @param[out] data_rd  : Pointer to data buffer to store the read data
 * @param[in] length    : No of bytes of data to be read
 *
 * @return Result of API execution status
 * @return 0 -> Success
 * @return != 0 -> Fail
 */
adxl_error_e ADXL345::_read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length)
{
    esp_err_t ret = ESP_FAIL;

    uint8_t dummy_buffer[length];
    memset(dummy_buffer, 0, sizeof(dummy_buffer[0] * length));

    uint8_t mask;
    if (length == 1)
    {
        mask = ADXL345_SPI_RD_MASK;
    }
    else
    {
        mask = ADXL345_SPI_RD_MASK | ADXL345_SPI_MULTI_MASK;
    }

    spi_transaction_t t =
        {
            .addr = (uint64_t)(reg_addr | mask),
            .length = 8 * length,
            .rxlength = 8 * length,
            .tx_buffer = dummy_buffer, // Dummy data buffer
            .rx_buffer = data_rd,
        };

    if ((length > 0) && (data_rd != NULL))
    {
        if (xSemaphoreTake(_xSpiSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ret = spi_device_transmit(_spi, &t);
            xSemaphoreGive(_xSpiSemaphore);
        }
    }
    else
    {
        return ADXL_ERR_NULL_POINTER;
    }

    if (ret == 0)
    {
        return ADXL_OK;
    }
    else
    {
        return ADXL_ERR_RD;
    }
}
