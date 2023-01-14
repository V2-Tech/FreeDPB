#include "position_def.h"
#include "position.h"

RotSense::RotSense(gpio_num_t opto_gpio_num)
{
    _in_GPIO = opto_gpio_num;
    _rot_done = _rpm = 0;

    ESP_LOGI(TAG, "Create timer handle");
    gptimer_config_t rpm_timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&rpm_timer_config, &_rpmTimer));
    ESP_LOGI(TAG, "Enable rpm timer");
    ESP_ERROR_CHECK(gptimer_enable(_rpmTimer));

    /* GPIO interrupt seem to be bugged (ESP-IDF v5.0): even with a debounce circuit, ghost events fire without reasons */
    /* Better use PCNT which have an anti-glitch already builded inside. With a propper debounce circuit, it's seem to work perfectly */
    ESP_LOGI(TAG, "install pcnt unit");
    pcnt_unit_config_t unit_config = {
        .low_limit = -1,
        .high_limit = 10,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &__rpm_pcnt));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 5000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(__rpm_pcnt, &filter_config));

    ESP_LOGI(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = _in_GPIO,
        .level_gpio_num = -1,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(__rpm_pcnt, &chan_a_config, &pcnt_chan_a));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));

    ESP_LOGI(TAG, "add watch points and register callbacks");
    int watch_points[] = {DEFAULT_PROP_NUM};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++)
    {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(__rpm_pcnt, watch_points[i]));
    }
    pcnt_event_callbacks_t cbs = {
        .on_reach = __opto_isr_handler_static,
    };
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(__rpm_pcnt, &cbs, this));
    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(__rpm_pcnt));
    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(__rpm_pcnt));
    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(__rpm_pcnt));
}

RotSense::~RotSense()
{
    ;
}

uint8_t RotSense::init_rpm_sensor(TaskHandle_t Task_handle, QueueHandle_t xQueueSysOutput_handle, uint8_t n_propeller)
{
    if ((n_propeller == 0) || (xQueueSysOutput_handle == 0) || (Task_handle == 0))
    {
        return ESP_FAIL;
    }

    _prop_number = n_propeller;
    __xQueueSysOutput = xQueueSysOutput_handle;
    __task_calc = Task_handle;

    return ESP_OK;
}

void RotSense::rpm_update()
{
    uint64_t count;
    float_t f_count;
    

    static uint8_t __isr_event_counter = 0;

    ESP_ERROR_CHECK(pcnt_unit_clear_count(__rpm_pcnt));

    if (__isr_event_counter == 0)
    {
        __isr_event_counter++;
        ESP_ERROR_CHECK(gptimer_start(_rpmTimer));
        return;
    }

    ESP_ERROR_CHECK(gptimer_get_raw_count(_rpmTimer, &count));
    f_count = static_cast<float_t>(count);

    _rpm = 60 / (f_count / 1000000);
    gptimer_stop(_rpmTimer);
    ESP_ERROR_CHECK(gptimer_set_raw_count(_rpmTimer, 0));

    __isr_event_counter = 0;
    _rot_done = 1;
}

float_t RotSense::get_rpm()
{
    return _rpm;
}

uint8_t RotSense::get_rotation_done()
{
    return _rot_done;
}

void RotSense::reset_rotation_done()
{
    _rot_done = 0;
}

void RotSense::set_propeller(uint8_t n_propeller)
{
    _prop_number = n_propeller;
}

uint8_t RotSense::get_propeller()
{
    return _prop_number;
}

/* Thank to @FoxKeys https://github.com/espressif/esp-idf/issues/2355 */
/* void IRAM_ATTR RotSense::__opto_isr_handler_static(void *arg)
{
    reinterpret_cast<RotSense *>(arg)->__opto_isr_handler();
} */
bool IRAM_ATTR RotSense::__opto_isr_handler_static(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    reinterpret_cast<RotSense *>(user_ctx)->__opto_isr_handler();
    return pdTRUE;
}

void IRAM_ATTR RotSense::__opto_isr_handler()
{
    BaseType_t xHigherPriorityTaskWoken;

    vTaskNotifyGiveIndexedFromISR(__task_calc, 0, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}