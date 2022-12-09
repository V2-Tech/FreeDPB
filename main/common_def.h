#ifndef INC_COMMON_DEF_H
#define INC_COMMON_DEF_H

/************************
 *      DEVICE CHIOSE   *
 ************************/
#define USE_BMX055

/************************************/
/*      STRUCT DECLARATIONS         */
/************************************/

struct acc_sensor_data
{
#ifdef USE_BMX055
    int16_t accel_data[3];
#endif
};

union gui_status
{
    uint32_t allBits;
    struct
    {
        unsigned chart_running : 1;
        unsigned nd1 : 1;
        unsigned nd2 : 1;
        unsigned nd3 : 1;
        unsigned nd4 : 1;
        unsigned nd5 : 1;
        unsigned nd6 : 1;
        unsigned nd7 : 1;
    };
};

struct command_data
{
    uint8_t command;
    uint8_t value;
};

#endif