#ifndef INC_APP_H
#define INC_APP_H

#include "../common/common_def.h"
#include "../shared/shared_data.h"
#include "../common/SignalProcessing.h"

#include "../sensors/accelerations.h"
#include "../motor/motor.h"
#include "../sensors/position.h"

//************************/
//*      DEFINES         */
//************************/
#define INIT_ISR_DONE 1U << 1
#define INIT_ESC_DONE 1U << 2
#define INIT_ACCEL_DONE 1U << 3
#define INIT_RPM_S_DONE 1U << 4

//*******************************/
//*      CLASS DECLARATION      */
//*******************************/
class DPB : public Motor, public RotSense, public Accel
{
public:
    DPB(uint8_t escGPIO, dshot_mode_t escSpeed, gpio_num_t rotSensorGPIO, BMX055 *accel);
    DPB(uint8_t escGPIO, dshot_mode_t escSpeed, gpio_num_t rotSensorGPIO, ADXL345 *accel);

    int16_t init(QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, TaskHandle_t supportTask_handle, DPBShared *sharedData);
    int16_t init_rpm(TaskHandle_t supportTask_handle, gptimer_handle_t xTimer_handle, uint8_t n_propeller);
    int16_t init_esc();
    int16_t init_accel();

    void loop(void);
    void loop_rpm(void);
    void loop_accel(void);
    void exe(command_data_t command);
    void ask_acc_charts_update(void);
    void ask_fft_chart_update(void);
    void ask_unbalance_arrow_update(void);
    void ask_unbalance_step_1(void);
    void ask_unbalance_step_2(void);
    void ask_unbalance_step_3(void);
    void ask_unbalance_step_4(void);
    void ask_main_page(void);
    void ask_setting_update(sys_command_e request, int64_t value);

    void set_step(app_steps_e v);

private:
    QueueHandle_t _xQueueSysInput;
    QueueHandle_t _xQueueSysOutput;
    TaskHandle_t _xSuppTask;
    DPBShared &_xShared = DPBShared::getInstance();
    SignalProcessing _analyzer;

    uint16_t _init_status;
    uint8_t _init_done;
    app_steps_e _app_step;
    TimerHandle_t _motorStartupTimer;
    gptimer_handle_t _rpmTimer;
    size_t _XpeakCount;
    size_t _YpeakCount;
    app_search_type_e _search_type;
    float_t _steps_amplitude[4] = {0};

    void _exe_start(void);
    void _exe_reset(void);
    void _exe_filter(void);
    void _exe_analyze(void);
    void _exe_fft(uint8_t data_type);
    void _exe_bpf(uint8_t data_type);
    void _exe_step_managment(app_steps_e requested_step);
    void _exe_get_settings(sys_command_e request);
    void _exe_set_settings(command_data_t commnad);

    void _reset(void);
    void _unbalance_magnitude_calc(void);
    int16_t _filter_data_iir_zero(data_orig_e data_type);
    int16_t _fft_calc(data_orig_e data_type);
    void _signal_peak_finder(void);
    void _fft_peak_finder(void);
    void _dummy_data_remove(void);
    
    void _set_searchType(app_search_type_e type);

    void _log_acc_data(void);
    void _log_acc_data_filtered(void);

    void _standard_peaks_analisys(void);
    void _z_scores_peaks_analisys(void);
    void _unbalance_finder_optical(void);
    void _unbalance_finder_steps(void);
    void _unbalance_step_1(void);
    void _unbalance_step_2(void);
    void _unbalance_step_3(void);
    void _unbalance_step_4(void);

    float_t _get_fundamental_freq(uint16_t sample_freq, size_t fft_lenght);
    float_t _get_vibe_vector_mod(void);
    int8_t _range_2_gui_value_convert(uint8_t range);

    static void __motorStartupTimerCallback_static(TimerHandle_t pxTimer);
    void __motorStartupTimerCallback(TimerHandle_t pxTimer);
};

#endif