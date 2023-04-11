#pragma once

#include <stdio.h>
#include <math.h>

#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/pulse_cnt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//*******************************/
//*      CLASS DECLARATION       /
//*******************************/
class RPMTracker
{
public:
    RPMTracker(gpio_num_t opto_gpio_num);
    ~RPMTracker();

    uint8_t init(TaskHandle_t Task_handle, uint8_t n_propeller, void (*rotDoneCallback)(void));
    uint8_t init(TaskHandle_t Task_handle, uint8_t n_propeller);
    void set_init_status(uint8_t status);
    uint8_t get_init_status();

    void loop();

    float_t get_rpm();
    uint8_t get_time(uint64_t& time);

    void start_rpm_cnt();
    void reset_rpm_cnt();
    void stop_rpm_cnt();

    uint8_t get_rotation_done();
    void reset_rotation_done();

    void set_propeller(uint8_t n_propeller);
    uint8_t get_propeller();

private:
    gpio_num_t _in_GPIO;
    gptimer_handle_t _rpmTimer;
    uint8_t _init_done;
    uint16_t _rpm;
    uint8_t _rot_done;
    uint8_t _prop_number;
    void (*_rotCatchedCallback)(void) = 0;

    void _basic_init();
    void _rpm_update();
    void _set_rotation_done();

protected:
    static bool __opto_isr_handler_static(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx);
    void __opto_isr_handler();
    pcnt_unit_handle_t __rpm_pcnt;
    TaskHandle_t __task_calc;

    SemaphoreHandle_t __xSem;
};