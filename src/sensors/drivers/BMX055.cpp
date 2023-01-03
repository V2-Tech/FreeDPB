#include "BMX055.h"

bmx_error BMX055::init(spi_device_handle_t spiHandle)
{
    int8_t status = 0;
    uint8_t dummy_value;
    uint32_t l_dummy_value;
    bmx_int_scr int_src_active;
    bmx_power_mode pwr_mode;

    _spi = spiHandle;

    _xSpiSemaphore = xSemaphoreCreateMutex();

    if (get_chipID(&dummy_value) == BMX_OK)
    {
        printf("Chip id : 0x%x\n", dummy_value);
        if (dummy_value == BMX_CHIPID)
        {
            /* Get accelerometer actual configuration */
            status += get_accel_conf(&_acc_conf);
            if (status != 0)
            {
                printf("* ACCEL CONFIG FAILED *\n");
            }
            else
            {
                printf("******************************\n");
                printf("* ACCEL CONFIG DEFAULT VALUE *\n");
                printf("******************************\n");
                printf("Range: 0x0%x\n", _acc_conf.range);
                printf("Bandwidth: 0x0%x\n", _acc_conf.bw);
                printf("Shadowing mode: ");
                printf("%s\n", _acc_conf.shadow_dis ? "true" : "false");
                printf("Data type: ");
                printf("%s\n", _acc_conf.data_high_bw ? "Unfiltered" : "filtered");
            }

            /* Set accelerometer offsets using fast compensation feature */
            // TODO
            
            /* Get FIFO actual configuration */
            status += get_fifo_config(&_fifo_conf);
            if (status != 0)
            {
                printf("* FIFO CONFIG READ FAILED *\n");
            }
            else
            {
                printf("*****************************\n");
                printf("* FIFO CONFIG DEFAULT VALUE *\n");
                printf("*****************************\n");
                printf("FIFO data type: %x\n", _fifo_conf.fifo_data_select);
                printf("FIFO mode: %x\n", _fifo_conf.fifo_mode_select);
                printf("FIFO watermark level: %d\n", _fifo_conf.wm_level);
                printf("FIFO frame count: %d\n", _fifo_conf.fifo_frame_count);
                printf("FIFO overrun? ");
                printf("%s\n", _fifo_conf.fifo_overrun ? "YES" : "NO");
            }

            /* Set FIFO configuration required value*/
            _fifo_conf.fifo_data_select = BMX_XYZ_AXES;
            _fifo_conf.fifo_mode_select = BMX_MODE_BYPASS;
            _fifo_conf.length = BMX_FIFO_BUFFER_SIZE;
            _fifo_conf.wm_level = BMX_FIFO_WM_LEVEL;
            status += set_fifo_config(&_fifo_conf);
            if (status != 0)
            {
                printf("* FIFO CONFIG FAILED *\n");
            }
            else
            {
                printf("*************************\n");
                printf("* FIFO CONFIG APP VALUE *\n");
                printf("*************************\n");
                printf("FIFO data type: %x\n", _fifo_conf.fifo_data_select);
                printf("FIFO mode: %x\n", _fifo_conf.fifo_mode_select);
                printf("FIFO watermark level: %d\n", _fifo_conf.wm_level);
                printf("FIFO frame count: %d\n", _fifo_conf.fifo_frame_count);
                printf("FIFO overrun? ");
                printf("%s\n", _fifo_conf.fifo_overrun ? "YES" : "NO");
            }

            /* Enable desired interrupts */
            status += set_interrupt_source(BMX_INT_EN_FIFO_WM | BMX_INT_EN_FIFO_FULL);
            if (status != 0)
            {
                printf("* ACTIVATION OF FIFO WM and FULL INTERRUPT FAILED *\n");
            }
            status += get_interrupt_source(&int_src_active);
            if (status != 0)
            {
                printf("* INTERRUPT SOURCES READ FAILED *\n");
            }
            else
            {
                printf("**********************\n");
                printf("* INTERRUPTS ENABLED *\n");
                printf("**********************\n");
                printf("Flat: ");
                printf("%s\n", int_src_active.flat ? "YES" : "NO");
                printf("Orientation: ");
                printf("%s\n", int_src_active.orient ? "YES" : "NO");
                printf("Single tap: ");
                printf("%s\n", int_src_active.s_tap ? "YES" : "NO");
                printf("Double tap: ");
                printf("%s\n", int_src_active.d_tap ? "YES" : "NO");
                printf("Slope X: ");
                printf("%s\n", int_src_active.slope_x ? "YES" : "NO");
                printf("Slope Y: ");
                printf("%s\n", int_src_active.slope_y ? "YES" : "NO");
                printf("Slope Z: ");
                printf("%s\n", int_src_active.slope_z ? "YES" : "NO");
                printf("FIFO wm: ");
                printf("%s\n", int_src_active.fifo_wm ? "YES" : "NO");
                printf("FIFO full: ");
                printf("%s\n", int_src_active.fifo_full ? "YES" : "NO");
                printf("Data ready: ");
                printf("%s\n", int_src_active.data_rdy ? "YES" : "NO");
            }

            /* Get actual power mode */
            status += get_power_mode(&pwr_mode);
            if (status != 0)
            {
                printf("* POWER MODE READ FAILED *\n");
            }
            else
            {
                printf("*********************\n");
                printf("* ACTUAL POWER MODE *\n");
                printf("*********************\n");
                printf("Power mode: ");
                if (pwr_mode == BMX_NORMAL_MODE)
                {
                    printf("NORMAL\n");
                }
                else if (pwr_mode == BMX_DEEP_SUSPEND_MODE)
                {
                    printf("DEEP SUSPEND\n");
                }
                else if (pwr_mode == BMX_LOW_POWER_MODE_1)
                {
                    printf("LOW POWER 2\n");
                }
                else if (pwr_mode == BMX_SUSPEND_MODE)
                {
                    printf("SUSPEND\n");
                }
                else if (pwr_mode == BMX_LOW_POWER_MODE_2)
                {
                    printf("LOW POWER 2\n");
                }
                else if (pwr_mode == BMX_STANDBY_MODE)
                {
                    printf("STANDBY\n");
                }
                else
                {
                    printf("UNKNOWN\n");
                }
            }

            /* Set power mode to NORMAL */
            status += set_power_mode(BMX_NORMAL_MODE);
            if (status != 0)
            {
                printf("* POWER MODE WRITE FAILED *\n");
            }
        }
        else
        {
            return BMX_ERR_DEV_NOT_FOUND;
        }
    }
    else
    {
        return BMX_ERR_DEV_NOT_FOUND;
    }

    if (status == 0)
    {
        printf("\nAccelerometer initialization COMPLETED\n");
        return BMX_OK;
    }
    else
    {
        printf("\nAccelerometer initialization FAILED\n");
        return BMX_ERR_INIT;
    }
};

/*!
 * @details This API reads the chip ID value of the connected device
 *
 * @param[out] chipID_value  : Pointer to data buffer to store the read chip id
 *
 * @return Result of API execution status
 * @return 0 -> Success
 * @return != 0 -> Fail
 */
bmx_error BMX055::get_chipID(uint8_t *chipID_value)
{
    return read_regs(BMX_REG_CHIPID, chipID_value, 1);
}

/*!
 * @details This API read the acc conf registers and store them into a struct
 *
 * @param[out] acc_conf_struct  : Pointer to bmx_acc_conf struct to store the actual conf
 *
 * @return Result of API execution status
 * @return 0 -> Success
 * @return != 0 -> Fail
 */
bmx_error BMX055::get_accel_conf(bmx_acc_conf *acc_conf_struct)
{
    uint8_t reg_data[2];

    if (acc_conf_struct != NULL)
    {
        /* Get the values in the accel configuration registers */
        if (read_regs(BMX_REG_PMU_RANGE, reg_data, 2) == BMX_OK)
        {
            acc_conf_struct->range = BMX_GET_BITS_POS_0(reg_data[0], BMX_RANGE);
            acc_conf_struct->bw = BMX_GET_BITS_POS_0(reg_data[1], BMX_BW);

            if (read_regs(BMX_REG_ACCD_HBW, &reg_data[0], 1) == BMX_OK)
            {
                acc_conf_struct->shadow_dis = BMX_GET_BITS(reg_data[0], BMX_SHADOW_DIS);
                acc_conf_struct->data_high_bw = BMX_GET_BITS(reg_data[0], BMX_DATA_HIGH_BW);
            }
            else
            {
                return BMX_ERR_RD;
            }
        }
        else
        {
            return BMX_ERR_RD;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 * @details This API read an acc conf data struct and write the acc conf registers according to the strut value
 *
 * @param[in] acc_conf_struct  : Pointer to bmx_acc_conf struct which is to be written
 *
 * @return Result of API execution status
 * @return 0 -> Success
 * @return != 0 -> Fail
 */
bmx_error BMX055::set_accel_conf(bmx_acc_conf *acc_conf_struct)
{
    return BMX_OK;
}

/*!
 *  @brief This API is used to get fifo configuration of the sensor.
 */
bmx_error BMX055::get_fifo_config(bmx_fifo_conf *fifo_conf_struct)
{
    uint8_t reg_data;

    if (fifo_conf_struct != NULL)
    {
        if (read_regs(BMX_REG_FIFO_STATUS, &reg_data, 1) == BMX_OK)
        {
            fifo_conf_struct->fifo_frame_count = BMX_GET_BITS_POS_0(reg_data, BMX_FIFO_FRAME_COUNT);
            fifo_conf_struct->fifo_overrun = BMX_GET_BITS(reg_data, BMX_FIFO_OVERRUN);

            if (read_regs(BMX_REG_FIFO_CONFIG_0, &reg_data, 1) == BMX_OK)
            {
                fifo_conf_struct->wm_level = BMX_GET_BITS_POS_0(reg_data, BMX_FIFO_WATER_MARK);
            }
            else
            {
                return BMX_ERR_RD;
            }

            if (read_regs(BMX_REG_FIFO_CONFIG_1, &reg_data, 1) == BMX_OK)
            {
                fifo_conf_struct->fifo_data_select = BMX_GET_BITS_POS_0(reg_data, BMX_FIFO_DATA_SELECT);
                fifo_conf_struct->fifo_mode_select = BMX_GET_BITS(reg_data, BMX_FIFO_MODE_SELECT);
            }
            else
            {
                return BMX_ERR_RD;
            }
        }
        else
        {
            return BMX_ERR_RD;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 *  @brief This API is used to get fifo configuration of the sensor.
 */
bmx_error BMX055::set_fifo_config(bmx_fifo_conf *fifo_conf_struct)
{
    uint8_t reg_data;

    if (fifo_conf_struct != NULL)
    {
        if (read_regs(BMX_REG_FIFO_CONFIG_0, &reg_data, 1) == BMX_OK)
        {
            reg_data = BMX_SET_BITS_POS_0(reg_data, BMX_FIFO_WATER_MARK, _fifo_conf.wm_level);
        }
        else
        {
            return BMX_ERR_RD;
        }

        if (write_regs(BMX_REG_FIFO_CONFIG_0, &reg_data, 1) == BMX_OK)
        {
            if (read_regs(BMX_REG_FIFO_CONFIG_1, &reg_data, 1) == BMX_OK)
            {
                reg_data = BMX_SET_BITS_POS_0(reg_data, BMX_FIFO_DATA_SELECT, _fifo_conf.fifo_data_select);
                reg_data = BMX_SET_BITS(reg_data, BMX_FIFO_MODE_SELECT, _fifo_conf.fifo_mode_select);

                if (write_regs(BMX_REG_FIFO_CONFIG_1, &reg_data, 1) != BMX_OK)
                {
                    return BMX_ERR_WR;
                }
            }
            else
            {
                return BMX_ERR_RD;
            }
        }
        else
        {
            return BMX_ERR_WR;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 * @brief This API is used to enable the various interrupts
 */
bmx_error BMX055::set_interrupt_source(uint32_t int_source_to_en)
{
    uint8_t reg_data[3];

    if (read_regs(BMX_REG_INT_EN_0, reg_data, 3) == BMX_OK)
    {
        reg_data[0] = BMX_SET_BITS_POS_0(reg_data[0], BMX_INT_EN_0, int_source_to_en);
        reg_data[1] = (uint8_t)(BMX_SET_BITS_POS_0(reg_data[1], BMX_INT_EN_1, int_source_to_en) >> 8);
        reg_data[2] = (uint8_t)(BMX_SET_BITS_POS_0(reg_data[2], BMX_INT_EN_2, int_source_to_en) >> 16);
        printf("Desired Interrupt source bytes value: 0x%x, 0x%x, 0x%x\n", reg_data[0], reg_data[1], reg_data[2]);
        if (write_regs(BMX_REG_INT_EN_0, &reg_data[0], 1) != BMX_OK)
        {
            return BMX_ERR_WR;
        }
        if (write_regs(BMX_REG_INT_EN_1, &reg_data[1], 1) != BMX_OK)
        {
            return BMX_ERR_WR;
        }
        if (write_regs(BMX_REG_INT_EN_2, &reg_data[2], 1) != BMX_OK)
        {
            return BMX_ERR_WR;
        }
    }
    else
    {
        return BMX_ERR_RD;
    }

    return BMX_OK;
}

/*!
 * @brief This API is used to get the various interrupts
 * which are enabled in the sensor
 */
bmx_error BMX055::get_interrupt_source(bmx_int_scr *int_en)
{
    uint8_t reg_data[3];
    uint32_t d_dummy_var;

    if (int_en != NULL)
    {
        if (read_regs(BMX_REG_INT_EN_0, reg_data, 3) == BMX_OK)
        {
            d_dummy_var = (uint32_t)((uint32_t)(reg_data[2] << 16) | (uint16_t)(reg_data[1] << 8) | reg_data[0]);
            memcpy(int_en, &d_dummy_var, sizeof(d_dummy_var));
        }
        else
        {
            return BMX_ERR_RD;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 * @brief This API is used to get the power mode of the sensor
 */
bmx_error BMX055::get_power_mode(bmx_power_mode *power_mode)
{
    uint8_t reg_data[2];
    uint8_t power_mode_val;

    if (power_mode != NULL)
    {
        if (read_regs(BMX_REG_PMU_LPW, reg_data, 2) == BMX_OK)
        {
            reg_data[0] = BMX_GET_BITS(reg_data[0], BMX_POWER_MODE_CTRL);
            reg_data[1] = BMX_GET_BITS(reg_data[1], BMX_LOW_POWER_MODE);

            /* Power_mode has the following bit arrangement
             *      {BIT3 : BIT2 : BIT1 : BIT0} =
             *{lowpower_mode : suspend : lowpower_en : deep_suspend}
             */
            power_mode_val = ((uint8_t)(reg_data[1] << 3)) | reg_data[0];

            /* Check if deep suspend bit is enabled. If enabled then assign powermode as deep suspend */
            if (power_mode_val & BMX_DEEP_SUSPEND_MODE)
            {
                /* Device is in deep suspend mode */
                power_mode_val = BMX_DEEP_SUSPEND_MODE;
            }

            *power_mode = (bmx_power_mode)power_mode_val;

            /* Store the actual power mode in class value */
            _power_mode = *power_mode;
        }
        else
        {
            return BMX_ERR_RD;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 * @brief This API is used to set the power mode of the sensor
 */
bmx_error BMX055::set_power_mode(bmx_power_mode power_mode)
{
    bmx_error rslt = BMX_ERR_WR;
    uint8_t reg_data[2];
    uint8_t low_power_mode;

    /* Read the power control registers */
    if (read_regs(BMX_REG_PMU_LPW, reg_data, 2) == BMX_OK)
    {
        switch (power_mode)
        {
        case BMX_NORMAL_MODE:
        case BMX_DEEP_SUSPEND_MODE:
            rslt = set_normal_mode();
            break;

        case BMX_LOW_POWER_MODE_1:
        case BMX_SUSPEND_MODE:
            if ((_power_mode == BMX_LOW_POWER_MODE_2) || (_power_mode == BMX_STANDBY_MODE) ||
                (_power_mode == BMX_DEEP_SUSPEND_MODE))
            {
                rslt = set_normal_mode();
            }

            break;
        case BMX_LOW_POWER_MODE_2:
        case BMX_STANDBY_MODE:
            if ((_power_mode == BMX_LOW_POWER_MODE_1) || (_power_mode == BMX_SUSPEND_MODE) ||
                (_power_mode == BMX_DEEP_SUSPEND_MODE))
            {
                rslt = set_normal_mode();
            }

            break;
        default:
            rslt = BMX_ERR_INVALID_POWERMODE;
            break;
        }

        if (rslt == BMX_OK)
        {
            /* To overcome invalid powermode state a delay is provided. Since
             * 2 registers are accessed to set powermode */
            vTaskDelay(pdMS_TO_TICKS(1));

            low_power_mode = BMX_GET_BITS(power_mode, BMX_POWER_MODE_EXTRACT);
            reg_data[1] = BMX_SET_BITS(reg_data[1], BMX_LOW_POWER_MODE, low_power_mode);

            power_mode = (bmx_power_mode)((uint8_t)power_mode & BMX_POWER_MODE_MASK);
            reg_data[0] = BMX_SET_BITS(reg_data[0], BMX_POWER_MODE_CTRL, power_mode);

            if (write_regs(BMX_REG_PMU_LPW, &reg_data[0], 1) == BMX_OK)
            {
                /* To overcome invalid powermode state a delay of 450us is provided. Since
                 * 2 registers are accessed to set powermode */
                vTaskDelay(pdMS_TO_TICKS(1));

                if (write_regs(BMX_REG_LOW_NOISE, &reg_data[1], 1) == BMX_OK)
                {
                    _power_mode = power_mode;
                }
                else
                {
                    return BMX_ERR_WR;
                }
            }
            else
            {
                return BMX_ERR_WR;
            }
        }
        else
        {
            return rslt;
        }
    }
    else
    {
        return BMX_ERR_RD;
    }

    return BMX_OK;
}

/*!
 * @brief This internal API is used to set the powermode as normal.
 */
bmx_error BMX055::set_normal_mode()
{
    uint8_t reg_data[2];

    /* Read the power control registers */
    if (read_regs(BMX_REG_PMU_LPW, reg_data, 2) == BMX_OK)
    {
        if (_power_mode == BMX_DEEP_SUSPEND_MODE)
        {
            /* Soft reset is performed to return to normal mode from deepsuspend mode.
             * Since no read or write operation is possible in deepsuspend mode */
            if (soft_reset() != BMX_OK)
            {
                return BMX_ERR_WR;
            }
        }
        else
        {
            reg_data[0] = BMX_SET_BIT_VAL_0(reg_data[0], BMX_POWER_MODE_CTRL);
            reg_data[1] = BMX_SET_BIT_VAL_0(reg_data[1], BMX_LOW_POWER_MODE);

            if (write_regs(BMX_REG_PMU_LPW, &reg_data[0], 1) == BMX_OK)
            {
                /* To overcome invalid powermode state a delay is provided. Since
                 * 2 registers are accessed to set powermode */
                vTaskDelay(pdMS_TO_TICKS(1));

                if (write_regs(BMX_REG_LOW_NOISE, &reg_data[1], 1) != BMX_OK)
                {
                    return BMX_ERR_WR;
                }

                vTaskDelay(pdMS_TO_TICKS(1));
            }
            else
            {
                return BMX_ERR_WR;
            }
        }
    }
    else
    {
        return BMX_ERR_RD;
    }

    return BMX_OK;
}

/*!
 * @brief This API is used to perform soft-reset of the sensor
 * where all the registers are reset to their default values
 */
bmx_error BMX055::soft_reset()
{
    uint8_t soft_rst_cmd = BMX_SOFT_RESET_CMD;

    /* Soft-reset is done by writing soft-reset command into the register */
    if (write_regs(BMX_REG_BGW_SOFT_RESET, &soft_rst_cmd, 1) == BMX_OK)
    {
        /* Delay for soft-reset */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    else
    {
        return BMX_ERR_WR;
    }

    return BMX_OK;
}

/*!
 *  @brief This API gets the interrupt status from the registers.
 */
bmx_error BMX055::get_int_status(bmx_int_status *int_status)
{
    uint8_t reg_data[4];

    if (int_status != NULL)
    {
        if (read_regs(BMX_REG_INT_STATUS_0, reg_data, 4) == BMX_OK)
        {
            int_status->int_status_0 = reg_data[0];
            int_status->int_status_1 = reg_data[1];
            int_status->int_status_2 = reg_data[2];
            int_status->int_status_3 = reg_data[3];
        }
        else
        {
            return BMX_ERR_RD;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 *  @brief This API is used to read the FIFO data from FIFO data register
 */
bmx_error BMX055::read_fifo_data()
{
    uint16_t fifo_data_byte_count = 0;

    if (get_fifo_frame_count() == BMX_OK)
    {
        if (_fifo_conf.fifo_data_select == BMX_XYZ_AXES)
        {
            fifo_data_byte_count = (uint16_t)(_fifo_conf.fifo_frame_count * BMX_FIFO_XYZ_AXIS_FRAME_SIZE);
        }
        else
        {
            fifo_data_byte_count = (uint16_t)(_fifo_conf.fifo_frame_count * BMX_FIFO_SINGLE_AXIS_FRAME_SIZE);
        }
    }
    else
    {
        return BMX_ERR_READ_FIFO_CONFIG;
    }

    if (_fifo_conf.length > fifo_data_byte_count)
    {
        /* Handling the case where user requests
         * more data than available in FIFO */
        _fifo_conf.length = fifo_data_byte_count;
    }

    /* Read only the filled bytes in the FIFO Buffer */
    if (read_regs_dma(BMX_REG_FIFO_DATA, _FIFO_data, _fifo_conf.length) != BMX_OK)
    {
        return BMX_ERR_RD_DMA;
    }

    return BMX_OK;
}

bmx_error BMX055::get_fifo_frame_count()
{
    uint8_t reg_data;

    if (read_regs(BMX_REG_FIFO_STATUS, &reg_data, 1) == BMX_OK)
    {
        _fifo_conf.fifo_frame_count = BMX_GET_BITS_POS_0(reg_data, BMX_FIFO_FRAME_COUNT);
        _fifo_conf.fifo_overrun = BMX_GET_BITS(reg_data, BMX_FIFO_OVERRUN);
    }
    else
    {
        return BMX_ERR_RD;
    }

    return BMX_OK;
}

bmx_error BMX055::fifo_extract_frames(sensor_3D_data *accel_data, uint16_t *acc_index)
{
    int8_t rslt = 0;
    uint16_t data_index = 0;
    uint16_t index = 0;

    if ((accel_data != NULL) && (acc_index != NULL))
    {
        for (; (data_index < _fifo_conf.length) && (rslt != BMX_ERR_FIFO_FRAME_EMPTY) && (rslt != BMX_ERR_INVALID_CONFIG);)
        {
            rslt = decode_fifo_frames(&accel_data[index], &data_index);

            index++;

            /* Update the valid frame count */
            *acc_index = index;
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    return BMX_OK;
}

/*!
 *  @brief This internal API is used to unpack the accel data.
 */
bmx_error BMX055::decode_fifo_frames(sensor_3D_data *accel_data, uint16_t *data_index)
{
    bmx_error rslt;

    switch (_fifo_conf.fifo_data_select)
    {
    case BMX_XYZ_AXES:
        if (!((_FIFO_data[*data_index] == 0) && (_FIFO_data[*data_index + 1] == 0) &&
              (_FIFO_data[*data_index + 2] == 0) && (_FIFO_data[*data_index + 3] == 0) &&
              (_FIFO_data[*data_index + 4] == 0) && (_FIFO_data[*data_index + 5] == 0)))
        {
            /* Accel x data */
            accel_data->x = (int16_t)(((uint16_t)_FIFO_data[*data_index + 1] << 8) | _FIFO_data[*data_index]);

            /* Accel y data */
            accel_data->y = (int16_t)(((uint16_t)_FIFO_data[*data_index + 3] << 8) | _FIFO_data[*data_index + 2]);

            /* Accel z data */
            accel_data->z = (int16_t)(((uint16_t)_FIFO_data[*data_index + 5] << 8) | _FIFO_data[*data_index + 4]);

            *data_index += BMX_ACCEL_DATA_XYZ_AXES_LEN;
            rslt = BMX_OK;
        }
        else
        {
            rslt = BMX_ERR_FIFO_FRAME_EMPTY;

            /* Move the data index to the last byte to mark completion */
            *data_index = _fifo_conf.length;
        }

        break;
    case BMX_X_AXIS:
        rslt = fifo_frame_empty_check(&(*data_index));

        if (rslt == BMX_OK)
        {
            /* Accel x data */
            accel_data->x = (int16_t)(((uint16_t)_FIFO_data[*data_index + 1] << 8) | _FIFO_data[*data_index]);

            /* Accel y data */
            accel_data->y = 0;

            /* Accel z data */
            accel_data->z = 0;

            *data_index += BMX_ACCEL_DATA_SINGLE_AXIS_LEN;
            rslt = BMX_OK;
        }

        break;
    case BMX_Y_AXIS:
        rslt = fifo_frame_empty_check(&(*data_index));

        if (rslt == BMX_OK)
        {
            /* Accel x data */
            accel_data->x = 0;

            /* Accel y data */
            accel_data->y = (int16_t)(((uint16_t)_FIFO_data[*data_index + 1] << 8) | _FIFO_data[*data_index]);

            /* Accel z data */
            accel_data->z = 0;

            *data_index += BMX_ACCEL_DATA_SINGLE_AXIS_LEN;
            rslt = BMX_OK;
        }

        break;
    case BMX_Z_AXIS:
        rslt = fifo_frame_empty_check(&(*data_index));

        if (rslt == BMX_OK)
        {
            /* Accel x data */
            accel_data->x = 0;

            /* Accel y data */
            accel_data->y = 0;

            /* Accel z data */
            accel_data->z = (int16_t)(((uint16_t)_FIFO_data[*data_index + 1] << 8) | _FIFO_data[*data_index]);

            *data_index += BMX_ACCEL_DATA_SINGLE_AXIS_LEN;
            rslt = BMX_OK;
        }

        break;
    default:
        accel_data->x = 0;
        accel_data->y = 0;
        accel_data->z = 0;
        rslt = BMX_ERR_INVALID_CONFIG;
    }

    return rslt;
}

/*!
 *  @brief This internal API is check if the fifo frame is empty.
 */
bmx_error BMX055::fifo_frame_empty_check(uint16_t *data_index)
{
    if ((_FIFO_data[*data_index] == 0) && (_FIFO_data[*data_index + 1] == 0) && (_FIFO_data[*data_index + 2] == 0) &&
        (_FIFO_data[*data_index + 3] == 0))
    {
        /* Move the data index to the last byte to mark completion */
        *data_index = _fifo_conf.length;

        return BMX_ERR_FIFO_FRAME_EMPTY;
    }
    else
    {
        return BMX_OK;
    }
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
bmx_error BMX055::read_regs(uint8_t reg_addr, uint8_t *data_rd, uint32_t length)
{
    esp_err_t ret = ESP_FAIL;

    uint8_t dummy_buffer[length];
    memset(dummy_buffer, 0, sizeof(dummy_buffer[0] * length));

    spi_transaction_t t =
        {
            .addr = (uint64_t)(reg_addr | BMX_SPI_RD_MASK),
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
        return BMX_ERR_NULL_POINTER;
    }

    if (ret == 0)
    {
        return BMX_OK;
    }
    else
    {
        return BMX_ERR_RD;
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
bmx_error BMX055::write_regs(uint8_t reg_addr, uint8_t *data_wr, uint32_t length)
{
    esp_err_t ret = ESP_FAIL;

    spi_transaction_t t =
        {
            .addr = (uint64_t)(reg_addr & BMX_SPI_WR_MASK),
            .length = 8 * length,
            .tx_buffer = data_wr // Buffer of data to write
        };

    if ((length > 0) && (data_wr != NULL))
    {
        if (xSemaphoreTake(_xSpiSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ret = spi_device_polling_transmit(_spi, &t);
            xSemaphoreGive(_xSpiSemaphore);
        }
    }
    else
    {
        return BMX_ERR_NULL_POINTER;
    }

    if (ret == 0)
    {
        return BMX_OK;
    }
    else
    {
        return BMX_ERR_WR;
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
bmx_error BMX055::read_regs_dma(uint8_t reg_addr, uint8_t *data_rd, uint32_t length)
{
    esp_err_t ret = ESP_FAIL;

    uint8_t dummy_buffer[length];
    memset(dummy_buffer, 0, sizeof(dummy_buffer[0] * length));

    spi_transaction_t t =
        {
            .addr = (uint64_t)(reg_addr | BMX_SPI_RD_MASK),
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
        return BMX_ERR_NULL_POINTER;
    }

    if (ret == 0)
    {
        return BMX_OK;
    }
    else
    {
        return BMX_ERR_RD;
    }
}