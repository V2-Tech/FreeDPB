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
#include "esp_heap_caps.h"

#include "esp_dsp.h"
#include "nvs_flash.h"
#include "nvs.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/
//!         PROJECT DEFINES         /
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/
//* Compiler directives
// #define APP_DEBUG_MODE
// #define APP_Z_SCORES_PEAKS

//* I/O
#define ACC_SPI_MISO GPIO_NUM_32
#define ACC_SPI_MOSI GPIO_NUM_33
#define ACC_SPI_SCLK GPIO_NUM_25
#define ACC_SPI_CS GPIO_NUM_27
#define SPI_SPEED_BMX055 10 // Mhz
#define SPI_SPEED_ADXL345 4 // Mhz
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
#define DEFAULT_MEASURE_THROTTLE 225 /* 225~3600rpm, 175~2700rpm for a 1700kV motor */
#define DEFAULT_FILTER_C_FREQ 0.025F
#define DEFAULT_FILTER_Q_FACTOR 0.7071F
#define SECOND_FILTER_Q_FACTOR DEFAULT_FILTER_Q_FACTOR
#define DUMMY_DATA_QUANTITY 128
#define USER_SETTINGS_NAMESPACE "user_set"

//*******************************/
//*         GLOBAL DEFINES      */
//*******************************/
#define DEFAULT_PROP_NUM 3
#define DEFAULT_FUNC_TIMOUT pdTICKS_TO_MS(100)

//*******************************/
//*         TYPE DEFINES        */
//*******************************/
enum sys_command_e
{
    //* System operation
    APP_CMD,
    MOTOR_CMD,
    START_BUT_CMD,
    GUI_ACC_CHART_UPDATE_CMD,
    FFT_REQUEST_CMD,
    GUI_FFT_CHART_UPDATE_CMD,
    GUI_UNBALANCE_UPDATE_CMD,
    FILTER_REQUEST_CMD,
    SEARCH_TYPE_CMD,
    APP_STEP_CMD,
    STORE_SETTINGS_CMD,

    //* Settings
    APP_SET_ACC_MODEL_CMD,
    APP_SET_SOURCE_CMD,
    ACCEL_SET_BW_CMD,
    ACCEL_SET_RANGE_CMD,
    MOTOR_SET_SPEED_CMD,
    IIR_SET_FREQ_CMD,
    IIR_SET_Q_CMD,

    APP_GET_ACC_MODEL_CMD,
    APP_GET_SOURCE_CMD,
    ACCEL_GET_BW_CMD,
    ACCEL_GET_RANGE_CMD,
    MOTOR_GET_SPEED_CMD,
    IIR_GET_FREQ_CMD,
    IIR_GET_Q_CMD,

    //* Init
    GUI_INIT_COMPLETED_CMD, // ToDo
    GUI_INIT_ACC_CMD,       // ToDo
    GUI_INIT_RPM_CMD,       // ToDo
    GUI_INIT_ESC_CMD,       // ToDo
};

enum app_command_e
{
    SYS_START,
    SYS_STOP,
    SYS_RESET,
    SYS_ANALYZE_DATA,
};

enum app_steps_e
{
    PREVIOUS = -1,
    IDLE,
    START_MOTOR,
    VIBES_REC,
    FILTERING,
    ANALYSING,
    UNBALANCE_OPT_FINDING,
    UNBALANCE_STEP_1,
    UNBALANCE_STEP_2,
    UNBALANCE_STEP_3,
    UNBALANCE_STEP_4,
};

enum gui_unbalance_command_e
{
    GUI_UNBALANCE_OPT,
    GUI_UNBALANCE_STEP_1,
    GUI_UNBALANCE_STEP_2,
    GUI_UNBALANCE_STEP_3,
    GUI_UNBALANCE_STEP_4,
};

enum data_orig_e
{
    RAW_DATA,
    FILTERED_DATA
};

enum app_search_type_e
{
    SEARCH_OPTICAL,
    SEARCH_4_STEPS
};

enum app_unbalance_source_e
{
    X_AXIS_SOURCE,
    Y_AXIS_SOURCE
};

enum dpb_range_e
{
    RANGE_UNKNOW = -1,
    RANGE_2G,
    RANGE_4G,
    RANGE_8G,
    RANGE_16G,
};

enum acc_model_e
{
    MODEL_BMX055,
    MODEL_ADXL,
    MODEL_LSM6DS3,
};

union dpb_generic_u
{
    float_t f;
    uint64_t ull;
    int64_t ll;
};

struct command_data_t
{
    sys_command_e command;
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

struct app_settings_t
{
    acc_model_e device_model;
    uint16_t range;
    uint16_t bandWidth;
    app_unbalance_source_e unbalanceSource;
    uint16_t measureThrottle;
    float_t iirCenterFreq;
    float_t iirQFactor;
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
extern TaskHandle_t sensorTaskHandle;
extern TaskHandle_t appTaskHandle;

#endif