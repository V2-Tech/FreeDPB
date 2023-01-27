#include "ADXL345.h"

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
    int8_t status = 0;
    uint8_t dummy_value;
    uint32_t l_dummy_value;

    _spi = spiHandle;

    _xSpiSemaphore = xSemaphoreCreateMutex();

    if (get_chipID(&dummy_value) == ADXL_OK)
    {
        printf("Chip id : 0x%x\n", dummy_value);
        if (dummy_value == ADXL345_CHIPID)
        {
            return ADXL_OK;
        }
    }

    if (status == 0)
    {
        printf("\nAccelerometer initialization COMPLETED\n");
        return ADXL_OK;
    }
    else
    {
        printf("\nAccelerometer initialization FAILED\n");
        return ADXL_ERR_INIT;
    }
}

adxl_error_e ADXL345::get_chipID(uint8_t *chipID_value)
{
    return _read_regs(ADXL345_REG_CHIPID, chipID_value, 1);
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
