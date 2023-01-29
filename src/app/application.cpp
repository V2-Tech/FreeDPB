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

DPB::DPB(uint8_t escGPIO, dshot_mode_t escSpeed, gpio_num_t rotSensorGPIO, ADXL345 *accelDev) : Motor(escGPIO, escSpeed), RotSense(rotSensorGPIO), Accel(accelDev)
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

int16_t DPB::init(QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, TaskHandle_t supportTask_handle, DPBShared *sharedData)
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

int16_t DPB::init_rpm(TaskHandle_t supportTask_handle, gptimer_handle_t xTimer_handle, uint8_t n_propeller)
{
    RotSense::init_rpm_sensor(supportTask_handle, xTimer_handle, n_propeller);

    _init_status |= INIT_RPM_S_DONE;

    return ESP_OK;
}

int16_t DPB::init_esc()
{
    Motor::arm();

    _init_status |= INIT_ESC_DONE;

    return ESP_OK;
}

int16_t DPB::init_accel()
{
    accel_settings_t as;

    if (Accel::set_default_config() != ESP_OK)
    {
        return ESP_FAIL;
    }
    Accel::get_acc_settings(&as);

    _xShared.setBandWidth(as.band);
    _xShared.setRange(as.range);

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
    size_t i;
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    uint8_t *pRotBuf = _xShared.getDPBRotDoneBuffer_us();
    uint64_t *pTimeBuf = _xShared.getDPBTimeBuffer_us();
    uint16_t rotCount = 0;

    if (!_init_done)
    {
        return;
    };

    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    _app_step = VIBES_REC;
    i = 0;
    rotCount = 0;

    _xShared.lockDPBDataAcc();

    RotSense::get_rotation_done(); //? call to clear dummy set of bit
    while (i < ACC_DATA_BUFFER_SIZE)
    {
        if (read_acceleration_data(&acc_data) == ESP_OK)
        {
            uint8_t r_d;
            uint64_t time;

            gptimer_get_raw_count(_rpmTimer, &time);
            r_d = get_rotation_done();

            pAccXBuf[i] = acc_data.acc_x;
            pAccYBuf[i] = acc_data.acc_y;
            pRotBuf[i] = r_d;
            pTimeBuf[i] = time;

            if (r_d)
            {
                rotCount++;
            }

            i++;
        }
    }
    _xShared.unlockDPBDataAcc();

    _xShared.setRotCount(rotCount);

    command_data_t command;

    command.command = APP_CMD;
    command.value.ull = SYS_ANALYZE_DATA;
    xQueueSend(_xQueueSysInput, &command, portMAX_DELAY);
}

void DPB::exe(command_data_t command)
{
    switch (command.command)
    {
    case APP_CMD:
        if (command.value.ull == SYS_RESET)
        {
            _exe_reset();
        }
        if (command.value.ull == SYS_START)
        {
            _exe_start();
        }
        if (command.value.ull == SYS_ANALYZE_DATA)
        {
            _exe_filter();
            _exe_analyzing();
            _exe_unbalance_finder();
            _exe_reset();
        }
        break;

    case MOTOR_CMD:
        Motor::set_throttle(command.value.ull);
        break;

    case START_BUT_CMD:
        if (_app_step == IDLE)
        {
            _exe_start();
            break;
        }
        _exe_reset();
        break;

    case FFT_REQUEST_CMD:
        _exe_fft();
        _exe_reset();
        break;

    case LPF_REQUEST_CMD:
        _exe_lpf();
        _exe_reset();
        break;
    default:
        break;
    }
}

void DPB::_exe_start(void)
{
    RotSense::reset_rpm_cnt();
    Motor::set_throttle(DEFAULT_MEASURE_THROTTLE);
    if (xTimerReset(_motorStartupTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer start-command timeout");
    }
    setStep(START_MOTOR);
}

void DPB::_exe_reset(void)
{
    if (xTimerStop(_motorStartupTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer stop-command timeout");
    }
    Motor::set_throttle(0);
    setStep(IDLE);
}

void DPB::_exe_filter(void)
{
    setStep(FILTERING);

    Motor::motor_stop();
    _log_acc_data();
    if (filter_data_iir_zero(RAW_DATA) != ESP_OK)
    {
        // TODO Error message
        return;
    }
    _log_acc_data_filtered();
    ask_acc_charts_update(); // ? Show filtered data
}

void DPB::_exe_analyzing(void)
{
    setStep(ANALYSING);
    if (fft_calc(FILTERED_DATA) != ESP_OK)
    {
        // TODO Error message
        return;
    }
    fft_peak_finder();
    signal_peak_finder();
}

void DPB::_exe_unbalance_finder(void)
{
    static const char *TAG = "EXE-UNBALANCE";
    int64_t averageX = 0;
    int64_t averageY = 0;
    uint16_t averageQuantity = 0;
    uint64_t averageRotTime = 0;
    int64_t lastDelta = 0;
    size_t maxLoops = 0;
    uint8_t *pRotDoneBuf = _xShared.getDPBRotDoneBuffer_us();
    uint64_t *pTimeBuf = _xShared.getDPBTimeBuffer_us();
    size_t *pXPeakIndexBuf = _xShared.getXPeaksIndexPointer_us();
    size_t *pYPeakIndexBuf = _xShared.getYPeaksIndexPointer_us();

    setStep(UNBALANCE_FIND);

    float_t count2deg = ((1 / _xShared.getUnbalanceFreq()) * 1000000.0) / 360.0;

    size_t rotCount = _xShared.getRotCount();
    size_t *rotDoneIndexBuf = new size_t[rotCount]();

    _xShared.lockDPBDataAcc();
    _xShared.lockPeaksIndex();

    for (size_t i = 0, j = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        if (pRotDoneBuf[i] == 1)
        {
            rotDoneIndexBuf[j++] = i;
        }
    }

    uint64_t timePeak, timeRot;
    size_t i = 0, j = 0;
    maxLoops = std::min(_xShared.getXPeakCount(), rotCount);
    if (rotDoneIndexBuf[0] < pXPeakIndexBuf[0])
    {
        j++;
    }
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "X-Unbalance delta times:");
#endif

    while (i < maxLoops && j < maxLoops)
    {
        timePeak = pTimeBuf[pXPeakIndexBuf[i]];
        timeRot = pTimeBuf[rotDoneIndexBuf[j]];
        int64_t currDelta = (int64_t)(timeRot - timePeak);
        //? It s not possible to have a sign inversion: some data is missing: shift ahead.
        if ((lastDelta < 0 && currDelta >= 0) || (lastDelta > 0 && currDelta < 0))
        {
#ifdef APP_DEBUG_MODE
            ESP_LOGW(TAG, "X-Peak [%d] missing", j);
#endif
            j++;
            continue;
        }

        lastDelta = (int64_t)(timeRot - timePeak);
#ifdef APP_DEBUG_MODE
        printf("%lli\n", lastDelta);
#endif

        averageX += lastDelta;

        averageQuantity++;

        i++;
        j++;
    }
    if (averageQuantity != 0)
    {
        averageX /= averageQuantity;
    }
#ifdef APP_DEBUG_MODE
    printf("AVERAGE-X:%lli\n", averageX);
#endif

    i = j = averageQuantity = 0;
    maxLoops = std::min(_xShared.getYPeakCount(), rotCount);
    if (rotDoneIndexBuf[0] < pYPeakIndexBuf[0])
    {
        j++;
    }
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Y-Unbalance delta times:");
#endif

    while (i < maxLoops && j < maxLoops)
    {
        timePeak = pTimeBuf[pYPeakIndexBuf[i]];
        timeRot = pTimeBuf[rotDoneIndexBuf[j]];
        int64_t currDelta = (int64_t)(timeRot - timePeak);
        //? It s not possible to have a sign inversion: some data is missing: shift ahead.
        if ((lastDelta < 0 && currDelta >= 0) || (lastDelta > 0 && currDelta < 0))
        {
#ifdef APP_DEBUG_MODE
            ESP_LOGW(TAG, "Y-Peak [%d] missing", j);
#endif
            j++;
            continue;
        }

        lastDelta = (int64_t)(timeRot - timePeak);
#ifdef APP_DEBUG_MODE
        printf("%lli\n", lastDelta);
#endif

        averageY += lastDelta;

        averageQuantity++;

        i++;
        j++;
    }
    if (averageQuantity != 0)
    {
        averageY /= averageQuantity;
    }

#ifdef APP_DEBUG_MODE
    printf("AVERAGE-Y:%lli\n", averageY);
#endif

    _xShared.unlockDPBDataAcc();
    _xShared.unlockPeaksIndex();

    _xShared.setUnbalanceXAngle((float_t)averageX / count2deg);
    _xShared.setUnbalanceYAngle((float_t)averageY / count2deg);

    delete[] rotDoneIndexBuf;
}

void DPB::_exe_fft(void)
{
    if (fft_calc(FILTERED_DATA) != ESP_OK)
    {
        return;
    }
    fft_peak_finder();
    ask_fft_chart_update();
}

void DPB::_exe_lpf(void)
{
    if (filter_data_iir(FILTERED_DATA) != ESP_OK)
    {
        return;
    }
    ask_acc_charts_update(); // ? Show filtered data
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

int16_t DPB::filter_data_iir(data_orig_e data_type)
{
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    int16_t *pAccXFltBuf = _xShared.getDPBDataFltAccXBuffer_us();
    int16_t *pAccYFltBuf = _xShared.getDPBDataFltAccYBuffer_us();

    __attribute__((aligned(16))) static float accX_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accX_filtered[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_filtered[ACC_DATA_BUFFER_SIZE];
    float_t coeffs_lpf[5];
    float_t w_lpf[5] = {0, 0};

    setStep(FILTERING);
    _xShared.lockDPBDataAcc();
    _xShared.lockDPBDataFltAcc();

    for (int32_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        if (data_type == RAW_DATA)
        {
            accX_input[i] = pAccXBuf[i];
            accY_input[i] = pAccYBuf[i];
        }
        else
        {
            accX_input[i] = pAccXFltBuf[i];
            accY_input[i] = pAccYFltBuf[i];
        }
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
        pAccXFltBuf[i] = accX_filtered[i];
        pAccYFltBuf[i] = accY_filtered[i];
    }

    _xShared.unlockDPBDataAcc();
    _xShared.unlockDPBDataFltAcc();
    return ESP_OK;
}

int16_t DPB::filter_data_iir_zero(data_orig_e data_type)
{
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    int16_t *pAccXFltBuf = _xShared.getDPBDataFltAccXBuffer_us();
    int16_t *pAccYFltBuf = _xShared.getDPBDataFltAccYBuffer_us();

    __attribute__((aligned(16))) static float accX_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accX_filtered[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_filtered[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accX_filtered_rev[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_filtered_rev[ACC_DATA_BUFFER_SIZE];
    float_t coeffs_lpf[5];
    float_t w_lpf[5] = {0, 0};

    _xShared.lockDPBDataAcc();
    _xShared.lockDPBDataFltAcc();

    for (int32_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        if (data_type == RAW_DATA)
        {
            accX_input[i] = pAccXBuf[i];
            accY_input[i] = pAccYBuf[i];
        }
        else
        {
            accX_input[i] = pAccXFltBuf[i];
            accY_input[i] = pAccYFltBuf[i];
        }
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

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        accX_filtered_rev[i] = accX_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
        accY_filtered_rev[i] = accY_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
    }

    // Filter X signal
    if (dsps_biquad_f32(accX_filtered_rev, accX_filtered, ACC_DATA_BUFFER_SIZE, coeffs_lpf, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }
    // Filter Y signal
    if (dsps_biquad_f32(accY_filtered_rev, accY_filtered, ACC_DATA_BUFFER_SIZE, coeffs_lpf, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        accX_filtered_rev[i] = accX_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
        accY_filtered_rev[i] = accY_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
    }

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        pAccXFltBuf[i] = accX_filtered_rev[i];
        pAccYFltBuf[i] = accY_filtered_rev[i];
    }

    _xShared.unlockDPBDataAcc();
    _xShared.unlockDPBDataFltAcc();

    return ESP_OK;
}

int16_t DPB::fft_calc(data_orig_e data_type)
{
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    int16_t *pAccXFltBuf = _xShared.getDPBDataFltAccXBuffer_us();
    int16_t *pAccYFltBuf = _xShared.getDPBDataFltAccYBuffer_us();
    float_t *pFFTXBuf = _xShared.getFFTXBuffer_us();
    float_t *pFFTYBuf = _xShared.getFFTXBuffer_us();

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

    _xShared.lockDPBDataAcc();
    _xShared.lockDPBDataFltAcc();

    // Convert input vectors to complex vectors
    for (int32_t i = 0; i < N; i++)
    {
        if (data_type == RAW_DATA)
        {
            x_cf[i * 2 + 0] = pAccXBuf[i] * wind[i];
            x_cf[i * 2 + 1] = 0;
            y_cf[i * 2 + 0] = pAccYBuf[i] * wind[i];
            y_cf[i * 2 + 1] = 0;
        }
        else
        {
            x_cf[i * 2 + 0] = pAccXFltBuf[i] * wind[i];
            x_cf[i * 2 + 1] = 0;
            y_cf[i * 2 + 0] = pAccYFltBuf[i] * wind[i];
            y_cf[i * 2 + 1] = 0;
        }
    }

    _xShared.unlockDPBDataAcc();
    _xShared.unlockDPBDataFltAcc();

    // FFT X-axes
    ret += dsps_fft2r_fc32(x_cf, N);
    // Bit reverse
    ret += dsps_bit_rev_fc32(x_cf, N);
    // Convert one complex vector to two complex vectors
    ret += dsps_cplx2reC_fc32(x_cf, N);

    // FFT Y-axes
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

    _xShared.lockFFT();
    // Make module and normalize it
    for (int i = 0; i < N / 2; i++)
    {
        //? I scale by 2 to make sure i dont lose too much precision when i convert the data to int16 for the FFT chart
        pFFTXBuf[i] = 2 * sqrtf((x_cf[i * 2 + 0] * x_cf[i * 2 + 0] + x_cf[i * 2 + 1] * x_cf[i * 2 + 1] + 0.0000001) / N);
        pFFTYBuf[i] = 2 * sqrtf((y_cf[i * 2 + 0] * y_cf[i * 2 + 0] + y_cf[i * 2 + 1] * y_cf[i * 2 + 1] + 0.0000001) / N);
    }

    _xShared.unlockFFT();

    return ESP_OK;
}

void DPB::signal_peak_finder(void)
{
    size_t maxXPeakIndex = 0, maxYPeakIndex = 0;

    //* Find all local peaks index and absolute max peak index
    _xShared.lockDPBDataFltAcc();
    _xShared.lockPeaksIndex();

    ESP_ERROR_CHECK(peaks_finder<int16_t>(_xShared.getDPBDataFltAccXBuffer_us(), _xShared.getXPeaksIndexPointer_us(), &_XpeakCount, &maxXPeakIndex, ACC_DATA_BUFFER_SIZE, ACC_DATA_BUFFER_SIZE));
    ESP_ERROR_CHECK(peaks_finder<int16_t>(_xShared.getDPBDataFltAccYBuffer_us(), _xShared.getYPeaksIndexPointer_us(), &_YpeakCount, &maxYPeakIndex, ACC_DATA_BUFFER_SIZE, ACC_DATA_BUFFER_SIZE));
    _xShared.setAccXMaxIndex(maxXPeakIndex);
    _xShared.setAccYMaxIndex(maxYPeakIndex);

    _xShared.unlockDPBDataFltAcc();
    _xShared.unlockPeaksIndex();

    //* Calc min distance between two real local peak
    float_t fund = _get_fundamental_freq(_xShared.getBandWidth(), FFT_DATA_BUFFER_SIZE);
    float_t peak_min_dist = ((0.8 * 1000000.0) / fund); //! _rpmTimer has 1us resolution
    _xShared.setUnbalanceFreq(fund);

    //* Remove peak indexs that dont meet the min required distance
    size_t *height_sorted_Xpeak_index = new size_t[_XpeakCount](); //? Index of peak index
    size_t *height_sorted_Ypeak_index = new size_t[_YpeakCount](); //? Index of peak index

    _xShared.lockDPBDataAcc();
    _xShared.lockPeaksIndex();
    ESP_ERROR_CHECK(array_map_incr<int16_t>(_xShared.getXPeaksIndexPointer_us(), _xShared.getDPBDataFltAccXBuffer_us(), height_sorted_Xpeak_index, _XpeakCount));
    ESP_ERROR_CHECK(array_map_incr<int16_t>(_xShared.getYPeaksIndexPointer_us(), _xShared.getDPBDataFltAccYBuffer_us(), height_sorted_Ypeak_index, _YpeakCount));

#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Filter X-Peak");
#endif
    _peaks_filter_by_distance(_xShared.getDPBTimeBuffer_us(), _xShared.getXPeaksIndexPointer_us(), height_sorted_Xpeak_index, _XpeakCount, peak_min_dist);
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Filter Y-Peak");
#endif
    _peaks_filter_by_distance(_xShared.getDPBTimeBuffer_us(), _xShared.getYPeaksIndexPointer_us(), height_sorted_Ypeak_index, _YpeakCount, peak_min_dist);
    _xShared.unlockDPBDataAcc();

    ESP_ERROR_CHECK(array_value_remover<size_t>(_xShared.getXPeaksIndexPointer_us(), &_XpeakCount, (size_t)-1));
    ESP_ERROR_CHECK(array_value_remover<size_t>(_xShared.getYPeaksIndexPointer_us(), &_YpeakCount, (size_t)-1));
    _xShared.unlockPeaksIndex();

    _xShared.setXPeakCount(_XpeakCount);
    _xShared.setYPeakCount(_YpeakCount);

    delete[] height_sorted_Xpeak_index;
    delete[] height_sorted_Ypeak_index;
}

void DPB::fft_peak_finder(void)
{
    float_t *pFFTXBuf = _xShared.getFFTXBuffer_us();
    float_t *pFFTYBuf = _xShared.getFFTXBuffer_us();

    float_t fft_x = 0, fft_y = 0;
    float_t peak_x = 0, peak_y = 0;
    size_t peak_x_index = 0, peak_y_index = 0;

    _xShared.lockFFT();
    for (size_t i = 2; i < FFT_DATA_BUFFER_SIZE; i++)
    {
        fft_x = pFFTXBuf[i];
        fft_y = pFFTYBuf[i];
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
    _xShared.unlockFFT();

    _xShared.setFFTXMaxIndex(peak_x_index);
    _xShared.setFFTYMaxIndex(peak_y_index);
}

void DPB::setStep(app_steps_e v)
{
    _app_step = v;
    _xShared.setAppStatus(v);
}

void DPB::_log_acc_data(void)
{
#ifdef APP_DEBUG_MODE
    int16_t x;
    int16_t y;
    uint8_t r;
    uint64_t t;

    ESP_LOGI(TAG, "Accelerations data acquired");

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        _xShared.getDPBData(&x, &y, i);
        _xShared.getDPBRotDone(&r, i);
        _xShared.getDPBTime(&t, i);
        printf("%d;%d;%llu;%d\n", x, y, t, r);
    }
#endif
}

void DPB::_log_acc_data_filtered(void)
{
#ifdef APP_DEBUG_MODE
    int16_t x;
    int16_t y;
    uint8_t r;
    uint64_t t;
    ESP_LOGI(TAG, "Filtered accelerations data");

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        _xShared.getDPBDataFlt(&x, &y, i);
        _xShared.getDPBRotDone(&r, i);
        _xShared.getDPBTime(&t, i);
        printf("%d;%d;%llu;%d\n", x, y, t, r);
    }
#endif
}

float_t DPB::_get_fundamental_freq(uint16_t sample_freq, size_t fft_lenght)
{
    float_t fft_res = (float_t)sample_freq / (float_t)fft_lenght;
    float_t signal_fundamental = (float_t)_xShared.getFFTXMaxIndex();
    signal_fundamental *= fft_res;

    if (signal_fundamental == 0)
    {
        signal_fundamental = fft_res; // Min distance equal to FFT resolution
    }

    return signal_fundamental;
}

void DPB::_peaks_filter_by_distance(uint64_t *ref_array, size_t *ref_peaks, size_t *sorted_peaks, size_t peak_num, uint64_t req_distance)
{
    const char *TAG = "PEAKS-FILTER-DIST";
    size_t current = 0;
    u_short *to_remove = new u_short[peak_num]();

    for (size_t i = 0; i < peak_num; i++)
    {
        current = sorted_peaks[i];

        if (to_remove[current] == 1)
        {
            continue; // peak will already be removed, move on.
        }

        // check on left side of peak
        int16_t neighbor = current - 1;
        while (neighbor >= 0 && (ref_array[ref_peaks[current]] - ref_array[ref_peaks[neighbor]]) < req_distance)
        {
            to_remove[neighbor] = 1;
            --neighbor;
        }

        // check on right side of peak
        neighbor = current + 1;
        while (neighbor < peak_num && (ref_array[ref_peaks[current]] - ref_array[ref_peaks[neighbor]]) < req_distance)
        {
            to_remove[neighbor] = 1;
            ++neighbor;
        }
    }

    // Remove the incorrect peak indexs
    for (size_t i = 0; i < peak_num; i++)
    {
        if (to_remove[i] == 1)
        {
#ifdef APP_DEBUG_MODE
            ESP_LOGW(TAG, "Peak[%d] removed", i);
#endif
            ref_peaks[i] = -1;
        }
    }

    delete[] to_remove;
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