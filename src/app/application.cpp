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

    __initNVS();

    ESP_LOGI(TAG, "Reading user settings...");
    _settings_read();

    ESP_LOGI(TAG, "Wait until all has been initialized");
    init_accel();
    init_esc();
    init_rpm(_xSuppTask, _rpmTimer, DEFAULT_PROP_NUM);

    /* Wait until all has been initialized */

    while (_init_status != (INIT_ESC_DONE | INIT_ACCEL_DONE | INIT_RPM_S_DONE | READ_SETTINGS_DONE))
        ;

    ESP_LOGI(TAG, "DPB intialization completed");
    ask_main_page();

    _init_done = 1;
    set_step(IDLE);
    _set_searchType(SEARCH_OPTICAL);

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
        // ToDo: implement error message in loading screen
        return ESP_FAIL;
    }
    if (Accel::set_range((uint8_t)_xShared.getRange()) != ESP_OK)
    {
        // ToDo: implement error message in loading screen
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
    _xShared.setRPM(RotSense::get_rpm());
}

void DPB::loop_accel(void)
{
    acc_data_i_t acc_data;
    size_t i;
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    uint8_t *pRotBuf = _xShared.getDPBRotDoneBuffer_us();
    uint64_t *pTimeBuf = _xShared.getDPBTimeBuffer_us();
    uint16_t rotCount = 0, lastRotCount = 0, _sample_cnt_360 = 0, _sample_cnt = 0;

    if (!_init_done)
    {
        return;
    };

    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    _app_step = VIBES_REC;
    i = 0;
    rotCount = lastRotCount = _sample_cnt = _sample_cnt_360 = 0;

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

            if (rotCount > 0)
            {
                _sample_cnt++;
            }
            if (rotCount != lastRotCount)
            {
                lastRotCount = rotCount;
                _sample_cnt_360 += _sample_cnt;
                _sample_cnt_360 /= rotCount;
            }

            i++;
        }
    }
    _xShared.unlockDPBDataAcc();

    _xShared.setRotCount(rotCount);
    _xShared.setUnbalanceErr(360.0 / (float_t)_sample_cnt_360);

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
            _exe_analyze();
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
        _exe_fft(command.value.ull);
        _exe_reset();
        break;

    case APP_STEP_CMD:
        _exe_step_managment((app_steps_e)command.value.ll);
        break;

    case FILTER_REQUEST_CMD:
        _exe_bpf(command.value.ull);
        _exe_reset();
        break;

    case APP_GET_SOURCE_CMD:
    case ACCEL_GET_BW_CMD:
    case ACCEL_GET_RANGE_CMD:
    case MOTOR_GET_SPEED_CMD:
    case IIR_GET_FREQ_CMD:
    case IIR_GET_Q_CMD:
        _exe_get_settings(command.command);
        break;

    case APP_SET_SOURCE_CMD:
    case ACCEL_SET_BW_CMD:
    case ACCEL_SET_RANGE_CMD:
    case MOTOR_SET_SPEED_CMD:
    case IIR_SET_FREQ_CMD:
    case IIR_SET_Q_CMD:
        _exe_set_settings(command);
        break;

    case STORE_SETTINGS_CMD:
        _exe_store_settings();
        break;
    default:
        break;
    }
}

void DPB::_exe_start(void)
{
    RotSense::reset_rpm_cnt();
    Motor::set_throttle(_xShared.getMeasureThrottle());
    if (xTimerReset(_motorStartupTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer start-command timeout");
    }
    set_step(START_MOTOR);
}

void DPB::_exe_reset(void)
{
    _reset();
}

void DPB::_reset(void)
{
    if (xTimerStop(_motorStartupTimer, DEFAULT_FUNC_TIMOUT) != pdTRUE)
    {
        esp_system_abort("Timer stop-command timeout");
    }
    Motor::set_throttle(0);
    RotSense::stop_rpm_cnt();
    if (_xShared.getSearchType() == SEARCH_4_STEPS)
    {
        ask_unbalance_step_1();
    }
    set_step(IDLE);
}

void DPB::_exe_filter(void)
{
    set_step(FILTERING);

    Motor::motor_stop();
    _log_acc_data();
    _unbalance_magnitude_calc();
    if (_filter_data_iir_zero(RAW_DATA) != ESP_OK)
    {
        // TODO Error message
        return;
    }
    //_dummy_data_remove();
    _log_acc_data_filtered();
    ask_acc_charts_update(); // ? Show filtered data
}

void DPB::_exe_analyze(void)
{
    set_step(ANALYSING);

    if (_fft_calc(FILTERED_DATA) != ESP_OK)
    {
        // TODO Error message
        return;
    }
    _fft_peak_finder();
    ask_fft_chart_update();

    if (_search_type == SEARCH_OPTICAL)
    {
        set_step(UNBALANCE_OPT_FINDING);
        _signal_peak_finder();
        _unbalance_finder_optical();
        _reset();
    }
    else
    {
        _unbalance_finder_steps();
    }
}

void DPB::_exe_fft(uint8_t data_type)
{
    if (_fft_calc((data_orig_e)data_type) != ESP_OK)
    {
        return;
    }
    _fft_peak_finder();
    ask_fft_chart_update();
}

void DPB::_exe_bpf(uint8_t data_type)
{
    set_step(FILTERING);

    if (_filter_data_iir_zero((data_orig_e)data_type) != ESP_OK)
    {
        return;
    }
    ask_acc_charts_update(); // ? Show filtered data
}

void DPB::_exe_step_managment(app_steps_e requested_step)
{
    // TODO
}

void DPB::_exe_get_settings(sys_command_e request)
{
    accel_settings_t acc_settings;
    int64_t v;

    switch (request)
    {
    case APP_GET_SOURCE_CMD:
        v = (int64_t)_xShared.getUnbalanceSource();
        ask_setting_update(request, v);
        break;

    case ACCEL_GET_BW_CMD:
        Accel::get_acc_settings(&acc_settings);
        v = (int64_t)acc_settings.band;
        ask_setting_update(request, v);
        break;

    case ACCEL_GET_RANGE_CMD:
        Accel::get_acc_settings(&acc_settings);
        v = (int64_t)_range_2_gui_value_convert(acc_settings.range);
        ask_setting_update(request, v);
        break;

    case MOTOR_GET_SPEED_CMD:
        v = (int64_t)_xShared.getMeasureThrottle();
        ask_setting_update(request, v);
        break;

    case IIR_GET_FREQ_CMD:
        v = (int64_t)(_xShared.getIIRCenterFreq() * 1000 * 2);
        ask_setting_update(request, v);
        break;

    case IIR_GET_Q_CMD:
        v = (int64_t)(_xShared.getIIRQFactor() * 100);
        ask_setting_update(request, v);
        break;

    default:
        break;
    }
}

void DPB::_exe_set_settings(command_data_t commnad)
{
    switch (commnad.command)
    {
    case APP_SET_SOURCE_CMD:
        _xShared.setUnbalanceSource((app_unbalance_source_e)commnad.value.ull);
        break;

    case ACCEL_SET_BW_CMD:
        break;

    case ACCEL_SET_RANGE_CMD:
        if (Accel::set_range((uint8_t)(1U << (commnad.value.ull + 1))) == ESP_OK)
        {
            _xShared.setRange((uint16_t)(1U << (commnad.value.ull + 1)));
        }
        break;

    case MOTOR_SET_SPEED_CMD:
        _xShared.setMeasureThrottle((uint16_t)commnad.value.ull);
        break;

    case IIR_SET_FREQ_CMD:
        _xShared.setIIRCenterFreq(((float_t)commnad.value.ll / 1000.0) * 0.5);
        break;

    case IIR_SET_Q_CMD:
        _xShared.setIIRQFactor((float_t)commnad.value.ll / 100.0);
        break;

    default:
        break;
    }
}

void DPB::_exe_store_settings(void)
{
    ESP_LOGI(TAG, "Saving user settings...");
    _settings_write();
}

void DPB::ask_acc_charts_update(void)
{
    command_data_t command;

    command.command = GUI_ACC_CHART_UPDATE_CMD;
    command.value.ull = RAW_DATA;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_fft_chart_update(void)
{
    command_data_t command;

    command.command = GUI_FFT_CHART_UPDATE_CMD;
    command.value.ull = 1;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_unbalance_arrow_update(void)
{
    command_data_t command;

    command.command = GUI_UNBALANCE_UPDATE_CMD;
    command.value.ull = GUI_UNBALANCE_OPT;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_unbalance_step_1(void)
{
    command_data_t command;

    command.command = GUI_UNBALANCE_UPDATE_CMD;
    command.value.ull = GUI_UNBALANCE_STEP_1;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_unbalance_step_2(void)
{
    command_data_t command;

    command.command = GUI_UNBALANCE_UPDATE_CMD;
    command.value.ull = GUI_UNBALANCE_STEP_2;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_unbalance_step_3(void)
{
    command_data_t command;

    command.command = GUI_UNBALANCE_UPDATE_CMD;
    command.value.ull = GUI_UNBALANCE_STEP_3;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_unbalance_step_4(void)
{
    command_data_t command;

    command.command = GUI_UNBALANCE_UPDATE_CMD;
    command.value.ull = GUI_UNBALANCE_STEP_4;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_main_page(void)
{
    command_data_t command;

    command.command = GUI_INIT_COMPLETED_CMD;
    command.value.ull = 1;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::ask_setting_update(sys_command_e request, int64_t value)
{
    command_data_t command;

    command.command = request;
    command.value.ll = value;
    xQueueSend(_xQueueSysOutput, &command, portMAX_DELAY);
}

void DPB::set_step(app_steps_e v)
{
    _app_step = v;
    _xShared.setAppStatus(v);
}

void DPB::_unbalance_magnitude_calc(void)
{
    int16_t maxX = 0, maxY = 0, minX = 0, minY = 0;

    _xShared.lockDPBDataAcc();

    _analyzer.array_max_min_finder<int16_t>(_xShared.getDPBDataAccXBuffer_us(), ACC_DATA_BUFFER_SIZE, &maxX, &minX);
    _analyzer.array_max_min_finder<int16_t>(_xShared.getDPBDataAccYBuffer_us(), ACC_DATA_BUFFER_SIZE, &maxY, &minY);

    _xShared.unlockDPBDataAcc();

    _xShared.setUnbalanceMag((maxX - minX) + (maxY - minY));
}

int16_t DPB::_filter_data_iir_zero(data_orig_e data_type)
{
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    int16_t *pAccXFltBuf = _xShared.getDPBDataFltAccXBuffer_us();
    int16_t *pAccYFltBuf = _xShared.getDPBDataFltAccYBuffer_us();

    __attribute__((aligned(16))) static float accX_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_input[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accX_filtered[ACC_DATA_BUFFER_SIZE];
    __attribute__((aligned(16))) static float accY_filtered[ACC_DATA_BUFFER_SIZE];

    float_t filter_coeffs[5];
    float_t w_lpf[5] = {0, 0};
    float_t filter_Q[2] = {_xShared.getIIRQFactor(), _xShared.getIIRQFactor()};
    float_t filter_freq = _xShared.getIIRCenterFreq();

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

    //* BAND-PASS FILTERING
    // Calculate iir filter coefficients
    if (dsps_biquad_gen_bpf_f32(filter_coeffs, filter_freq, filter_Q[0]) != ESP_OK)
    {
        ESP_LOGE(TAG, "IIR coefs calc error");
        return ESP_FAIL;
    }

    // Filter X signal
    if (dsps_biquad_f32(accX_input, accX_filtered, ACC_DATA_BUFFER_SIZE, filter_coeffs, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }
    // Filter Y signal
    if (dsps_biquad_f32(accY_input, accY_filtered, ACC_DATA_BUFFER_SIZE, filter_coeffs, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        accX_input[i] = accX_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
        accY_input[i] = accY_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
    }

    // Calculate iir filter coefficients
    if (dsps_biquad_gen_bpf_f32(filter_coeffs, filter_freq, filter_Q[1]) != ESP_OK)
    {
        ESP_LOGE(TAG, "IIR coefs calc error");
        return ESP_FAIL;
    }
    // Filter X signal
    if (dsps_biquad_f32(accX_input, accX_filtered, ACC_DATA_BUFFER_SIZE, filter_coeffs, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }
    // Filter Y signal
    if (dsps_biquad_f32(accY_input, accY_filtered, ACC_DATA_BUFFER_SIZE, filter_coeffs, w_lpf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Filtering operation error");
        return ESP_FAIL;
    }

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        accX_input[i] = accX_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
        accY_input[i] = accY_filtered[ACC_DATA_BUFFER_SIZE - 1 - i];
    }

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
    {
        pAccXFltBuf[i] = accX_input[i];
        pAccYFltBuf[i] = accY_input[i];
    }

    _xShared.unlockDPBDataAcc();
    _xShared.unlockDPBDataFltAcc();

    return ESP_OK;
}

int16_t DPB::_fft_calc(data_orig_e data_type)
{
    int16_t *pAccXBuf = _xShared.getDPBDataAccXBuffer_us();
    int16_t *pAccYBuf = _xShared.getDPBDataAccYBuffer_us();
    int16_t *pAccXFltBuf = _xShared.getDPBDataFltAccXBuffer_us();
    int16_t *pAccYFltBuf = _xShared.getDPBDataFltAccYBuffer_us();
    float_t *pFFTXBuf = _xShared.getFFTXBuffer_us();
    float_t *pFFTYBuf = _xShared.getFFTYBuffer_us();

    set_step(ANALYSING);

    int32_t N = ACC_DATA_BUFFER_SIZE;
    esp_err_t ret = ESP_OK;

    // Window coefficients
    __attribute__((aligned(16))) static float_t wind[ACC_DATA_BUFFER_SIZE];
    // working complex array
    __attribute__((aligned(16))) static float_t input_cf[ACC_DATA_BUFFER_SIZE * 2];

    // Init FFT filter
    if (dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE) != ESP_OK)
    {
        char *TAG = "app_fft";
        ESP_LOGE(TAG, "Not possible to initialize FFT");
        return ESP_FAIL;
    }

    // Generate hann window
    dsps_wind_hann_f32(wind, N);

    for (size_t j = 0; j < 2; j++)
    {
        _xShared.lockDPBDataAcc();
        _xShared.lockDPBDataFltAcc();

        // Convert input vectors to complex vectors
        for (size_t i = 0; i < N; i++)
        {
            if (data_type == RAW_DATA)
            {
                if (j == 0)
                {
                    input_cf[i * 2 + 0] = pAccXBuf[i] * wind[i];
                    input_cf[i * 2 + 1] = 0;
                }
                else
                {
                    input_cf[i * 2 + 0] = pAccYBuf[i] * wind[i];
                    input_cf[i * 2 + 1] = 0;
                }
            }
            else
            {
                if (j == 0)
                {
                    input_cf[i * 2 + 0] = pAccXFltBuf[i] * wind[i];
                    input_cf[i * 2 + 1] = 0;
                }
                else
                {
                    input_cf[i * 2 + 0] = pAccYFltBuf[i] * wind[i];
                    input_cf[i * 2 + 1] = 0;
                }
            }
        }

        _xShared.unlockDPBDataAcc();
        _xShared.unlockDPBDataFltAcc();

        // FFT X-axes
        ret += dsps_fft2r_fc32(input_cf, N);
        // Bit reverse
        ret += dsps_bit_rev_fc32(input_cf, N);
        // Convert one complex vector to two complex vectors
        ret += dsps_cplx2reC_fc32(input_cf, N);

        if (ret != ESP_OK)
        {
            char *TAG = "app_fft";
            ESP_LOGE(TAG, "FFT calc error");
            return ESP_FAIL;
        }

        _xShared.lockFFT();

        // Make module and normalize it
        for (size_t i = 0; i < N / 2; i++)
        {
            if (j == 0)
            {
                //? I scale by 2 to make sure i dont lose too much precision when i convert the data to int16 for the FFT chart
                pFFTXBuf[i] = 2 * sqrtf((input_cf[i * 2 + 0] * input_cf[i * 2 + 0] + input_cf[i * 2 + 1] * input_cf[i * 2 + 1] + 0.0000001) / N);
            }
            else
            {
                //? I scale by 2 to make sure i dont lose too much precision when i convert the data to int16 for the FFT chart
                pFFTYBuf[i] = 2 * sqrtf((input_cf[i * 2 + 0] * input_cf[i * 2 + 0] + input_cf[i * 2 + 1] * input_cf[i * 2 + 1] + 0.0000001) / N);
            }
        }

        _xShared.unlockFFT();
    }

    return ESP_OK;
}

void DPB::_signal_peak_finder(void)
{
#ifndef APP_Z_SCORES_PEAKS
    _standard_peaks_analisys();
#else
    _z_scores_peaks_analisys();
#endif
}

void DPB::_fft_peak_finder(void)
{
    float_t *pFFTXBuf = _xShared.getFFTXBuffer_us();
    float_t *pFFTYBuf = _xShared.getFFTYBuffer_us();

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

void DPB::_dummy_data_remove(void)
{
    //* Remove the first X values of filtered data that present fake oscillations
    _xShared.lockDPBDataFltAcc();
    _xShared.lockDPBDataAcc();

    ESP_ERROR_CHECK(_analyzer.array_cutter<int16_t>(_xShared.getDPBDataFltAccXBuffer_us(), ACC_DATA_BUFFER_SIZE, DUMMY_DATA_QUANTITY, ACC_DATA_BUFFER_SIZE - DUMMY_DATA_QUANTITY));
    ESP_ERROR_CHECK(_analyzer.array_cutter<int16_t>(_xShared.getDPBDataFltAccYBuffer_us(), ACC_DATA_BUFFER_SIZE, DUMMY_DATA_QUANTITY, ACC_DATA_BUFFER_SIZE - DUMMY_DATA_QUANTITY));
    ESP_ERROR_CHECK(_analyzer.array_cutter<uint8_t>(_xShared.getDPBRotDoneBuffer_us(), ACC_DATA_BUFFER_SIZE, DUMMY_DATA_QUANTITY, ACC_DATA_BUFFER_SIZE - DUMMY_DATA_QUANTITY));
    ESP_ERROR_CHECK(_analyzer.array_cutter<uint64_t>(_xShared.getDPBTimeBuffer_us(), ACC_DATA_BUFFER_SIZE, DUMMY_DATA_QUANTITY, ACC_DATA_BUFFER_SIZE - DUMMY_DATA_QUANTITY));

    _xShared.unlockDPBDataFltAcc();
    _xShared.unlockDPBDataAcc();
}

void DPB::_set_searchType(app_search_type_e type)
{
    _search_type = type;
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

void DPB::_standard_peaks_analisys(void)
{
#ifndef APP_Z_SCORES_PEAKS

    size_t maxXPeakIndex = 0, maxYPeakIndex = 0;

    //* Find all local peaks index and absolute max peak index
    _xShared.lockDPBDataFltAcc();
    _xShared.lockPeaksIndex();

#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "X-Peak");
#endif
    ESP_ERROR_CHECK(_analyzer.peaks_finder<int16_t>(_xShared.getDPBDataFltAccXBuffer_us(), _xShared.getXPeaksIndexPointer_us(), &_XpeakCount, &maxXPeakIndex, ACC_DATA_BUFFER_SIZE, ACC_DATA_BUFFER_SIZE));
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Y-Peak");
#endif
    ESP_ERROR_CHECK(_analyzer.peaks_finder<int16_t>(_xShared.getDPBDataFltAccYBuffer_us(), _xShared.getYPeaksIndexPointer_us(), &_YpeakCount, &maxYPeakIndex, ACC_DATA_BUFFER_SIZE, ACC_DATA_BUFFER_SIZE));
    _xShared.setAccXMaxIndex(maxXPeakIndex);
    _xShared.setAccYMaxIndex(maxYPeakIndex);

    _xShared.unlockDPBDataFltAcc();
    _xShared.unlockPeaksIndex();

    //* Calc min distance between two real local peak
    float_t fund = _get_fundamental_freq(_xShared.getBandWidth(), FFT_DATA_BUFFER_SIZE, _xShared.getUnbalanceSource());
    float_t peak_min_dist = ((0.8 * 1000000.0) / fund); //! _rpmTimer has 1us resolution
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Fundamental frequency: %.1f", fund);
    ESP_LOGI(TAG, "Peaks min distance: %.1f", peak_min_dist);
#endif
    _xShared.setUnbalanceFreq(fund);

    //* Remove peak indexs that dont meet the min required distance
    int16_t *height_sorted_Xpeak_index = new int16_t[_XpeakCount](); //? Index of peak index
    int16_t *height_sorted_Ypeak_index = new int16_t[_YpeakCount](); //? Index of peak index

    _xShared.lockDPBDataAcc();
    _xShared.lockPeaksIndex();
    ESP_ERROR_CHECK(_analyzer.array_map_incr<int16_t>(_xShared.getXPeaksIndexPointer_us(), _xShared.getDPBDataFltAccXBuffer_us(), height_sorted_Xpeak_index, _XpeakCount));
    ESP_ERROR_CHECK(_analyzer.array_map_incr<int16_t>(_xShared.getYPeaksIndexPointer_us(), _xShared.getDPBDataFltAccYBuffer_us(), height_sorted_Ypeak_index, _YpeakCount));

#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Filter X-Peak");
#endif
    _analyzer.peaks_filter_by_distance<uint64_t>(_xShared.getDPBTimeBuffer_us(), _xShared.getXPeaksIndexPointer_us(), height_sorted_Xpeak_index, _XpeakCount, peak_min_dist);
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Filter Y-Peak");
#endif
    _analyzer.peaks_filter_by_distance<uint64_t>(_xShared.getDPBTimeBuffer_us(), _xShared.getYPeaksIndexPointer_us(), height_sorted_Ypeak_index, _YpeakCount, peak_min_dist);
    _xShared.unlockDPBDataAcc();

    ESP_ERROR_CHECK(_analyzer.array_value_remover<int16_t>(_xShared.getXPeaksIndexPointer_us(), &_XpeakCount, (size_t)-1));
    ESP_ERROR_CHECK(_analyzer.array_value_remover<int16_t>(_xShared.getYPeaksIndexPointer_us(), &_YpeakCount, (size_t)-1));
    _xShared.unlockPeaksIndex();

    _xShared.setXPeakCount(_XpeakCount);
    _xShared.setYPeakCount(_YpeakCount);

    delete[] height_sorted_Xpeak_index;
    delete[] height_sorted_Ypeak_index;
#endif
}

void DPB::_z_scores_peaks_analisys(void)
{
#ifdef APP_Z_SCORES_PEAKS
    size_t maxXPeakIndex = 0, maxYPeakIndex = 0;

    _xShared.lockDPBDataFltAcc();
    _xShared.lockPeaksIndex();

#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "X-Peak");
#endif
    __peaks_finder_z_scores<int16_t>(_xShared.getDPBDataFltAccXBuffer_us(), _xShared.getXPeaksIndexPointer_us(), 10, 5, 0.35, &_XpeakCount, &maxXPeakIndex, ACC_DATA_BUFFER_SIZE, ACC_DATA_BUFFER_SIZE);
#ifdef APP_DEBUG_MODE
    ESP_LOGI(TAG, "Y-Peak");
#endif
    __peaks_finder_z_scores<int16_t>(_xShared.getDPBDataFltAccYBuffer_us(), _xShared.getYPeaksIndexPointer_us(), 10, 2.5, 0.5, &_YpeakCount, &maxYPeakIndex, ACC_DATA_BUFFER_SIZE, ACC_DATA_BUFFER_SIZE);

    _xShared.unlockDPBDataFltAcc();
    _xShared.unlockPeaksIndex();

#endif
}

void DPB::_unbalance_finder_optical(void)
{
    static const char *TAG = "EXE-UNBALANCE";
    int64_t averageX = 0;
    int64_t averageY = 0;
    uint16_t averageQuantity = 0;
    int64_t lastDelta = 0;
    size_t maxLoops = 0;
    uint8_t *pRotDoneBuf = _xShared.getDPBRotDoneBuffer_us();
    uint64_t *pTimeBuf = _xShared.getDPBTimeBuffer_us();
    int16_t *pXPeakIndexBuf = _xShared.getXPeaksIndexPointer_us();
    int16_t *pYPeakIndexBuf = _xShared.getYPeaksIndexPointer_us();

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
        if ((lastDelta < 0 && currDelta >= 0) || (lastDelta > 0 && currDelta < 0))
        {
            //? It s not possible to have a sign inversion: some data are missing: shift ahead.
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
        if ((lastDelta < 0 && currDelta >= 0) || (lastDelta > 0 && currDelta < 0))
        {
            //? It s not possible to have a sign inversion: some data are missing: shift ahead.
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

    _xShared.setUnbalanceXAngle((float_t)(averageX / count2deg));
    _xShared.setUnbalanceYAngle((float_t)(averageY / count2deg));

    delete[] rotDoneIndexBuf;

    ask_unbalance_arrow_update();
}

void DPB::_unbalance_finder_steps(void)
{
    switch (_app_step)
    {
    case ANALYSING:
        set_step(UNBALANCE_STEP_1);
        _unbalance_step_1();
        ask_unbalance_step_2();
        break;

    case UNBALANCE_STEP_1:
        set_step(UNBALANCE_STEP_2);
        _unbalance_step_2();
        ask_unbalance_step_3();
        break;

    case UNBALANCE_STEP_2:
        set_step(UNBALANCE_STEP_3);
        _unbalance_step_3();
        ask_unbalance_step_4();
        break;

    case UNBALANCE_STEP_3:
        set_step(UNBALANCE_STEP_4);
        _unbalance_step_4();
        ask_unbalance_step_4();
        _reset();
        break;

    default:
        break;
    }
}

void DPB::_unbalance_step_1(void)
{
    _steps_amplitude[0] = _get_vibe_vector_mod();
}

void DPB::_unbalance_step_2(void)
{
    _steps_amplitude[1] = _get_vibe_vector_mod();
}

void DPB::_unbalance_step_3(void)
{
    _steps_amplitude[2] = _get_vibe_vector_mod();
}

void DPB::_unbalance_step_4(void)
{
    _steps_amplitude[3] = _get_vibe_vector_mod();
}

void DPB::_settings_read(void)
{
    esp_err_t err;
    app_settings_t *settings = new (app_settings_t);

    err = _settings_load(settings);
    if (err == ESP_OK)
    {
        _xShared.setRange(settings->range);
        _xShared.setMeasureThrottle(settings->measureThrottle);
        _xShared.setIIRQFactor(settings->iirQFactor);
        _xShared.setIIRCenterFreq(settings->iirCenterFreq);
        _xShared.setUnbalanceSource(settings->unbalanceSource);
    }
    else
    {
        printf("Error (%s) reading data from NVS!\n", esp_err_to_name(err));
    }

    _init_status |= READ_SETTINGS_DONE;

    delete settings;
}

int16_t DPB::_settings_load(app_settings_t *settings)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open nvs partition
    err = nvs_open_from_partition("user_data", USER_SETTINGS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening the NVS 'user_data' partition: %s", esp_err_to_name(err));
        return err;
    }

    // Write value including previously saved blob if available
    size_t required_size = sizeof(app_settings_t);
    err = nvs_get_blob(my_handle, "settings", settings, &required_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error reading data to NVS 'user_data' partition: %s", esp_err_to_name(err));
        return err;
    }

    // Close
    nvs_close(my_handle);

    return ESP_OK;
}

void DPB::_settings_write(void)
{
    app_settings_t settings;

    settings.unbalanceSource = _xShared.getUnbalanceSource();
    settings.measureThrottle = _xShared.getMeasureThrottle();
    settings.range = _xShared.getRange();
    settings.iirQFactor = _xShared.getIIRQFactor();
    settings.iirCenterFreq = _xShared.getIIRCenterFreq();

    _settings_save(&settings);
}

int16_t DPB::_settings_save(app_settings_t *settings)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open nvs partition
    err = nvs_open_from_partition("user_data", USER_SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening the NVS 'user_data' partition: %s", esp_err_to_name(err));
        return err;
    }

    // Write value including previously saved blob if available
    size_t required_size = sizeof(app_settings_t);
    err = nvs_set_blob(my_handle, "settings", settings, required_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error saving dato to NVS 'user_data' partition: %s", esp_err_to_name(err));
        return err;
    }

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error committing the changes to NVS 'user_data' partition: %s", esp_err_to_name(err));
        return err;
    }

    // Close
    nvs_close(my_handle);

    return ESP_OK;
}

float_t DPB::_get_fundamental_freq(uint16_t sample_freq, size_t fft_length, app_unbalance_source_e axis)
{
    static float_t signal_fundamental = 0.0;
    static float_t fft_res = 0.0;

    fft_res = (float_t)sample_freq / (float_t)fft_length;
    if (axis == X_AXIS_SOURCE)
    {
        signal_fundamental = (float_t)_xShared.getFFTXMaxIndex();
    }
    else
    {
        signal_fundamental = (float_t)_xShared.getFFTYMaxIndex();
    }

    signal_fundamental *= fft_res;

    if (signal_fundamental == 0.0)
    {
        signal_fundamental = fft_res; // Min distance equal to FFT resolution
    }

    return signal_fundamental;
}

float_t DPB::_get_vibe_vector_mod(void)
{
    int16_t min_x = 0;
    int16_t max_x = 0;
    int16_t min_y = 0;
    int16_t max_y = 0;
    float_t mod = 0;
    int16_t *pAccXBuff = nullptr;
    int16_t *pAccYBuff = nullptr;

    _xShared.lockDPBDataFltAcc();
    pAccXBuff = _xShared.getDPBDataFltAccXBuffer_us();
    pAccYBuff = _xShared.getDPBDataFltAccYBuffer_us();

    _analyzer.array_max_min_finder<int16_t>(pAccXBuff, ACC_DATA_BUFFER_SIZE, &max_x, &min_x);
    _analyzer.array_max_min_finder<int16_t>(pAccYBuff, ACC_DATA_BUFFER_SIZE, &max_y, &min_y);

    _xShared.unlockDPBDataFltAcc();

    mod = sqrt(((max_x - min_x) * (max_x - min_x)) + ((max_y - min_y) * (max_y - min_y)));

    return mod;
}

int8_t DPB::_range_2_gui_value_convert(uint8_t range)
{
    uint8_t value = dpb_range_e::RANGE_UNKNOW;

    switch (range)
    {
    case 2:
        value = dpb_range_e::RANGE_2G;
        break;
    case 4:
        value = dpb_range_e::RANGE_4G;
        break;
    case 8:
        value = dpb_range_e::RANGE_8G;
        break;
    case 16:
        value = dpb_range_e::RANGE_16G;
        break;
    default:
        value = dpb_range_e::RANGE_UNKNOW;
        break;
    }

    return value;
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

void DPB::__initNVS(void)
{
    esp_err_t err = nvs_flash_init_partition("user_data");
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase_partition("user_data"));
        err = nvs_flash_init_partition("user_data");
    }
    ESP_ERROR_CHECK(err);
}