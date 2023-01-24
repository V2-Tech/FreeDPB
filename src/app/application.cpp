#include "application.h"

//************************/
//*      VARIABLES       */
//************************/
static const char *TAG = "DPB";

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         FUNCTIONS DEFINITION        /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
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
    RotSense::init_rpm_sensor(supportTask_handle, xTimer_handle, n_propeller);

    _init_status |= INIT_RPM_S_DONE;

    return ESP_OK;
}

uint8_t DPB::init_esc()
{
    Motor::arm();

    _init_status |= INIT_ESC_DONE;

    return ESP_OK;
}

uint8_t DPB::init_accel()
{
    if (this->__set_default_config() != ESP_OK)
    {
        return ESP_FAIL;
    }

    _xShared.setSampleRate(1000);

    _init_status |= INIT_ACCEL_DONE;

    return ESP_OK;
}

void DPB::loop(void)
{
    if (!_init_done)
    {
        return;
    };

    command_data_t command;

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
    acc_data_i_t acc_data;
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

            _xShared.setDPBAccData(acc_data.acc_x, acc_data.acc_y, r_d, time, i);

            i++;
        }
    }

    command_data_t command;

    command.command = APP_CMD;
    command.value.ull = FILTERING;
    xQueueSend(_xQueueSysInput, &command, portMAX_DELAY);
}

void DPB::exe(command_data_t command)
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
        if (command.value.ull == FILTERING)
        {
            motor_stop();
            if (filter_data_iir(RAW_DATA) != ESP_OK)
            {
                // TODO Error message
                break;
            }
            _log_acc_data_filtered();
            if (fft_calc(FILTERED_DATA) != ESP_OK)
            {
                // TODO Error message
                break;
            }
            signal_peak_finder();
            ask_acc_charts_update(); // ? Show filtered data

            reset();
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
        if (fft_calc(FILTERED_DATA) == ESP_OK)
            ask_fft_chart_update();
        reset();
        break;
    case LPF_REQUEST_CMD:
        if (filter_data_iir(FILTERED_DATA) == ESP_OK)
            ask_acc_charts_update(); // ? Show filtered data
        reset();
        break;
    default:
        break;
    }
}

void DPB::start(void)
{
    RotSense::reset_rpm_cnt();
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

void DPB::ask_acc_charts_update(void)
{
    command_data_t command;

    command.command = ACC_CHART_UPDATE_CMD;
    command.value.ull = 1;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_fft_chart_update(void)
{
    command_data_t command;

    command.command = FFT_CHART_UPDATE_CMD;
    command.value.ull = 1;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

uint8_t DPB::filter_data_iir(data_orig_e data_type)
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
        if (data_type == RAW_DATA)
        {
            _xShared.getDPBAccData(&d, i);
        }
        else
        {
            _xShared.getDPBAccDataFiltered(&d, i);
        }

        accX_input[i] = d.accel_data.acc_x;
        accY_input[i] = d.accel_data.acc_y;
    }

    // Calculate iir filter coefficients
    if (dsps_biquad_gen_lpf_f32(coeffs_lpf, 0.02, 0.7071) != ESP_OK)
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
        _xShared.getDPBAccData(&d, i);
        _xShared.setDPBAccDataFiltered(accX_filtered[i], accY_filtered[i], d.xRot_done, d.time_counts, i);
    }

    return ESP_OK;
}

uint8_t DPB::fft_calc(data_orig_e data_type)
{
    setStep(ANALYSING);

    int32_t N = ACC_DATA_BUFFER_SIZE;
    esp_err_t ret = ESP_OK;

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
    for (int32_t i = 0; i < N; i++)
    {
        dpb_acc_data d;
        if (data_type == RAW_DATA)
        {
            _xShared.getDPBAccData(&d, i);
        }
        else
        {
            _xShared.getDPBAccDataFiltered(&d, i);
        }

        x_cf[i * 2 + 0] = d.accel_data.acc_x * wind[i];
        x_cf[i * 2 + 1] = 0;
        y_cf[i * 2 + 0] = d.accel_data.acc_y * wind[i];
        y_cf[i * 2 + 1] = 0;
    }

    // FFT
    ret += dsps_fft2r_fc32(x_cf, N);
    // Bit reverse
    ret += dsps_bit_rev_fc32(x_cf, N);
    // Convert one complex vector to two complex vectors
    ret += dsps_cplx2reC_fc32(x_cf, N);

    // FFT
    ret += dsps_fft2r_fc32(y_cf, N);
    // Bit reverse
    ret += dsps_bit_rev_fc32(y_cf, N);
    // Convert one complex vector to two complex vectors
    ret += dsps_cplx2reC_fc32(y_cf, N);

    if (ret != ESP_OK)
    {
        char *TAG = "app_fft";
        ESP_LOGE(TAG, "FFT calc error");
        return ESP_FAIL;
    }

    // Make module and normalize it
    for (int i = 0; i < N / 2; i++)
    {
        float_t t;

        //? I scale by 2 to make sure i dont lose too much precision when i convert the data to int16 for the FFT chart
        t = 2 * sqrtf((x_cf[i * 2 + 0] * x_cf[i * 2 + 0] + x_cf[i * 2 + 1] * x_cf[i * 2 + 1] + 0.0000001) / N);
        _xShared.setFFTX(t, i);

        t = 2 * sqrtf((y_cf[i * 2 + 0] * y_cf[i * 2 + 0] + y_cf[i * 2 + 1] * y_cf[i * 2 + 1] + 0.0000001) / N);
        _xShared.setFFTY(t, i);
    }

    fft_peak_finder();

    return ESP_OK;
}

void DPB::signal_peak_finder(void)
{
    dpb_acc_data data;
    dpb_acc_data data_b;
    dpb_acc_data data_a;
    dpb_acc_data data_t;
    dpb_acc_data data_lp;

    size_t maxPeakIndex = 0;

    _peakCount = 0;

    //* Find all local peaks index and absolute max peak index
    for (size_t i = 1; i < ACC_DATA_BUFFER_SIZE - 1; i++)
    {
        _xShared.getDPBAccDataFiltered(&data_b, i - 1);
        _xShared.getDPBAccDataFiltered(&data, i);
        _xShared.getDPBAccDataFiltered(&data_a, i + 1);

        if (data.accel_data.acc_x > data_b.accel_data.acc_x && data.accel_data.acc_x > data_a.accel_data.acc_x)
        {
            _xShared.setPeakIndex(i, _peakCount);

            if (_peakCount == 0)
            {
                maxPeakIndex = i;
            }

            _peakCount++;
        }
        _xShared.getDPBAccDataFiltered(&data_lp, maxPeakIndex);
        if ((data.accel_data.acc_x > data_lp.accel_data.acc_x))
        {
            maxPeakIndex = i;
        }
    }

    _xShared.setAccXMaxPeak(maxPeakIndex);

    bool *to_remove = new bool[_peakCount]();

    //* Calc min distance between two real local peak
    static float_t fft_res = _xShared.getSampleRate();
    fft_res /= FFT_DATA_BUFFER_SIZE;
    static float_t signal_fundamental = fft_res * _xShared.getFFTXPeak();
    if (signal_fundamental == 0)
    {
        signal_fundamental = fft_res; // Min distance equal to FFT resolution
    }

    static float_t peak_min_dist = ((float_t)900000.0 / signal_fundamental); //? supposing time_count_res 1us

    //* Remove peak indexs that dont meet the min required distance
    size_t *height_sorted_peak_index = new size_t[_peakCount](); //? Index of peak index
    _sort_index_by_height(height_sorted_peak_index, _peakCount);
    for (size_t i = 0; i < _peakCount; i++)
    {
        size_t current = height_sorted_peak_index[i];

        if (to_remove[current])
        {
            continue; // peak will already be removed, move on.
        }

        // check on left side of peak
        int16_t neighbor = current - 1;
        _xShared.getDPBAccDataFiltered(&data, _xShared.getPeakIndex(current));
        if (neighbor >= 0)
        {
            _xShared.getDPBAccDataFiltered(&data_t, _xShared.getPeakIndex(neighbor));
        }

        while (neighbor >= 0 && (data.time_counts - data_t.time_counts) < peak_min_dist)
        {
            to_remove[neighbor] = true;
            --neighbor;
            _xShared.getDPBAccDataFiltered(&data, _xShared.getPeakIndex(current));
            _xShared.getDPBAccDataFiltered(&data_t, _xShared.getPeakIndex(neighbor));
        }

        // check on right side of peak
        neighbor = current + 1;
        _xShared.getDPBAccDataFiltered(&data, _xShared.getPeakIndex(current));
        if (neighbor < _peakCount)
        {
            _xShared.getDPBAccDataFiltered(&data_t, _xShared.getPeakIndex(neighbor));
        }
        while (neighbor < _peakCount && (data_t.time_counts - data.time_counts) < peak_min_dist)
        {
            to_remove[neighbor] = true;
            ++neighbor;
            _xShared.getDPBAccDataFiltered(&data, _xShared.getPeakIndex(current));
            _xShared.getDPBAccDataFiltered(&data_t, _xShared.getPeakIndex(neighbor));
        }
    }

    // Remove the incorrect peak indexs
    for (size_t i = 0, j = 0; i < _peakCount; i++)
    {
        if (to_remove[i])
        {
            _xShared.setPeakIndex(-1, i);
        }
    }

    _array_value_remover(_xShared.getPeakIndexPointer(), _peakCount, -1);

    delete[] to_remove;
    delete[] height_sorted_peak_index;
}

void DPB::fft_peak_finder(void)
{
    float_t fft_x = 0, fft_y = 0;
    float_t peak_x = 0, peak_y = 0;
    size_t peak_x_index = 0, peak_y_index = 0;

    for (size_t i = 2; i < FFT_DATA_BUFFER_SIZE; i++)
    {
        fft_x = _xShared.getFFTX(i);
        fft_y = _xShared.getFFTY(i);
        if (fft_x > peak_x)
        {
            peak_x = fft_x;
            peak_x_index = i;
        }
        if (fft_y > peak_y)
        {
            peak_y = fft_y;
            peak_y_index = i;
        }
    }

    _xShared.setFFTXPeak(peak_x_index);
    _xShared.setFFTYPeak(peak_y_index);
}

void DPB::setStep(app_steps_e v)
{
    _app_step = v;
    _xShared.setAppStatus(v);
}

void DPB::_log_acc_data(void)
{
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Accelerations data acquired");

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        dpb_acc_data data;
        _xShared.getDPBAccData(&data, i);
        // ESP_LOGI(TAG, "| X: %d, Y: %d | Rot:%d | T:%llu |", data.accel_data.acc_x, data.accel_data.acc_y, data.xRot_done, data.time_counts);
        printf("%d;%d;%llu\n", data.accel_data.acc_x, data.accel_data.acc_y, data.time_counts);
    }
#endif
}

void DPB::_log_acc_data_filtered(void)
{
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Filtered accelerations data");

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        dpb_acc_data data;
        _xShared.getDPBAccDataFiltered(&data, i);
        // ESP_LOGI(TAG, "| X: %d, Y: %d | Rot:%d | T:%llu |", data.accel_data.acc_x, data.accel_data.acc_y, data.xRot_done, data.time_counts);
        printf("%d;%d;%llu;%d\n", data.accel_data.acc_x, data.accel_data.acc_y, data.time_counts, data.xRot_done);
    }
#endif
}

void DPB::_sort_index_by_height(size_t *output, size_t indexCount)
{
    dpb_acc_data data;
    dpb_acc_data data_a;

    for (size_t i = 0; i < indexCount; i++)
    {
        output[i] = i;
    }

    for (size_t i = 0; i < indexCount; i++)
    {
        for (size_t j = i + 1; j < indexCount; j++)
        {
            _xShared.getDPBAccDataFiltered(&data, _xShared.getPeakIndex(i));
            _xShared.getDPBAccDataFiltered(&data_a, _xShared.getPeakIndex(j));

            if (data.accel_data.acc_x < data_a.accel_data.acc_x)
            {
                size_t temp = output[i];
                output[i] = output[j];
                output[j] = temp;
            }
        }
    }
}

void DPB::_array_value_remover(int16_t *array, size_t arraySize, int16_t target)
{
    size_t new_size = arraySize;

    for (size_t i = 0; i < arraySize; i++)
    {
        if (array[i] == target)
        {
            for (size_t j = i; j < new_size - 1; j++)
            {
                array[j] = array[j + 1];
                if (j == (new_size - 2))
                {
                    array[j + 1] = 0;
                }
            }
            i--;
        }
    }
}

/* Thank to @FoxKeys https://github.com/espressif/esp-idf/issues/2355 */
void DPB::__motorStartupTimerCallback_static(TimerHandle_t pxTimer)
{
    reinterpret_cast<DPB *>(pvTimerGetTimerID(pxTimer))->__motorStartupTimerCallback(pxTimer);
}

void DPB::__motorStartupTimerCallback(TimerHandle_t pxTimer)
{
    xTaskNotifyGiveIndexed(accelTaskHandle, 0);
    if (xTimerStop(pxTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer stop-command timeout");
    }
}