#ifndef INC_COMMON_DEF_H
#define INC_COMMON_DEF_H

#include <array>
#include <stdexcept>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/pulse_cnt.h"

#include "esp_dsp.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/
//!         PROJECT DEFINES         /
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/
//* Debug serial loggin enabler
#define APP_DEBUG_MODE

//* Accelerometer type
#define USE_BMX055 1
// #define USE_ADXL345 1

//* I/O
#define GPIO_LED_ERROR GPIO_NUM_2
#define GPIO_OPT_SENSOR GPIO_NUM_35
#define GPIO_ESC_OUT GPIO_NUM_4

//* Charts
#define ACC_CHART_POINT_COUNT ACC_DATA_BUFFER_SIZE / 2
#define ACCX_TRACE_COLOR lv_palette_main(LV_PALETTE_RED)
#define ACCY_TRACE_COLOR lv_palette_main(LV_PALETTE_BLUE)

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/
//!         PROJECT SETTINGS        /
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/
#define MOTOR_STARTUP_DELAY_MS 2000
#define GUI_REFRESH_DELAY_MS 15
#define ACC_DATA_BUFFER_SIZE 1024
#define FFT_DATA_BUFFER_SIZE ACC_DATA_BUFFER_SIZE / 2
#define DEFAULT_MEASURE_THROTTLE 175 /* 225~3600rpm, 175~2700rpm for a 1700kV motor */

//*******************************/
//*         GLOBAL DEFINES      */
//*******************************/
#define DEFAULT_PROP_NUM 3
#define DEFAULT_FUNC_TIMOUT pdTICKS_TO_MS(100)

//*******************************/
//*         TYPE DEFINES        */
//*******************************/
enum app_command_e
{
    APP_CMD,
    MOTOR_CMD,
    START_BUT_CMD,
    ACC_CHART_UPDATE_CMD,
    FFT_REQUEST_CMD,
    FFT_CHART_UPDATE_CMD,
    LPF_REQUEST_CMD,
};

enum app_steps_e
{
    IDLE,
    START_MOTOR,
    VIBES_REC,
    FILTERING,
    ANALYSING,
    DECEL,
};

enum data_orig_e
{
    RAW_DATA,
    FILTERED_DATA
};

union dpb_generic_u
{
    float_t f;
    uint64_t ull;
    int64_t ll;
};

struct command_data_t
{
    app_command_e command;
    dpb_generic_u value;
};

struct acc_data_f_t
{
    float_t acc_x_f;
    float_t acc_y_f;
    float_t acc_z_f;
};

struct acc_data_i_t
{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
};

template <size_t N>
struct dpb_acc_data_t
{
    int16_t acc_x[N];
    int16_t acc_y[N];
};

union gui_status_u
{
    uint32_t allBits;
    struct
    {
        unsigned chart_running : 1;
        unsigned nd1 : 1;
        unsigned nd2 : 1;
        unsigned nd3 : 1;
        unsigned nd4 : 1;
        unsigned nd5 : 1;
        unsigned nd6 : 1;
        unsigned nd7 : 1;
    };
};

//*******************************/
//*      CLASS DECLARATION      */
//*******************************/
template <typename T>
class RINGBuffer
{
public:
    // Constructor
    RINGBuffer(int capacity) : capacity_(capacity), head_(0), tail_(0)
    {
        m_mutex = xSemaphoreCreateMutex();
        __attribute__((aligned(16))) buffer_ = new T[capacity];
    }

    // Destructor
    ~RINGBuffer()
    {
        vSemaphoreDelete(m_mutex);
        delete[] buffer_;
    }

    // Add an element to the end of the buffer
    void push(T element)
    {
        if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (full())
            {
                // The buffer is full, so we need to remove the oldest element
                // to make space for the new element
                pop();
            }

            buffer_[tail_] = element;
            tail_ = (tail_ + 1) % capacity_;

            xSemaphoreGive(m_mutex);
        }
    }

    void push_unsafe(T element)
    {
        if (full())
        {
            // The buffer is full, so we need to remove the oldest element
            // to make space for the new element
            pop();
        }

        buffer_[tail_] = element;
        tail_ = (tail_ + 1) % capacity_;
    }

    // Remove and return the oldest element from the buffer
    T pop()
    {
        T element;

        if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (empty())
            {
                ESP_LOGE(TAG, "RINGBuffer is empty");
            }

            element = buffer_[head_];
            head_ = (head_ + 1) % capacity_;

            xSemaphoreGive(m_mutex);
        }
        return element;
    }

    // Return the oldest element from the buffer without removing it
    T front() const
    {
        if (empty())
        {
            ESP_LOGE(TAG, "RINGBuffer is empty");
        }

        return buffer_[head_];
    }

    // Remove all elements from the buffer
    void clear()
    {
        if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE)
        {
            head_ = tail_ = 0;
            xSemaphoreGive(m_mutex);
        }
    }

    // Return the number of elements in the buffer
    int size() const
    {
        if (tail_ >= head_)
        {
            return tail_ - head_;
        }
        else
        {
            return capacity_ - (head_ - tail_);
        }
    }

    // Return true if the buffer is empty, false otherwise
    bool empty() const
    {
        return head_ == tail_;
    }

    // Return true if the buffer is full, false otherwise
    bool full() const
    {
        return (tail_ + 1) % capacity_ == head_;
    }

private:
    const char *TAG = "RINGBuffer-CLASS";
    SemaphoreHandle_t m_mutex;
    T *buffer_;    // The buffer
    int capacity_; // The maximum number of elements the buffer can hold
    int head_;     // The index of the oldest element in the buffer
    int tail_;     // The index where the next element will be added
};

//**********************************/
//*         GLOBAL VARIABLES        /
//**********************************/
extern TaskHandle_t guiTaskHandle;
extern TaskHandle_t accelTaskHandle;
extern TaskHandle_t senseTaskHandle;
extern TaskHandle_t appTaskHandle;

#endif