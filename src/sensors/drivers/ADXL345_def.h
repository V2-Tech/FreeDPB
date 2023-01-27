#ifndef INC_ADXL345_DEF_H
#define INC_ADXL345_DEF_H

/*******************************************************/
/*! @name Registers address                            */
/*******************************************************/

/*! @name Registers list */
#define ADXL345_REG_CHIPID (0x00)          ///< Device ID
#define ADXL345_REG_THRESH_TAP (0x1D)     ///< Tap threshold
#define ADXL345_REG_OFSX (0x1E)           ///< X-axis offset
#define ADXL345_REG_OFSY (0x1F)           ///< Y-axis offset
#define ADXL345_REG_OFSZ (0x20)           ///< Z-axis offset
#define ADXL345_REG_DUR (0x21)            ///< Tap duration
#define ADXL345_REG_LATENT (0x22)         ///< Tap latency
#define ADXL345_REG_WINDOW (0x23)         ///< Tap window
#define ADXL345_REG_THRESH_ACT (0x24)     ///< Activity threshold
#define ADXL345_REG_THRESH_INACT (0x25)   ///< Inactivity threshold
#define ADXL345_REG_TIME_INACT (0x26)     ///< Inactivity time
#define ADXL345_REG_ACT_INACT_CTL (0x27)  ///< Axis enable control for activity and inactivity detection
#define ADXL345_REG_THRESH_FF (0x28)      ///< Free-fall threshold
#define ADXL345_REG_TIME_FF (0x29)        ///< Free-fall time
#define ADXL345_REG_TAP_AXES (0x2A)       ///< Axis control for single/double tap
#define ADXL345_REG_ACT_TAP_STATUS (0x2B) ///< Source for single/double tap
#define ADXL345_REG_BW_RATE (0x2C)        ///< Data rate and power mode control
#define ADXL345_REG_POWER_CTL (0x2D)      ///< Power-saving features control
#define ADXL345_REG_INT_ENABLE (0x2E)     ///< Interrupt enable control
#define ADXL345_REG_INT_MAP (0x2F)        ///< Interrupt mapping control
#define ADXL345_REG_INT_SOURCE (0x30)     ///< Source of interrupts
#define ADXL345_REG_DATA_FORMAT (0x31)    ///< Data format control
#define ADXL345_REG_DATAX0 (0x32)         ///< X-axis data 0
#define ADXL345_REG_DATAX1 (0x33)         ///< X-axis data 1
#define ADXL345_REG_DATAY0 (0x34)         ///< Y-axis data 0
#define ADXL345_REG_DATAY1 (0x35)         ///< Y-axis data 1
#define ADXL345_REG_DATAZ0 (0x36)         ///< Z-axis data 0
#define ADXL345_REG_DATAZ1 (0x37)         ///< Z-axis data 1
#define ADXL345_REG_FIFO_CTL (0x38)       ///< FIFO control
#define ADXL345_REG_FIFO_STATUS (0x39)    ///< FIFO status

/*! @name Read and write mask */
#define ADXL345_SPI_WR_MASK UINT8_C(0x3F)
#define ADXL345_SPI_RD_MASK UINT8_C(0x80)
#define ADXL345_SPI_MULTI_MASK UINT8_C(0x40)

/*! @name Register status */
#define ADXL345_DISABLE UINT8_C(0x00)
#define ADXL345_ENABLE UINT8_C(0x01)
#define ADXL345_TRUE UINT8_C(1)
#define ADXL345_FALSE UINT8_C(0)

/*******************************************************/
/*! @name Values                                       */
/*******************************************************/
#define ADXL345_CHIPID UINT8_C(0xE5)
#define ADXL345_MG2G_MULTIPLIER (0.004) ///< 4mg per lsb

/***********************************************************************/
/*!  @name         Enum Declarations                                  */
/***********************************************************************/
/**
 * @brief Used with register 0x2C (ADXL345_REG_BW_RATE) to set bandwidth
 */
typedef enum
{
    ADXL345_DATARATE_3200_HZ = 0b1111, ///< 1600Hz Bandwidth
    ADXL345_DATARATE_1600_HZ = 0b1110, ///<  800Hz Bandwidth
    ADXL345_DATARATE_800_HZ = 0b1101,  ///<  400Hz Bandwidth
    ADXL345_DATARATE_400_HZ = 0b1100,  ///<  200Hz Bandwidth
    ADXL345_DATARATE_200_HZ = 0b1011,  ///<  100Hz Bandwidth
    ADXL345_DATARATE_100_HZ = 0b1010,  ///<   50Hz Bandwidth
    ADXL345_DATARATE_50_HZ = 0b1001,   ///<   25Hz Bandwidth
    ADXL345_DATARATE_25_HZ = 0b1000,   ///< 12.5Hz Bandwidth
    ADXL345_DATARATE_12_5_HZ = 0b0111, ///< 6.25Hz Bandwidth
    ADXL345_DATARATE_6_25HZ = 0b0110,  ///< 3.13Hz Bandwidth
    ADXL345_DATARATE_3_13_HZ = 0b0101, ///< 1.56Hz Bandwidth
    ADXL345_DATARATE_1_56_HZ = 0b0100, ///< 0.78Hz Bandwidth
    ADXL345_DATARATE_0_78_HZ = 0b0011, ///< 0.39Hz Bandwidth
    ADXL345_DATARATE_0_39_HZ = 0b0010, ///< 0.20Hz Bandwidth
    ADXL345_DATARATE_0_20_HZ = 0b0001, ///< 0.10Hz Bandwidth
    ADXL345_DATARATE_0_10_HZ = 0b0000  ///< 0.05Hz Bandwidth (default value)
} dataRate_e;

/**
 * @brief  Used with register 0x31 (ADXL345_REG_DATA_FORMAT) to set g range
 */
typedef enum
{
    ADXL345_RANGE_16_G = 0b11, ///< +/- 16g
    ADXL345_RANGE_8_G = 0b10,  ///< +/- 8g
    ADXL345_RANGE_4_G = 0b01,  ///< +/- 4g
    ADXL345_RANGE_2_G = 0b00   ///< +/- 2g (default value)
} range_e;

/*!
 * @brief Enum to function's return errors
 */
enum adxl_error_e
{
    ADXL_OK,
    ADXL_ERR_RD,
    ADXL_ERR_WR,
    ADXL_ERR_INIT,
    ADXL_ERR_DEV_NOT_FOUND,
    ADXL_ERR_NULL_POINTER,
    ADXL_ERR_INVALID_POWERMODE,
    ADXL_ERR_READ_FIFO_CONFIG,
    ADXL_ERR_READ_FIFO_DATA,
    ADXL_ERR_RD_DMA,
    ADXL_ERR_FIFO_FRAME_EMPTY,
    ADXL_ERR_INVALID_CONFIG,
    ADXL_ERR_NO_NEW_AVAILABLE,
};

/************************************************/
/*!  @name        Struct Declarations           */
/************************************************/
/*!
 * @brief ADXL345 accel configuration structure
 */
struct adxl_acc_conf_t
{
    /*! Accel g-range selection */
    uint8_t range;

    /*! Sample rate (output data rate) */
    uint8_t odr;
};

#endif