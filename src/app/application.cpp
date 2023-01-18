#include "application_def.h"
#include "application.h"

DPB::DPB(uint8_t escGPIO, dshot_mode_t escSpeed, gpio_num_t rotSensorGPIO, BMX055 *accelDev) : Motor(escGPIO, escSpeed), RotSense(rotSensorGPIO), Accel(accelDev)
{
    _motorStartupTimer = xTimerCreate("MotorStartupTimer", MOTOR_STARTUP_DELAY_MS, pdFALSE, this, __motorStartupTimerCallback_static);
    assert(_motorStartupTimer);

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
}

uint8_t DPB::init(QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, TaskHandle_t supportTask_handle, DPBShared *sharedData)
{
    if ((xQueueSysInput_handle == 0) || (xQueueSysOutput_handle == 0) || (supportTask_handle == 0) || (sharedData == 0))
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "DPB intialization started");
    _xQueueSysInput = xQueueSysInput_handle;
    _xQueueSysOutput = xQueueSysOutput_handle;
    _xSuppTask = supportTask_handle;

    init_accel();
    init_esc();
    init_rpm(_xSuppTask, _rpmTimer, DEFAULT_PROP_NUM);

    /* Wait until all has been initialized */
    ESP_LOGI(TAG, "Wait until all has been initialized");
    while (_init_status != (INIT_ESC_DONE | INIT_ACCEL_DONE | INIT_RPM_S_DONE))
        ;

    ESP_LOGI(TAG, "DPB intialization completed");

    _init_done = 1;
    setStep(IDLE);

    return ESP_OK;
}

uint8_t DPB::init_rpm(TaskHandle_t supportTask_handle, gptimer_handle_t xTimer_handle, uint8_t n_propeller)
{
    this->init_rpm_sensor(supportTask_handle, xTimer_handle, n_propeller);

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
    if (this->__set_default_config() != ESP_OK)
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
    _xShared.setRPM(get_rpm());
}

void DPB::loop_accel(void)
{
    acc_data_i acc_data;
    size_t i = 0;

    if (!_init_done)
    {
        return;
    };

    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    _app_step = VIBES_REC;
    i = 0;
    while (i < ACC_DATA_BUFFER_SIZE)
    {
        if (read_acceleration_data(&acc_data) == ESP_OK)
        {
            uint8_t r_d;
            uint64_t time;

            gptimer_get_raw_count(_rpmTimer, &time);
            r_d = get_rotation_done();

            _xShared.setAccX(acc_data.acc_x, r_d, time, i);
            _xShared.setAccY(acc_data.acc_y, r_d, time, i);

            reset_rotation_done();

            i++;
        }
    }

    command_data command;

    command.command = APP_CMD;
    command.value.ull = FILTERING;
    xQueueSend(_xQueueSysInput, &command, portMAX_DELAY);
}

void DPB::exe(command_data command)
{
    switch (command.command)
    {
    case APP_CMD:
        if (command.value.ull == IDLE)
        {
            reset();
        }
        if (command.value.ull == START_MOTOR)
        {
            start();
        }
        if (command.value.ull == VIBES_REC)
        {
            ;
        }
        if (command.value.ull == FILTERING)
        {
            motor_stop();
            log_acc_data();
            ask_charts_update(); // ? Show raw data
            reset();
        }
        if (command.value.ull == ANALYSING)
        {
            // app_fft();
        }
        break;

    case MOTOR_CMD:
        set_throttle(command.value.ull);
        break;

    case START_BUT_CMD:
        if (_app_step == IDLE)
        {
            start();
            break;
        }
        reset();
        break;
    case FFT_REQUEST_CMD:

        break;
    case LPF_REQUEST_CMD:
        if (filter_data() == ESP_OK)
            ask_charts_update(); // ? Show filtered data
        reset();
        break;
    default:
        break;
    }
}

void DPB::start(void)
{
    set_throttle(DEFAULT_MEASURE_THROTTLE);
    if (xTimerReset(_motorStartupTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer start-command timeout");
    }
    setStep(START_MOTOR);
}

void DPB::reset(void)
{
    if (xTimerStop(_motorStartupTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer stop-command timeout");
    }
    set_throttle(0);
    setStep(IDLE);
}

void DPB::ask_charts_update(void)
{
    command_data command;

    command.command = ACC_CHART_UPDATE_CMD;
    command.value.ull = 1;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

uint8_t DPB::filter_data(void)
{
    __attribute__((aligned(16))) static float accX_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accX_filtered[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_filtered[ACC_DATA_BUFFER_SIZE];
    float_t coeffs_lpf[5];
    float_t w_lpf[5] = {0, 0};

    setStep(FILTERING);

    for (int32_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        dpb_acc_data d;
        _xShared.getAccData(&d, i);

        accX_input[i] = d.accel_data.acc_x;
        accY_input[i] = d.accel_data.acc_y;
    }

    // Calculate iir filter coefficients
    if (dsps_biquad_gen_lpf_f32(coeffs_lpf, 0.1, 1) != ESP_OK)
    {
        ESP_LOGE(TAG, "IIR coefs calc error");
        return ESP_FAIL;
    }

    // Filter X signal
    if (dsps_biquad_f32(accX_input, accX_filtered, ACC_DATA_BUFFER_SIZE, coeffs_lpf, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }
    // Filter Y signal
    if (dsps_biquad_f32(accY_input, accY_filtered, ACC_DATA_BUFFER_SIZE, coeffs_lpf, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }

    for (int32_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        dpb_acc_data d;
        _xShared.getAccData(&d, i);

        _xShared.setAccX(accX_filtered[i], d.xRot_done, d.time_counts, i);
        _xShared.setAccY(accY_filtered[i], d.xRot_done, d.time_counts, i);
    }

    return ESP_OK;
}

uint8_t DPB::fft(void)
{
    setStep(ANALYSING);

    int32_t N = ACC_DATA_BUFFER_SIZE;

    // Window coefficients
    __attribute__((aligned(16)))
    float_t wind[ACC_DATA_BUFFER_SIZE];

    // Init FFT filter
    if (dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE) != ESP_OK)
    {
        char *TAG = "app_fft";
        ESP_LOGE(TAG, "Not possible to initialize FFT");
        return ESP_FAIL;
    }

    // Generate hann window
    dsps_wind_hann_f32(wind, N);
    // working complex array
    __attribute__((aligned(16))) static float x_cf[ACC_DATA_BUFFER_SIZE * 2];
    __attribute__((aligned(16))) static float y_cf[ACC_DATA_BUFFER_SIZE * 2];

    // Convert input vectors to complex vectors
    ESP_LOGW(TAG, "Signal accZ");
    for (int32_t i = 0; i < N; i++)
    {
        dpb_acc_data d;
        _xShared.getAccData(&d, i);

        /* _xShared.setAccX(d.accel_data.acc_x * wind[i], d.xRot_done, d.time_counts, i);
        _xShared.setAccY(d.accel_data.acc_y * wind[i], d.xRot_done, d.time_counts, i); */

        x_cf[i * 2 + 0] = d.accel_data.acc_x * wind[i];
        x_cf[i * 2 + 1] = 0;
        y_cf[i * 2 + 0] = d.accel_data.acc_y * wind[i];
        y_cf[i * 2 + 1] = 0;
    }

    return ESP_OK;
}

void DPB::setStep(app_steps v)
{
    _app_step = v;
    _xShared.setAppStatus(v);
}

int8_t DPB::get_app_step()
{
    return _app_step;
}

void DPB::log_acc_data(void)
{
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Accelerations data acquired");

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        dpb_acc_data data;
        _xShared.getAccData(&data, i);
        // ESP_LOGI(TAG, "| X: %d, Y: %d | Rot:%d | T:%llu |", data.accel_data.acc_x, data.accel_data.acc_y, data.xRot_done, data.time_counts);
        printf("%d;%d;%llu\n", data.accel_data.acc_x, data.accel_data.acc_y, data.time_counts);
    }
#endif
}

/* Thank to @FoxKeys https://github.com/espressif/esp-idf/issues/2355 */
void DPB::__motorStartupTimerCallback_static(TimerHandle_t pxTimer)
{
    reinterpret_cast<DPB *>(pvTimerGetTimerID(pxTimer))->__motorStartupTimerCallback(pxTimer);
}

void DPB::__motorStartupTimerCallback(TimerHandle_t pxTimer)
{
    xTaskNotifyGiveIndexed(accelTaskHandle, 0);
}