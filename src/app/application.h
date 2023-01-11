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
class DPB
{
public:
    uint8_t init(Motor *motor, gpio_num_t opto_gpio_num, QueueHandle_t xQueueSysInput_handle, QueueHandle_t xQueueSysOutput_handle, FIFOBuffer<acc_sensor_data> *pDataBuffer, fft_chart_data *pFFTOuput);
    void loop(void);
    void exe(command_data command);
    void start(void);
    void reset(void);
    void rec_timer_start(void);
    void fft(void);
    void filtering(void);

private:
    QueueHandle_t _xQueueSysInput;
    QueueHandle_t _xQueueSysOutput;
    FIFOBuffer<acc_sensor_data> *_pDataBuffer;
    fft_chart_data *_pFFTOuput;

    Motor *_pMotor;

    TimerHandle_t _vibeRecTimer;

    
};

#endif