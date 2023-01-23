#include "motor.h"

inline const char *TAG = "motor";

Motor::Motor(uint8_t pin, dshot_mode_t speed)
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    _esc_chan = NULL;

    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = pin,
        .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
        .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &_esc_chan));

    ESP_LOGI(TAG, "Install Dshot ESC encoder");
    _dshot_encoder = NULL;
    dshot_esc_encoder_config_t encoder_config = {
        .resolution = DSHOT_ESC_RESOLUTION_HZ,
        .baud_rate = (uint32_t)speed, // DSHOT protocol
        .post_delay_us = 50,          // extra delay between each frame
    };

    ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config, &_dshot_encoder));
}

Motor::~Motor()
{
    ;
}

void Motor::arm(void)
{
    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(_esc_chan));

    _tx_config = {
        .loop_count = -1, // infinite loop
    };
    _throttle = {
        .throttle = 0,
        .telemetry_req = false, // telemetry is not supported in this example
    };

    ESP_LOGI(TAG, "Start ESC by sending zero throttle for a while...");
    ESP_ERROR_CHECK(rmt_transmit(_esc_chan, _dshot_encoder, &_throttle, sizeof(_throttle), &_tx_config));
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGI(TAG, "ESC armed");
}

void Motor::set_throttle(uint16_t throttle_val)
{
    _throttle.throttle = throttle_val;
    ESP_ERROR_CHECK(rmt_transmit(_esc_chan, _dshot_encoder, &_throttle, sizeof(_throttle), &_tx_config));
    // the previous loop transfer is till undergoing, we need to stop it and restart,
    // so that the new throttle can be updated on the output
    ESP_ERROR_CHECK(rmt_disable(_esc_chan));
    ESP_ERROR_CHECK(rmt_enable(_esc_chan));
}

void Motor::motor_stop(void)
{
    set_throttle(0);
}