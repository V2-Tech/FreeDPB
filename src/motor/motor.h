#ifndef _MOTOR_H
#define _MOTOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "./dshot/dshot_esc_encoder.h"
#include "driver/rmt_tx.h"

#define DSHOT_ESC_RESOLUTION_HZ 40000000 // 40MHz resolution, DSHot protocol needs a relative high resolution

typedef enum dshot_mode_e
{
    DSHOT_OFF = 0,
    DSHOT150 = 150*1000,
    DSHOT300 = 300*1000,
    DSHOT600 = 600*1000,
    DSHOT1200 = 120*1000
} dshot_mode_t;

typedef enum telemetric_request_e
{
    NO_TELEMETRIC,
    ENABLE_TELEMETRIC,
} telemetric_request_t;

class Motor
{
public:
    Motor(uint8_t pin, dshot_mode_t speed);
    ~Motor();

    void arm(void);
    void set_throttle(uint16_t throttle_val);
    void motor_stop(void);

private:
    rmt_channel_handle_t _esc_chan;
    rmt_encoder_handle_t _dshot_encoder;
    rmt_transmit_config_t _tx_config;
    dshot_esc_throttle_t _throttle;
};

#endif