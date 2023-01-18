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

    dpb_acc_data dpbAccBuff[1024];

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
    void ask_charts_update(void);
    uint8_t filter_data(void);
    uint8_t fft(void);
    void setStep(app_steps v);

    int8_t get_app_step();

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

    void log_acc_data(void);

protected:
    static void __motorStartupTimerCallback_static(TimerHandle_t pxTimer);
    void __motorStartupTimerCallback(TimerHandle_t pxTimer);
};

#endif