#ifndef INC_POSITION_H
#define INC_POSITION_H

#include "../../main/common_def.h"

class RotSense
{
public:
    RotSense(gpio_num_t opto_gpio_num);
    ~RotSense();

private:
    gpio_num_t _in_GPIO;
    gptimer_handle_t _rpmTimer;
    static IRAM_ATTR uint8_t _isr_event_counter;

    static void opto_isr_handler_static(void *arg);
    void opto_isr_handler();
};

#endif