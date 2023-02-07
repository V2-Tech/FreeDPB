#ifndef INC_APP_H
#define INC_APP_H

#include "../common/common_def.h"
#include "../shared/shared_data.h"
#include "../common/utils.h"

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
    void ask_unbalance_update(void);
    void ask_main_page(void);
    int16_t filter_data_iir(data_orig_e data_type);
    int16_t filter_data_iir_zero(data_orig_e data_type);
    int16_t fft_calc(data_orig_e data_type);
    void signal_peak_finder(void);
    void fft_peak_finder(void);

    void setStep(app_steps_e v);

private:
    QueueHandle_t _xQueueSysInput;
    QueueHandle_t _xQueueSysOutput;
    TaskHandle_t _xSuppTask;
    DPBShared &_xShared = DPBShared::getInstance();

    uint16_t _init_status;
    uint8_t _init_done;
    app_steps_e _app_step;
    TimerHandle_t _motorStartupTimer;
    gptimer_handle_t _rpmTimer;
    size_t _XpeakCount;
    size_t _YpeakCount;

    void _exe_start(void);
    void _exe_reset(void);
    void _exe_filter(void);
    void _exe_analyzing(void);
    void _exe_unbalance_finder(void);
    void _exe_fft(void);
    void _exe_lpf(void);

    void _log_acc_data(void);
    void _log_acc_data_filtered(void);
    float_t _get_fundamental_freq(uint16_t sample_freq, size_t fft_lenght);
    void _peaks_filter_by_distance(uint64_t *ref_array, size_t *ref_peaks, size_t *sorted_peak, size_t peak_num, uint64_t req_distance);

protected:
    static void __motorStartupTimerCallback_static(TimerHandle_t pxTimer);
    void __motorStartupTimerCallback(TimerHandle_t pxTimer);
};

#endif