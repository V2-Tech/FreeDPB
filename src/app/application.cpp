#include "application_def.h"
#include "application.h"

app_status _step = IDLE;

/***********************************
 *      FUNCTION DEFINITIONS       *
 ***********************************/
uint8_t app_init(Motor *motor, gpio_num_t opto_gpio_num, QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, FIFOBuffer<acc_sensor_data> *pDataBuffer)
{
    uint8_t ret = ESP_OK;

    if ((xQueueSysInput_handle == 0) || (xQueueSysOutput_handle == 0) || (motor == 0))
    {
        return ESP_FAIL;
    }

    _xQueueSysInput = xQueueSysInput_handle;
    _xQueueSysOutput = xQueueSysOutput_handle;
    _pMotor = motor;

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = (1ULL << opto_gpio_num);
    io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // enable pull-down mode
    // io_conf.pull_up_en = GPIO_PULLUP_ENABLE;  // disable pull-up mode

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(opto_gpio_num, (gpio_isr_t)opto_isr_handler, (void *)opto_gpio_num));

    _vibeRecTimer = xTimerCreate("VibeRecorderTimer", // Just a text name, not used by the kernel.
                                 VIBE_RECORD_TIME_MS, // The timer period in ticks.
                                 pdFALSE,             // The timers will auto-reload themselves when they expire.
                                 0,                   // Assign each timer a unique id equal to its array index.
                                 vibeTimerCallback    // Each timer calls the same callback when it expires.
    );

    return ret;
}

void app_loop()
{
    command_data command;

    /* Check incoming commands to execute */
    if (xQueueReceive(_xQueueSysInput, &command, portMAX_DELAY) == pdPASS)
    {
        app_exe(command);
    }
}

void app_exe(command_data command)
{
    switch (command.command)
    {
    case APP_CMD:
        if (command.value == IDLE)
        {
            app_reset();
        }
        if (command.value == POS_SEARCH)
        {
            app_start();
        }
        if (command.value == VIBES_REC)
        {
            app_rec_timer_start();
        }

        break;

    case MOTOR_CMD:
        _pMotor->set_throttle(command.value);
        break;

    default:
        break;
    }
}

void app_start()
{
    if (_app_step > IDLE)
    {
        return;
    }

    _pMotor->set_throttle(150);

    _app_step = POS_SEARCH;
}

void app_reset()
{
    _pMotor->set_throttle(0);
    _app_step = IDLE;
}

void app_rec_timer_start()
{
    xTimerStart(_vibeRecTimer, 0);
    _app_step = VIBES_REC;
}

void vibeTimerCallback(TimerHandle_t pxTimer)
{
    acceleration_stop_read();
    _pMotor->set_throttle(0);
}

void IRAM_ATTR opto_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken;
    uint32_t gpio_num = (uint32_t)arg;

    vTaskNotifyGiveIndexedFromISR(senseTaskHandle, 0, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
