#include "application_def.h"
#include "application.h"

uint8_t DPB::init(Motor *motor, gpio_num_t opto_gpio_num, QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, FIFOBuffer<acc_sensor_data> *pDataBuffer, fft_chart_data *pFFTOuput)
{
    uint8_t ret = ESP_OK;

    if ((xQueueSysInput_handle == 0) || (xQueueSysOutput_handle == 0) || (motor == 0) || (pDataBuffer == 0) || (pFFTOuput == 0))
    {
        return ESP_FAIL;
    }

    _xQueueSysInput = xQueueSysInput_handle;
    _xQueueSysOutput = xQueueSysOutput_handle;
    _pDataBuffer = pDataBuffer;
    _pMotor = motor;
    _pFFTOuput = pFFTOuput;

     _vibeRecTimer = xTimerCreate("VibeRecorderTimer", // Just a text name, not used by the kernel.
                                 VIBE_RECORD_TIME_MS, // The timer period in ticks.
                                 pdFALSE,             // The timers will auto-reload themselves when they expire.
                                 0,                   // Assign each timer a unique id equal to its array index.
                                 _vibeTimerCallback   // Each timer calls the same callback when it expires.
    );

    return ret;
}

void _vibeTimerCallback(TimerHandle_t pxTimer)
{
;
}

