#ifndef INC_POSITION_H
#define INC_POSITION_H

#include "../common/common_def.h"
#include "../shared/shared_data.h"

//*******************************/
//*      CLASS DECLARATION       /
//*******************************/
class RotSense
{
public:
    RotSense(gpio_num_t opto_gpio_num);
    ~RotSense();

    uint8_t init_rpm_sensor(TaskHandle_t Task_handle, gptimer_handle_t xTimer_handle, uint8_t n_propeller);
    void rpm_update();

    float_t get_rpm();
    void reset_rpm_cnt();
    void stop_rpm_cnt();

    uint8_t get_rotation_done();
    void reset_rotation_done();

    void set_propeller(uint8_t n_propeller);
    uint8_t get_propeller();

private:
    gpio_num_t _in_GPIO;
    uint16_t _rpm;
    uint8_t _rot_done;
    uint8_t _prop_number;
    
    void _set_rotation_done();

protected:
    static bool __opto_isr_handler_static(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx);
    void __opto_isr_handler();
    gptimer_handle_t __xGPTimer;
    pcnt_unit_handle_t __rpm_pcnt;
    TaskHandle_t __task_calc;

    SemaphoreHandle_t __xSem;
};

#endif