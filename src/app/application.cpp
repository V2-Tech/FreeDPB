#include "application_def.h"
#include "application.h"

DPB::DPB(uint8_t escGPIO, dshot_mode_t escSpeed, gpio_num_t rotSensorGPIO, BMX055 *accelDev) : Motor(escGPIO, escSpeed), RotSense(rotSensorGPIO), Accel(accelDev)
{
    _motorRunTimer = xTimerCreate(
        "VibeRecorderTimer", // Just a text name, not used by the kernel.
        VIBE_RECORD_TIME_MS, // The timer period in ticks.
        pdFALSE,             // The timers will auto-reload themselves when they expire.
        0,                   // Assign each timer a unique id equal to its array index.
        _vibeTimerCallback   // Each timer calls the same callback when it expires.
    );
    assert(_motorRunTimer);
}

uint8_t DPB::init(QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, TaskHandle_t supportTask_handle)
{
    if ((xQueueSysInput_handle == 0) || (xQueueSysOutput_handle == 0) || (supportTask_handle == 0))
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "DPB intialization started");
    _xQueueSysInput = xQueueSysInput_handle;
    _xQueueSysOutput = xQueueSysOutput_handle;
    _xSuppTask = supportTask_handle;

    init_accel();
    init_esc();
    init_rpm(_xSuppTask, _xQueueSysOutput, DEFAULT_PROP_NUM);

    /* Wait until all has been initialized */
    ESP_LOGI(TAG, "Wait until all has been initialized");
    while (_init_status != (INIT_ESC_DONE | INIT_ACCEL_DONE | INIT_RPM_S_DONE))
        ;

    ESP_LOGI(TAG, "DPB intialization completed");
    _init_done = 1;

    return ESP_OK;
}

uint8_t DPB::init_rpm(TaskHandle_t supportTask_handle, QueueHandle_t xQueueSysOutput_handle, uint8_t n_propeller)
{
    this->init_rpm_sensor(supportTask_handle, xQueueSysOutput_handle, n_propeller);

    _init_status |= INIT_RPM_S_DONE;

    return ESP_OK;
}

uint8_t DPB::init_esc()
{
    this->arm();

    _init_status |= INIT_ESC_DONE;

    return ESP_OK;
}

uint8_t DPB::init_accel()
{
    if (this->_set_default_config() != ESP_OK)
    {
        return ESP_FAIL;
    }

    _init_status |= INIT_ACCEL_DONE;

    return ESP_OK;
}

void DPB::loop(void)
{
    if (!_init_done)
    {
        return;
    };

    command_data command;

    /* Check incoming commands to execute */
    if (xQueueReceive(_xQueueSysInput, &command, portMAX_DELAY) == pdPASS)
    {
        exe(command);
    }
}

void DPB::loop_rpm(void)
{
    if (!_init_done)
    {
        return;
    };

    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    rpm_update();
}

void DPB::exe(command_data command)
{
    switch (command.command)
    {
    case APP_CMD:
        if (command.value == IDLE)
        {
            //app_reset();
        }
        if (command.value == POS_SEARCH)
        {
            //app_start();
        }
        if (command.value == VIBES_REC)
        {
            //app_rec_timer_start();
        }
        if (command.value == FILTERING)
        {
            ;
        }
        if (command.value == ANALYSING)
        {
            //app_fft();
        }
        break;

    case MOTOR_CMD:
        set_throttle(command.value);
        break;

    default:
        break;
    }
}

void _vibeTimerCallback(TimerHandle_t pxTimer)
{
    ;
}
