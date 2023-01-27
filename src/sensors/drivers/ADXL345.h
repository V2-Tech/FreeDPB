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

private:
    spi_device_handle_t _spi;
    SemaphoreHandle_t _xSpiSemaphore;
    adxl_acc_conf_t _acc_conf;

    adxl_error_e _read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
    adxl_error_e _write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length);

    adxl_error_e _read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length);
};

#endif