#ifndef INC_APP_H
#define INC_APP_H

#include "../shared/common_def.h"
#include "../shared/shared_data.h"

#include "../sensors/accelerations.h"
#include "../motor/motor.h"
#include "../sensors/position.h"

/************************************/
/*      FUNCTION DECLARATIONS       */
/************************************/
class DPB : public Motor, public RotSense, public Accel
{
public:
    DPB(uint8_t escGPIO, dshot_mode_t escSpeed, gpio_num_t rotSensorGPIO, BMX055 *accel);

    uint8_t init(QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, TaskHandle_t supportTask_handle, DPBShared *sharedData);
    uint8_t init_rpm(TaskHandle_t supportTask_handle, gptimer_handle_t xTimer_handle, uint8_t n_propeller);
    uint8_t init_esc();
    uint8_t init_accel();

    void loop(void);
    void loop_rpm(void);
    void loop_accel(void);
    void exe(command_data command);
    void start(void);
    void reset(void);
    void ask_acc_charts_update(void);
    void ask_fft_chart_update(void);
    uint8_t filter_data_iir(data_orig data_type);
    uint8_t fft_calc(data_orig data_type);
    void signal_peak_finder(void);
    void fft_peak_finder(void);

    void setStep(app_steps v);

private:
    QueueHandle_t _xQueueSysInput;
    QueueHandle_t _xQueueSysOutput;
    TaskHandle_t _xSuppTask;
    DPBShared &_xShared = DPBShared::getInstance();

    uint16_t _init_status;
    uint8_t _init_done;
    app_steps _app_step;
    TimerHandle_t _motorStartupTimer;
    gptimer_handle_t _rpmTimer;
    uint8_t _peakCount;

    void _log_acc_data(void);
    void _log_acc_data_filtered(void);
    void _sort_index_by_height(size_t *output, size_t indexCount);
    void _array_value_remover(int16_t *array, size_t arraySize, int16_t target);

protected:
    static void __motorStartupTimerCallback_static(TimerHandle_t pxTimer);
    void __motorStartupTimerCallback(TimerHandle_t pxTimer);
};

#endif