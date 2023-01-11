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

#include "esp_dsp.h"

#define APP_DEBUG_MODE

/************************/
/*      DEVICE CHIOSE   */
/************************/
#define USE_BMX055

/****************/
/*      I/O     */
/****************/
#define GPIO_LED_ERROR GPIO_NUM_2
#define GPIO_OPT_SENSOR GPIO_NUM_35

/*************************************/
/*      GLOBAL DEFINES               */
/*************************************/
#define VIBE_RECORD_TIME_MS 2000 //Not use actualy
#define ACC_DATA_BUFFER_SIZE 1024

/*!
 * @brief Enum to define commands
 */
enum app_command
{
    APP_CMD,
    MOTOR_CMD,
};

enum app_steps
{
    IDLE,
    POS_SEARCH,
    VIBES_REC,
    FILTERING,
    ANALYSING,
    DECEL,
};

struct command_data
{
    app_command command;
    uint8_t value;
};

/************************************/
/*      STRUCT DECLARATIONS         */
/************************************/

struct acc_sensor_data
{
#ifdef USE_BMX055
    float_t accel_data[3];
#endif
};

union gui_status
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

struct fft_chart_data
{
    float_t fft_data[ACC_DATA_BUFFER_SIZE];
};

/************************************/
/*      CLASSES DECLARATIONS        */
/************************************/
inline const char *TAG = "common";
template <typename T>
class FIFOBuffer
{
public:
    // Constructor
    FIFOBuffer(int capacity) : capacity_(capacity), head_(0), tail_(0)
    {
        m_mutex = xSemaphoreCreateMutex();
        buffer_ = new T[capacity];
    }

    // Destructor
    ~FIFOBuffer()
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

    // Remove and return the oldest element from the buffer
    T pop()
    {
        T element;

        if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (empty())
            {
                ESP_LOGE(TAG, "FIFOBuffer is empty");
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
            ESP_LOGE(TAG, "FIFOBuffer is empty");
        }

        return buffer_[head_];
    }

    // Remove all elements from the buffer
    void clear() const
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
    SemaphoreHandle_t m_mutex;
    T *buffer_;    // The buffer
    int capacity_; // The maximum number of elements the buffer can hold
    int head_;     // The index of the oldest element in the buffer
    int tail_;     // The index where the next element will be added
};

/*************************************/
/*          GLOBAL VARIABLES         */
/*************************************/
extern TaskHandle_t guiTaskHandle;
extern TaskHandle_t accelTaskHandle;
extern TaskHandle_t senseTaskHandle;
extern TaskHandle_t appTaskHandle;

#endif