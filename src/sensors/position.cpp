#include "position_def.h"
#include "position.h"

RotSense::RotSense(gpio_num_t opto_gpio_num)
{
    _in_GPIO = opto_gpio_num;
    _isr_event_counter = 0;

    ESP_LOGI(TAG, "Create interrupt input handler");
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << opto_gpio_num);
    io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // enable pull-down mode
    // io_conf.pull_up_en = GPIO_PULLUP_ENABLE;  // disable pull-up mode

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(_in_GPIO, (gpio_isr_t)opto_isr_handler_static, (void *)_in_GPIO));
    gpio_intr_disable(_in_GPIO);

    ESP_LOGI(TAG, "Create timer handle");
    gptimer_config_t rpm_timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 10000000, // 10MHz, 1 tick=0.1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&rpm_timer_config, &_rpmTimer));
    ESP_LOGI(TAG, "Enable rpm timer");
    ESP_ERROR_CHECK(gptimer_enable(_rpmTimer));
}

RotSense::~RotSense()
{
    ;
}

/* Thank to @FoxKeys https://github.com/espressif/esp-idf/issues/2355 */
void IRAM_ATTR RotSense::opto_isr_handler_static(void *arg)
{
    reinterpret_cast<RotSense *>(arg)->opto_isr_handler();
}

void IRAM_ATTR RotSense::opto_isr_handler()
{
    uint64_t count;
    float_t rpm;

    _isr_event_counter++;

    if (_isr_event_counter == 1)
    {
        ESP_ERROR_CHECK(gptimer_start(_rpmTimer));
    }
    if (_isr_event_counter > 3)
    {
        ESP_ERROR_CHECK(gptimer_get_raw_count(_rpmTimer, &count));
        ESP_LOGI(TAG, "RPM timer count value=%llu", count);
        rpm = 60 / ((count / 10) / 1000000);
        ESP_LOGI(TAG, "RPM value=%.2f", rpm);
        ESP_ERROR_CHECK(gptimer_set_raw_count(_rpmTimer, 0));

        _isr_event_counter = 0;
    }
}
