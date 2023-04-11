#include "include/RPMTracker.hpp"
#include "RPMTracker.hpp"

//************************/
//*      VARIABLES       */
//************************/
static const char *TAG = "RPMTracker";

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         FUNCTIONS DEFINITION        /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
RPMTracker::RPMTracker(gpio_num_t opto_gpio_num)
{
    _in_GPIO = opto_gpio_num;
    _rot_done = _rpm = 0;

    __xSem = xSemaphoreCreateMutex();
    assert(__xSem);

    ESP_LOGI(TAG, "Creating rpm timer");
    gptimer_config_t rpm_timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&rpm_timer_config, &_rpmTimer));
    ESP_LOGI(TAG, "Enable rpm timer");
    ESP_ERROR_CHECK(gptimer_enable(_rpmTimer));
    ESP_LOGI(TAG, "Starting rpm timer");
    ESP_ERROR_CHECK(gptimer_start(_rpmTimer));

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
}

RPMTracker::~RPMTracker()
{
    ;
}

uint8_t RPMTracker::init(TaskHandle_t Task_handle, uint8_t n_propeller, void (*signalCallback)(void))
{
    if ((n_propeller == 0) || (Task_handle == 0) || (signalCallback == 0))
    {
        return ESP_FAIL;
    }

    _prop_number = n_propeller;
    __task_calc = Task_handle;
    _rotCatchedCallback = signalCallback;

    _basic_init();

    return ESP_OK;
}
uint8_t RPMTracker::init(TaskHandle_t Task_handle, uint8_t n_propeller)
{
    if ((n_propeller == 0) || (Task_handle == 0))
    {
        return ESP_FAIL;
    }

    _prop_number = n_propeller;
    __task_calc = Task_handle;

    _basic_init();

    return ESP_OK;
}

void RPMTracker::set_init_status(uint8_t status)
{
    _init_done = status;
}

uint8_t RPMTracker::get_init_status()
{
    return _init_done;
}

void RPMTracker::loop()
{
    if (!get_init_status())
    {
        return;
    };

    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    _rpm_update();
}

void RPMTracker::_basic_init()
{
    ESP_LOGI(TAG, "add watch points and register callbacks");
    int watch_points[] = {_prop_number};
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

    ESP_LOGI(TAG, "RPM tracker initialized");

    set_init_status(1);
}

void RPMTracker::_rpm_update()
{
    uint64_t count;
    float_t f_delta;
    static uint64_t last_count = 0;

    ESP_ERROR_CHECK(pcnt_unit_clear_count(__rpm_pcnt));

    ESP_ERROR_CHECK(gptimer_get_raw_count(_rpmTimer, &count));
    f_delta = static_cast<float_t>(count - last_count);
    _rpm = static_cast<uint16_t>(60.0 / (f_delta / 1000000));

    last_count = count;

    _set_rotation_done();
}

float_t RPMTracker::get_rpm()
{
    return _rpm;
}

uint8_t RPMTracker::get_time(uint64_t& time) 
{
    return gptimer_get_raw_count(_rpmTimer, &time); 
}

void RPMTracker::start_rpm_cnt()
{
    ESP_ERROR_CHECK(pcnt_unit_start(__rpm_pcnt));
}

void RPMTracker::reset_rpm_cnt()
{
    ESP_ERROR_CHECK(pcnt_unit_clear_count(__rpm_pcnt));
    ESP_ERROR_CHECK(pcnt_unit_start(__rpm_pcnt));
}

void RPMTracker::stop_rpm_cnt()
{
    ESP_ERROR_CHECK(pcnt_unit_stop(__rpm_pcnt));
}

uint8_t RPMTracker::get_rotation_done()
{
    uint8_t v;
    xSemaphoreTake(__xSem, portMAX_DELAY);
    v = _rot_done;
    _rot_done = 0;
    xSemaphoreGive(__xSem);

    return v;
}

void RPMTracker::reset_rotation_done()
{
    xSemaphoreTake(__xSem, portMAX_DELAY);
    _rot_done = 0;
    xSemaphoreGive(__xSem);
}

void RPMTracker::set_propeller(uint8_t n_propeller)
{
    _prop_number = n_propeller;
}

uint8_t RPMTracker::get_propeller()
{
    return _prop_number;
}

void RPMTracker::_set_rotation_done()
{
    xSemaphoreTake(__xSem, portMAX_DELAY);
    _rot_done = 1;
    xSemaphoreGive(__xSem);
}

/* Thank to @FoxKeys https://github.com/espressif/esp-idf/issues/2355 */
bool IRAM_ATTR RPMTracker::__opto_isr_handler_static(
    pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata,
    void *user_ctx)
{
    reinterpret_cast<RPMTracker *>(user_ctx)->__opto_isr_handler();
    return pdTRUE;
}

void IRAM_ATTR RPMTracker::__opto_isr_handler()
{
    BaseType_t xHigherPriorityTaskWoken;

    vTaskNotifyGiveIndexedFromISR(__task_calc, 0, &xHigherPriorityTaskWoken);

    if (_rotCatchedCallback != 0)
        _rotCatchedCallback();

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}