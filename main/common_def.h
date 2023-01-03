#ifndef INC_COMMON_DEF_H
#define INC_COMMON_DEF_H

#include <array>
#include <stdexcept>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "esp_system.h"
#include "driver/gpio.h"

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
#define VIBE_RECORD_TIME_MS 2000

/*!
 * @brief Enum to define commands
 */
enum app_command
{
    APP_CMD,
    MOTOR_CMD,
};

enum app_status
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
    int16_t accel_data[3];
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

/************************************/
/*      CLASSES DECLARATIONS        */
/************************************/
template <typename T>
class FIFOBuffer
{
public:
    // Constructor
    FIFOBuffer(int capacity) : capacity_(capacity), head_(0), tail_(0)
    {
        buffer_ = new T[capacity];
    }

    // Destructor
    ~FIFOBuffer()
    {
        delete[] buffer_;
    }

    // Add an element to the end of the buffer
    void push(T element)
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
        if (empty())
        {
            throw std::out_of_range("FIFOBuffer is empty");
        }

        T element = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        return element;
    }

    // Return the oldest element from the buffer without removing it
    T front() const
    {
        if (empty())
        {
            throw std::out_of_range("FIFOBuffer is empty");
        }

        return buffer_[head_];
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