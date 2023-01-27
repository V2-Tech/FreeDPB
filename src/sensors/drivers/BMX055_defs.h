#ifndef INC_BMX055_DEFS_H
#define INC_BMX055_DEFS_H

/*******************************************************/
/*! @name Application settings                       */
/*******************************************************/
/*! Watermark level for FIFO */
#define BMX_FIFO_WM_LEVEL UINT8_C(16)

/*! Number of Accel frames inside FIFO */
#define BMX_FIFO_DATA_FRAME_COUNT UINT8_C(32)

/*******************************************************/
/*! @name Registers address                            */
/*******************************************************/

/*! @name Registers list */
#define BMX_REG_CHIPID UINT8_C(0x00)
#define BMX_REG_ACCD_X_LSB UINT8_C(0x02)
#define BMX_REG_ACCD_X_MSB UINT8_C(0x03)
#define BMX_REG_ACCD_Y_LSB UINT8_C(0x04)
#define BMX_REG_ACCD_Y_MSB UINT8_C(0x05)
#define BMX_REG_ACCD_Z_LSB UINT8_C(0x06)
#define BMX_REG_ACCD_Z_MSB UINT8_C(0x07)
#define BMX_REG_ACCD_TEMP UINT8_C(0x08)
#define BMX_REG_INT_STATUS_0 UINT8_C(0x09)
#define BMX_REG_INT_STATUS_1 UINT8_C(0x0A)
#define BMX_REG_INT_STATUS_2 UINT8_C(0x0B)
#define BMX_REG_INT_STATUS_3 UINT8_C(0x0C)
#define BMX_REG_FIFO_STATUS UINT8_C(0x0E)
#define BMX_REG_PMU_RANGE UINT8_C(0x0F)
#define BMX_REG_PMU_BW UINT8_C(0x10)
#define BMX_REG_PMU_LPW UINT8_C(0x11)
#define BMX_REG_LOW_NOISE UINT8_C(0x12)
#define BMX_REG_ACCD_HBW UINT8_C(0x13)
#define BMX_REG_BGW_SOFT_RESET UINT8_C(0x14)
#define BMX_REG_INT_EN_0 UINT8_C(0x16)
#define BMX_REG_INT_EN_1 UINT8_C(0x17)
#define BMX_REG_INT_EN_2 UINT8_C(0x18)
#define BMX_REG_INT_MAP_0 UINT8_C(0x19)
#define BMX_REG_INT_MAP_1 UINT8_C(0x1A)
#define BMX_REG_INT_MAP_2 UINT8_C(0x1B)
#define BMX_REG_INT_SRC UINT8_C(0x1E)
#define BMX_REG_INT_OUT_CTRL UINT8_C(0x20)
#define BMX_REG_INT_RST_LATCH UINT8_C(0x21)
#define BMX_REG_INT_0 UINT8_C(0x22)
#define BMX_REG_INT_1 UINT8_C(0x23)
#define BMX_REG_INT_2 UINT8_C(0x24)
#define BMX_REG_INT_3 UINT8_C(0x25)
#define BMX_REG_INT_4 UINT8_C(0x26)
#define BMX_REG_INT_5 UINT8_C(0x27)
#define BMX_REG_INT_6 UINT8_C(0x28)
#define BMX_REG_INT_7 UINT8_C(0x29)
#define BMX_REG_INT_8 UINT8_C(0x2A)
#define BMX_REG_INT_9 UINT8_C(0x2B)
#define BMX_REG_INT_A UINT8_C(0x2C)
#define BMX_REG_INT_B UINT8_C(0x2D)
#define BMX_REG_INT_C UINT8_C(0x2E)
#define BMX_REG_INT_D UINT8_C(0x2F)
#define BMX_REG_FIFO_CONFIG_0 UINT8_C(0x30)
#define BMX_REG_PMU_SELF_TEST UINT8_C(0x32)
#define BMX_REG_TRIM_NVM_CTRL UINT8_C(0x33)
#define BMX_REG_BGW_SPI3_WDT UINT8_C(0x34)
#define BMX_REG_OFC_CTRL UINT8_C(0x36)
#define BMX_REG_OFC_SETTING UINT8_C(0x37)
#define BMX_REG_OFC_OFFSET_X UINT8_C(0x38)
#define BMX_REG_OFC_OFFSET_Y UINT8_C(0x39)
#define BMX_REG_OFC_OFFSET_Z UINT8_C(0x3A)
#define BMX_REG_TRIM_GP0 UINT8_C(0x3B)
#define BMX_REG_TRIM_GP1 UINT8_C(0x3C)
#define BMX_REG_FIFO_CONFIG_1 UINT8_C(0x3E)
#define BMX_REG_FIFO_DATA UINT8_C(0x3F)

/*! @name Read and write mask */
#define BMX_SPI_WR_MASK UINT8_C(0x7F)
#define BMX_SPI_RD_MASK UINT8_C(0x80)

/*! @name Register status */
#define BMX_DISABLE UINT8_C(0x00)
#define BMX_ENABLE UINT8_C(0x01)
#define BMX_TRUE UINT8_C(1)
#define BMX_FALSE UINT8_C(0)

/*******************************************************/
/*! @name Values                                       */
/*******************************************************/

/*! @name BMI055 chip ID value */
#define BMX_CHIPID UINT8_C(0xFA)

/*! @name Delay definition */
#define BMX_DELAY_SOFT_RESET UINT16_C(1800)
/*********************************************************************/

/*! @name BASIC ACCEL SETTINGS                                       */
/*********************************************************************/

/*! @name BMX Range settings  */
#define BMX_ACC_RANGE_2G UINT8_C(0x03)
#define BMX_ACC_RANGE_4G UINT8_C(0x05)
#define BMX_ACC_RANGE_8G UINT8_C(0x08)
#define BMX_ACC_RANGE_16G UINT8_C(0x0C)

/*! @name BMX bandwidth settings  */
#define BMX_ACC_BW_7_81_HZ UINT8_C(0x08)
#define BMX_ACC_BW_15_63_HZ UINT8_C(0x09)
#define BMX_ACC_BW_31_25_HZ UINT8_C(0x0A)
#define BMX_ACC_BW_62_5_HZ UINT8_C(0x0B)
#define BMX_ACC_BW_125_HZ UINT8_C(0x0C)
#define BMX_ACC_BW_250_HZ UINT8_C(0x0D)
#define BMX_ACC_BW_500_HZ UINT8_C(0x0E)
#define BMX_ACC_BW_1000_HZ UINT8_C(0x0F)

/*! @name BMX Shadow enable/disable settings  */
#define BMX_SHADOWING_ENABLE UINT8_C(0x00)
#define BMX_SHADOWING_DISABLE UINT8_C(0x01)

/*! @name Filtered/un-filtered data selection in data_high_bw */
#define BMX_FILTERED_DATA UINT8_C(0x00)
#define BMX_UN_FILTERED_DATA UINT8_C(0x01)

/*! @name BMX DATA READ SETTINGS */
#define BMX_ACCEL_DATA_ONLY UINT8_C(0x00)
#define BMX_ACCEL_DATA_TEMPERATURE UINT8_C(0x01)

/*! @name BMX Data size */
#define BMX_ACCEL_DATA_XYZ_AXES_LEN UINT8_C(0x06)
#define BMX_ACCEL_DATA_SINGLE_AXIS_LEN UINT8_C(0x02)

/*! @name BMX Fifo frame size */
#define BMX_FIFO_XYZ_AXIS_FRAME_SIZE UINT8_C(6)
#define BMX_FIFO_SINGLE_AXIS_FRAME_SIZE UINT8_C(2)
#define BMX_FIFO_BUFFER_SIZE UINT8_C(192)

/*********************************************************************/
/*! @name INTERRUPT SETTINGS                                         */
/*********************************************************************/
/*! @name Interrupt pin conf */
#define BMX_OPEN_DRAIN UINT8_C(0x01)
#define BMX_PUSH_PULL UINT8_C(0x00)

#define BMX_ACTIVE_LOW UINT8_C(0x00)
#define BMX_ACTIVE_HIGH UINT8_C(0x01)

#define BMX_FILTERED_DATA_SRC UINT8_C(0x00)
#define BMX_UN_FILTERED_DATA_SRC UINT8_C(0x01)

#define BMX_RESET_LATCHED_INT UINT8_C(0x01)
#define BMX_LATCHED_INT_ACTIVE UINT8_C(0x00)

/*! @name Interrupts enabled */
#define BMX_INT_EN_SLOPE_X_AXIS UINT32_C(0x000001)
#define BMX_INT_EN_SLOPE_Y_AXIS UINT32_C(0x000002)
#define BMX_INT_EN_SLOPE_Z_AXIS UINT32_C(0x000004)
#define BMX_INT_EN_DOUBLE_TAP UINT32_C(0x000010)
#define BMX_INT_EN_SINGLE_TAP UINT32_C(0x000020)
#define BMX_INT_EN_ORIENTATION UINT32_C(0x000040)
#define BMX_INT_EN_FLAT UINT32_C(0x000080)
#define BMX_INT_EN_HIGH_G_X_AXIS UINT32_C(0x000100)
#define BMX_INT_EN_HIGH_G_Y_AXIS UINT32_C(0x000200)
#define BMX_INT_EN_HIGH_G_Z_AXIS UINT32_C(0x000400)
#define BMX_INT_EN_LOW_G UINT32_C(0x000800)
#define BMX_INT_EN_DATA_READY UINT32_C(0x001000)
#define BMX_INT_EN_FIFO_FULL UINT32_C(0x002000)
#define BMX_INT_EN_FIFO_WM UINT32_C(0x004000)
#define BMX_INT_EN_SLOW_NO_MOTION_X_AXIS UINT32_C(0x010000)
#define BMX_INT_EN_SLOW_NO_MOTION_Y_AXIS UINT32_C(0x020000)
#define BMX_INT_EN_SLOW_NO_MOTION_Z_AXIS UINT32_C(0x040000)
#define BMX_INT_EN_SLOW_NO_MOTION_SEL UINT32_C(0x080000)

/*! @name Interrupts asserted */
#define BMX_INT_0_ASSERTED_LOW_G UINT8_C(0x01)
#define BMX_INT_0_ASSERTED_HIGH_G UINT8_C(0x02)
#define BMX_INT_0_ASSERTED_SLOPE UINT8_C(0x04)
#define BMX_INT_0_ASSERTED_SLOW_NO_MOTION UINT8_C(0x08)
#define BMX_INT_0_ASSERTED_DOUBLE_TAP UINT8_C(0x10)
#define BMX_INT_0_ASSERTED_SINGLE_TAP UINT8_C(0x20)
#define BMX_INT_0_ASSERTED_ORIENTATION UINT8_C(0x40)
#define BMX_INT_0_ASSERTED_FLAT UINT8_C(0x80)
#define BMX_INT_1_ASSERTED_FIFO_FULL UINT8_C(0x20)
#define BMX_INT_1_ASSERTED_FIFO_WM UINT8_C(0x40)
#define BMX_INT_1_ASSERTED_DATA_READY UINT8_C(0x80)
#define BMX_INT_2_ASSERTED_SLOPE_X UINT8_C(0x01)
#define BMX_INT_2_ASSERTED_SLOPE_Y UINT8_C(0x02)
#define BMX_INT_2_ASSERTED_SLOPE_Z UINT8_C(0x04)
#define BMX_INT_2_ASSERTED_SLOPE_SIGN UINT8_C(0x08)
#define BMX_INT_2_ASSERTED_TAP_X UINT8_C(0x10)
#define BMX_INT_2_ASSERTED_TAP_Y UINT8_C(0x20)
#define BMX_INT_2_ASSERTED_TAP_Z UINT8_C(0x40)
#define BMX_INT_2_ASSERTED_TAP_SIGN UINT8_C(0x80)
#define BMX_INT_3_ASSERTED_HIGH_X UINT8_C(0x01)
#define BMX_INT_3_ASSERTED_HIGH_Y UINT8_C(0x02)
#define BMX_INT_3_ASSERTED_HIGH_Z UINT8_C(0x04)
#define BMX_INT_3_ASSERTED_HIGH_SIGN UINT8_C(0x08)
#define BMX_INT_3_ASSERTED_FLAT_POSITION UINT8_C(0x80)

/* Orientation interrupt axis */
#define BMX_INT_3_ASSERTED_ORIENTATION_Z UINT8_C(0x40)
#define BMX_INT_3_ASSERTED_ORIENTATION_POTRAIT_UPRIGHT UINT8_C(0x00)
#define BMX_INT_3_ASSERTED_ORIENTATION_POTRAIT_UPSIDE_DOWN UINT8_C(0x01)
#define BMX_INT_3_ASSERTED_ORIENTATION_LANDSCAPE_LEFT UINT8_C(0x02)
#define BMX_INT_3_ASSERTED_ORIENTATION_LANDSCAPE_RIGHT UINT8_C(0x03)

/*! @name BMX interrupt map/unmap macros */
#define BMX_INT_MAP UINT8_C(0x01)
#define BMX_INT_UNMAP UINT8_C(0x00)

/*! @name BMX interrupt map macors */
#define BMX_INT1_MAP_LOW_G UINT32_C(0x000001)
#define BMX_INT1_MAP_HIGH_G UINT32_C(0x000002)
#define BMX_INT1_MAP_SLOPE UINT32_C(0x000004)
#define BMX_INT1_MAP_SLOW_NO_MOTION UINT32_C(0x000008)
#define BMX_INT1_MAP_DOUBLE_TAP UINT32_C(0x000010)
#define BMX_INT1_MAP_SINGLE_TAP UINT32_C(0x000020)
#define BMX_INT1_MAP_ORIENTATION UINT32_C(0x000040)
#define BMX_INT1_MAP_FLAT UINT32_C(0x000080)
#define BMX_INT1_MAP_DATA_READY UINT32_C(0x000100)
#define BMX_INT1_MAP_FIFO_WM UINT32_C(0x000200)
#define BMX_INT1_MAP_FIFO_FULL UINT32_C(0x000400)
#define BMX_INT2_MAP_FIFO_FULL UINT32_C(0x002000)
#define BMX_INT2_MAP_FIFO_WM UINT32_C(0x004000)
#define BMX_INT2_MAP_DATA_READY UINT32_C(0x008000)
#define BMX_INT2_MAP_LOW_G UINT32_C(0x010000)
#define BMX_INT2_MAP_HIGH_G UINT32_C(0x020000)
#define BMX_INT2_MAP_SLOPE UINT32_C(0x040000)
#define BMX_INT2_MAP_SLOW_NO_MOTION UINT32_C(0x080000)
#define BMX_INT2_MAP_DOUBLE_TAP UINT32_C(0x100000)
#define BMX_INT2_MAP_SINGLE_TAP UINT32_C(0x200000)
#define BMX_INT2_MAP_ORIENTATION UINT32_C(0x400000)
#define BMX_INT2_MAP_FLAT UINT32_C(0x800000)

#define BMX_INT1_MAP_ALL UINT32_C(0x0007FF)
#define BMX_INT2_MAP_ALL UINT32_C(0xFFE000)
#define BMX_INT_MAP_ALL UINT32_C(0xFFE7FF)

/*! @name BMX interrupt filtered/unfiltered macros */
#define BMX_INT_FILTERED_DATA UINT8_C(0x00)
#define BMX_INT_UNFILTERED_DATA UINT8_C(0x01)

/*! @name BMX interrupt source select macros */
#define BMX_INT_SRC_LOW_G UINT8_C(0x01)
#define BMX_INT_SRC_HIGH_G UINT8_C(0x02)
#define BMX_INT_SRC_SLOPE UINT8_C(0x04)
#define BMX_INT_SRC_SLOW_NO_MOTION UINT8_C(0x08)
#define BMX_INT_SRC_TAP UINT8_C(0x10)
#define BMX_INT_SRC_DATA UINT8_C(0x20)

/********************************************************/
/*! @name Macro to SET and GET BITS of registers        */
/********************************************************/
/* Bits GET and SET functions */
#define BMX_SET_BITS(reg_data, bitname, data) ((reg_data & ~(bitname##_MSK)) | ((data << bitname##_POS) & bitname##_MSK))

#define BMX_GET_BITS(reg_data, bitname) ((reg_data & (bitname##_MSK)) >> (bitname##_POS))

#define BMX_SET_BITS_POS_0(reg_data, bitname, data) ((reg_data & ~(bitname##_MSK)) | (data & bitname##_MSK))

#define BMX_GET_BITS_POS_0(reg_data, bitname) (reg_data & (bitname##_MSK))

#define BMX_SET_BIT_VAL_0(reg_data, bitname) (reg_data & ~(bitname##_MSK))

/* Accelerometer confgiurations MASK */
#define BMX_RANGE_MSK UINT8_C(0x0F)
#define BMX_BW_MSK UINT8_C(0x1F)
#define BMX_SLEEP_DUR_MSK UINT8_C(0x1E)
#define BMX_SLEEP_DUR_POS UINT8_C(0x01)
#define BMX_SLEEP_TIMER_MODE_MSK UINT8_C(0x20)
#define BMX_SLEEP_TIMER_MODE_POS UINT8_C(0x05)
#define BMX_SHADOW_DIS_MSK UINT8_C(0x40)
#define BMX_SHADOW_DIS_POS UINT8_C(0x06)
#define BMX_DATA_HIGH_BW_MSK UINT8_C(0x80)
#define BMX_DATA_HIGH_BW_POS UINT8_C(0x07)
#define BMX_NEW_DATA_MSK UINT8_C(0x01)

/* Fifo configurations MASK */
#define BMX_FIFO_MODE_SELECT_MSK UINT8_C(0xC0)
#define BMX_FIFO_MODE_SELECT_POS UINT8_C(6)
#define BMX_FIFO_DATA_SELECT_MSK UINT8_C(0x03)
#define BMX_FIFO_WATER_MARK_MSK UINT8_C(0x3F)
#define BMX_FIFO_FRAME_COUNT_MSK UINT8_C(0x7F)
#define BMX_FIFO_OVERRUN_MSK UINT8_C(0x80)
#define BMX_FIFO_OVERRUN_POS UINT8_C(7)

/* Interrupt sources configurations MASK*/
#define BMX_INT_EN_0_MSK UINT32_C(0x0000FF)
#define BMX_INT_EN_1_MSK UINT32_C(0x00FF00)
#define BMX_INT_EN_2_MSK UINT32_C(0xFF0000)

/* Powermode */
#define BMX_POWER_MODE_CTRL_MSK UINT8_C(0xE0)
#define BMX_POWER_MODE_CTRL_POS UINT8_C(0x05)
#define BMX_LOW_POWER_MODE_MSK UINT8_C(0x40)
#define BMX_LOW_POWER_MODE_POS UINT8_C(0x06)
#define BMX_POWER_MODE_EXTRACT_MSK UINT8_C(0x08)
#define BMX_POWER_MODE_EXTRACT_POS UINT8_C(0x03)

/**********************************************************/
/*!  @name         COMMON MACROS                          */
/**********************************************************/

#define BMX_POWER_MODE_MASK UINT8_C(0x07)
#define BMX_SOFT_RESET_CMD UINT8_C(0xB6)
#define BMX_NEW_DATA_XYZ UINT8_C(0x07)
#define BMX_RES_MAX_VAL UINT8_C(2047)
#define BMX_RES_NEG_VAL UINT8_C(4096)
#define BMX_RES_SHIFT_VAL UINT8_C(4)

/*!  Macro to SET and GET BITS of a register*/
#define BMX_SET_LOW_BYTE UINT16_C(0x00FF)
#define BMX_SET_HIGH_BYTE UINT16_C(0xFF00)

/*! Macro to convert milliseconds to microseconds */
#define BMX_MS_TO_US(X) (X * 1000)

/*! Absolute value */
#ifndef ABS
#define ABS(a) ((a) > 0 ? (a) : -(a))
#endif

/***********************************************************************/
/*!  @name         Enum Declarations                                  */
/***********************************************************************/

/*!
 * @brief Enum to function's return errors
 */
enum bmx_error_e
{
    BMX_OK,
    BMX_ERR_RD,
    BMX_ERR_WR,
    BMX_ERR_INIT,
    BMX_ERR_DEV_NOT_FOUND,
    BMX_ERR_NULL_POINTER,
    BMX_ERR_INVALID_POWERMODE,
    BMX_ERR_READ_FIFO_CONFIG,
    BMX_ERR_READ_FIFO_DATA,
    BMX_ERR_RD_DMA,
    BMX_ERR_FIFO_FRAME_EMPTY,
    BMX_ERR_INVALID_CONFIG,
    BMX_ERR_NO_NEW_AVAILABLE,
};

/*!
 * @brief Enum to define fifo modes
 */
enum bmx_fifo_mode_e
{
    /*! Bypass mode */
    BMX_MODE_BYPASS,
    /*! Fifo mode */
    BMX_MODE_FIFO,
    /*! Stream mode */
    BMX_MODE_STREAM
};

/*!
 * @brief Enum to define fifo select data
 */
enum bmx_fifo_data_select_e
{
    /*! X, Y and Z-axis selection */
    BMX_XYZ_AXES,
    /*! X-axis selection */
    BMX_X_AXIS,
    /*! Y-axis selection */
    BMX_Y_AXIS,
    /*! Z-axis selection */
    BMX_Z_AXIS
};

/*!
 * @brief Enum to define device power modes
 */
enum bmx_power_mode_e
{
    BMX_NORMAL_MODE = 0x00,
    BMX_DEEP_SUSPEND_MODE = 0x01,
    BMX_LOW_POWER_MODE_1 = 0x02,
    BMX_SUSPEND_MODE = 0x04,
    BMX_LOW_POWER_MODE_2 = 0x0A,
    BMX_STANDBY_MODE = 0x0C
};

/************************************************/
/*!  @name        Struct Declarations           */
/************************************************/

/*!
 * @brief BMX accel configuration structure
 */
struct bmx_acc_conf_t
{
    /*! Accel g-range selection
     *  Assignable macros :
     *   - BMX_ACC_RANGE_2G       - BMX_ACC_RANGE_4G
     *   - BMX_ACC_RANGE_8G       - BMX_ACC_RANGE_16G
     */
    uint8_t range;

    /*! Data filter bandwidth setting
     *  Assignable macros :
     *  - BMX_ACC_BW_7_81_HZ       - BMX_ACC_BW_125_HZ
     *  - BMX_ACC_BW_15_63_HZ      - BMX_ACC_BW_250_HZ
     *  - BMX_ACC_BW_31_25_HZ      - BMX_ACC_BW_500_HZ
     *  - BMX_ACC_BW_62_5_HZ       - BMX_ACC_BW_1000_HZ
     */
    uint8_t bw;

    /*! Data register configurations
     *  Assignable macros :
     *  - BMX_SHADOWING_ENABLE
     *  - BMX_SHADOWING_DISABLE
     */
    uint8_t shadow_dis;

    /*! Data register configurations
     *  Assignable macros :
     *  - BMX_FILTERED_DATA
     *  - BMX_UN_FILTERED_DATA
     */
    uint8_t data_high_bw;
};

/*!
 * @brief BMX Fifo frame configuration
 */
struct bmx_fifo_conf_t
{
    /*! While calling the API  "bma2_get_fifo_data" , length stores
     *  number of bytes in FIFO to be read (specified by user as input)
     *  and after execution of the API ,number of FIFO data bytes
     *  available is provided as an output to user
     */
    uint16_t length;

    /*! Selection of axis for data
     * Assignable macros :
     * - BMX_XYZ_AXES
     * - BMX_X_AXIS,
     * - BMX_Y_AXIS,
     * - BMX_Z_AXIS
     */
    uint8_t fifo_data_select;

    /*! FIFO mode selection
     * Assignable macros :
     * - BMX_MODE_BYPASS
     * - BMX_MODE_FIFO
     * - BMX_MODE_STREAM
     */
    uint8_t fifo_mode_select;

    /*! Water-mark level for FIFO */
    uint16_t wm_level;

    /*! Value of fifo frame count */
    uint8_t fifo_frame_count;

    /*! Flag to check Fifo overrun */
    uint8_t fifo_overrun;
};

/*!
 * @brief BMX sensor data
 */
struct sensor_3D_data_t
{
    /*! X-axis sensor data */
    int16_t x;

    /*! Y-axis sensor data */
    int16_t y;

    /*! Z-axis sensor data */
    int16_t z;
};

/*!
 * @brief BMX interrupt status
 */
struct bmx_int_status_t
{
    /*! Variable that holds the interrupt status
     * of flat, orientation, single tap, double tap, slow/no-motion, slope,
     * high-g and low-g interrupt.
     */
    uint8_t int_status_0;

    /*! Variable that holds the interrupt status
     * of fifo watermark, fifo full and data ready interrupt.
     */
    uint8_t int_status_1;

    /*! Variable that holds the interrupt status
     * of trigger axis and direction(sign) of the interrupt for tap and slope interrupt.
     */
    uint8_t int_status_2;

    /*! Variable that holds the interrupt status
     * of flat position, orientation and high-g trigger axis and direction(sign) of high-g interrupt.
     */
    uint8_t int_status_3;
};

/*!
 * @brief BMX interrupt sources
 */
union bmx_int_scr_u
{
    /* Bit inversion because ESP32 work with msb architecture. In this way it's easier to manage
    memcpy conversions */

    uint32_t allBits;
    struct
    {
        unsigned slope_x : 1;
        unsigned slope_y : 1;
        unsigned slope_z : 1;
        unsigned res1 : 1;
        unsigned d_tap : 1;
        unsigned s_tap : 1;
        unsigned orient : 1;
        unsigned flat : 1;

        unsigned high_g_x : 1;
        unsigned high_g_y : 1;
        unsigned high_g_z : 1;
        unsigned low_g : 1;
        unsigned data_rdy : 1;
        unsigned fifo_full : 1;
        unsigned fifo_wm : 1;
        unsigned res2 : 1;

        unsigned slow_m_x : 1;
        unsigned slow_m_y : 1;
        unsigned slow_m_z : 1;
        unsigned slow_m_sel : 1;
        unsigned res6 : 1;
        unsigned res5 : 1;
        unsigned res4 : 1;
        unsigned res3 : 1;

        unsigned nd8 : 1;
        unsigned nd7 : 1;
        unsigned nd6 : 1;
        unsigned nd5 : 1;
        unsigned nd4 : 1;
        unsigned nd3 : 1;
        unsigned nd2 : 1;
        unsigned nd1 : 1;
    };
};
#endif