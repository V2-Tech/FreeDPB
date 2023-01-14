#ifndef INC_APP_H
#define INC_APP_H

#include "../../main/common_def.h"
#include "../GUI/GUI.h"
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

    uint8_t init(QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, TaskHandle_t supportTask_handle);
    uint8_t init_rpm(TaskHandle_t supportTask_handle, QueueHandle_t xQueueSysOutput_handle, uint8_t n_propeller);
    uint8_t init_esc();
    uint8_t init_accel();

    void loop(void);
    void loop_rpm(void);
    void exe(command_data command);
    void start(void);
    void reset(void);
    void rec_timer_start(void);
    void fft(void);
    void filtering(void);

private:
    QueueHandle_t _xQueueSysInput;
    QueueHandle_t _xQueueSysOutput;
    TaskHandle_t _xSuppTask;

    uint16_t _init_status;
    uint8_t _init_done;
    TimerHandle_t _motorRunTimer;
};

#endif