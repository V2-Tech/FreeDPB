#pragma once

#include <array>
#include <stdexcept>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

class SignalProcessing
{
public:
    SignalProcessing(){};
    ~SignalProcessing(){};

    //! Template methods cannot be defined in a cpp file but they need to be declared and defined in the header file
    /// @brief Remove a particular value from an array and compact it again.
    /// @tparam T Input array variables type
    /// @param array Pointer to input array in which we need to remove every element with value equal to target
    /// @param arraySize Pointer to number of element to scan which will be updated according to the actual number of valid data present in the input list
    /// @param target Value to search and remove
    /// @return !=0 if some errors happend
    template <typename T>
    int16_t array_value_remover(T *array, size_t *arraySize, T target)
    {
        size_t new_size = *arraySize;

        if ((array == NULL) || (arraySize == NULL))
        {
            return -1;
        }

        for (size_t i = 0; i < new_size; i++)
        {
            if (array[i] == target)
            {
                for (size_t j = i; j < new_size - 1; j++)
                {
                    array[j] = array[j + 1];
                    if (j == (new_size - 2))
                    {
                        array[j + 1] = 0;
                    }
                }
                i--;
                new_size--;
            }
        }

        *arraySize = new_size;

        return 0;
    }
    /// @brief Returns an array with indices indicating how we would access the list in increasing order.
    /// @tparam T Input array variables type
    /// @param inputArray Pointer to list to order
    /// @param refArray Pointer to a list of values which inputArray indices are based on
    /// @param output Pointer to array of index ordered by increasing value
    /// @param arraySize Number of element inside the inputArray
    /// @return !=0 if some errors happend
    template <typename T>
    int16_t array_map_incr(int16_t *inputArray, T *refArray, int16_t *output, size_t arraySize)
    {
        if ((inputArray == NULL) || (refArray == NULL) || (output == NULL))
        {
            return -1;
        }

        // Fill the output array with ordered index values
        for (size_t i = 0; i < arraySize; i++)
        {
            output[i] = i;
        }

        // Order output array
        for (size_t i = 0; i < arraySize; i++)
        {
            for (size_t j = i + 1; j < arraySize; j++)
            {
                if (refArray[inputArray[i]] > refArray[inputArray[j]])
                {
                    size_t temp = output[i];
                    output[i] = output[j];
                    output[j] = temp;
                }
            }
        }

        return 0;
    }

    /// @brief Returns a list of indices corresponding to the local peaks (change of sign of the derivative) and the indices corresponding to the absolute maximum and minimum.
    /// @tparam T Input array variables type
    /// @param inputArray Pointer to list of values to analyze.
    /// @param localPeaks Pointer to output array which contain the local peaks indices
    /// @param peaksCount Pointer to number of local peaks found
    /// @param absMaxPeak Pointer to absolute max peaks indice
    /// @param arraySize Number of element inside the inputArray
    /// @param outArraySize Number of element inside the outArraySize
    /// @return !=0 if some errors happend
    template <typename T>
    int16_t peaks_finder(T *inputArray, int16_t *localPeaks, size_t *peaksCount, size_t *absMaxPeak, size_t inArraySize, size_t outArraySize)
    {
        size_t _peakCount = 0, _maxPeak = 0;
        float_t _deriv_zero_cnt = 0;

        if ((inputArray == NULL) || (localPeaks == NULL) || (absMaxPeak == NULL) || (inArraySize == 0) || (outArraySize == 0))
        {
            return -1;
        }

        for (size_t i = 1; i < inArraySize - 1; i++)
        {
            //* Find all local peaks index
            // Calc the derivatives
            T deriv_act = inputArray[i] - inputArray[i - 1];
            T deriv_next = inputArray[i + 1] - inputArray[i];

            // Compare
            if ((deriv_act > 0 && deriv_next <= 0) && (_deriv_zero_cnt == 0))
            {
                if (deriv_next == 0)
                {
                    _deriv_zero_cnt++;
                }
                if (deriv_next < 0)
                {
                    localPeaks[_peakCount++] = i;
#ifdef APP_DEBUG_MODE
                    printf("1;%da\n", i);
#endif
                    if (_peakCount == 0)
                    {
                        _maxPeak = i;
                    }
                }
            }
            if ((deriv_act == 0 && deriv_next < 0) && (_deriv_zero_cnt != 0))
            {
                localPeaks[_peakCount++] = i - (size_t)((_deriv_zero_cnt / 2.0) + 0.5);

#ifdef APP_DEBUG_MODE
                printf("1;%db\n", i - (size_t)((_deriv_zero_cnt / 2.0) + 0.5));
#endif
                _deriv_zero_cnt = 0;
            }

            if (!((deriv_act > 0 && deriv_next <= 0) && (_deriv_zero_cnt == 0)) && !((deriv_act == 0 && deriv_next < 0) && (_deriv_zero_cnt != 0)))
            {
#ifdef APP_DEBUG_MODE
                printf("0;%d\n", i);
#endif
            }

            //* Find absolute peak index
            if ((inputArray[i] > inputArray[_maxPeak]))
            {
                _maxPeak = i;
            }
        }

        *peaksCount = _peakCount;
        *absMaxPeak = _maxPeak;

        return 0;
    }

    /// @brief Returns a list of indices corresponding to the local peaks (change of sign of the derivative) and the indices corresponding to the absolute maximum and minimum.
    /// @tparam T Input array variables type
    /// @param inputArray Pointer to list of values to analyze.
    /// @param localPeaks Pointer to output array which contain the local peaks indices
    /// @param lag How much your data will be smoothed and how adaptive the algorithm is to changes in the long-term average of the data.
    /// @param threshold The number of standard deviations from the moving mean above which the algorithm will classify a new datapoint as being a signal.
    /// @param influence If you put the influence at 0, you implicitly assume stationarity. If this is not the case, you should put the influence parameter somewhere between 0 and 1.
    /// @param peaksCount Pointer to number of local peaks found
    /// @param absMaxPeak Pointer to absolute max peaks indice
    /// @param arraySize Number of element inside the inputArray
    /// @param localPeaksArraySize Number of element inside the localPeaks array
    /// @return !=0 if some errors happend
    template <typename T>
    int16_t peaks_finder_z_scores(T *inputArray, int16_t *localPeaks, int16_t lag, T threshold, float_t influence, size_t inArraySize)
    {
        if ((inputArray == NULL) || (localPeaks == NULL) || (inArraySize == 0))
        {
            return -1;
        }

        T *filteredY = new T[inArraySize];
        T *avgFilter = new T[inArraySize];
        T *stdFilter = new T[inArraySize];

        avgFilter[lag - 1] = this->_mean(inputArray, lag);
        stdFilter[lag - 1] = this->_stddev(inputArray, lag);

        for (size_t i = lag; i < inArraySize; i++)
        {
            if (fabsf(inputArray[i] - avgFilter[i - 1]) > threshold * stdFilter[i - 1])
            {
                if (inputArray[i] > avgFilter[i - 1])
                {
                    localPeaks[i] = 1;
#ifdef APP_DEBUG_MODE
                    printf("1;%d\n", i);
#endif
                }
                else
                {
                    localPeaks[i] = -1;
#ifdef APP_DEBUG_MODE
                    printf("-1;%d\n", i);
#endif
                }
                filteredY[i] = influence * inputArray[i] + (1 - influence) * filteredY[i - 1];
            }
            else
            {
                localPeaks[i] = 0;
#ifdef APP_DEBUG_MODE
                printf("0;%d\n", i);
#endif
            }
            avgFilter[i] = this->_mean(filteredY + i - lag, lag);
            stdFilter[i] = this->_stddev(filteredY + i - lag, lag);
        }

        delete[] filteredY;
        delete[] avgFilter;
        delete[] stdFilter;

        return 0;
    }

    /// @brief
    /// @param ref_array
    /// @param ref_peaks
    /// @param sorted_peaks
    /// @param peak_num
    /// @param req_distance
    /// @return
    template <typename T>
    void peaks_filter_by_distance(T *ref_array, int16_t *ref_peaks, int16_t *sorted_peaks, size_t peak_num, T req_distance)
    {
        size_t current = 0;

        if (peak_num == 0)
        {
            return;
        }

        uint8_t *to_remove = new uint8_t[peak_num]();

#ifdef APP_DEBUG_MODE
        ESP_LOGW(TAG, "Peaks whose distance is less than %llu:\nPeaks to scan: %d", req_distance, peak_num);
#endif

        for (size_t i = 0; i < peak_num; i++)
        {
            current = sorted_peaks[i];

            if (to_remove[current] == 1)
            {
                continue; // peak has already been removed, move on.
            }
#ifdef APP_DEBUG_MODE
            ESP_LOGI(TAG, "Check left side");
#endif
            // check on left side of peak
            int16_t neighbor = current - 1;
            while (neighbor >= 0 && (ref_array[ref_peaks[current]] - ref_array[ref_peaks[neighbor]]) < req_distance)
            {
                to_remove[neighbor] = 1;
#ifdef APP_DEBUG_MODE
                printf("%d,%d,%d,%d\n", current, neighbor, ref_peaks[current], ref_peaks[neighbor]);
#endif
                --neighbor;
            }

#ifdef APP_DEBUG_MODE
            ESP_LOGI(TAG, "Check right side");
#endif

            // check on right side of peak
            neighbor = current + 1;
            while (neighbor < peak_num && (ref_array[ref_peaks[neighbor]] - ref_array[ref_peaks[current]]) < req_distance)
            {
                to_remove[neighbor] = 1;
#ifdef APP_DEBUG_MODE
                printf("%d,%d,%d,%d\n", current, neighbor, ref_peaks[current], ref_peaks[neighbor]);
#endif
                ++neighbor;
            }
        }

        // Remove the incorrect peak indexs
        for (size_t i = 0; i < peak_num; i++)
        {
            if (to_remove[i] == 1)
            {
                ref_peaks[i] = -1;
            }
        }

        delete[] to_remove;
    }

    /// @brief
    /// @tparam T Input array variables type
    /// @param array
    /// @param arraySize
    /// @param max_value
    /// @param min_value
    template <typename T>
    int16_t array_max_min_finder(T *array, size_t arraySize, T *max_value, T *min_value)
    {
        if ((array == 0) || (max_value == 0) || (min_value == 0))
        {
            return -1;
        }

        for (size_t i = 0; i < arraySize; i++)
        {
            if (array[i] > *max_value)
            {
                *max_value = array[i];
            }
            if (array[i] < *min_value)
            {
                *min_value = array[i];
            }
        }

        return 0;
    }

    /// @brief
    /// @tparam T Input array variables type
    /// @param array
    /// @param arraySize
    /// @param newSize
    /// @param startIndex
    template <typename T>
    int16_t array_cutter(T *array, size_t arraySize, size_t newSize, size_t startIndex)
    {
        if ((array == 0) || ((startIndex + newSize) > arraySize))
        {
            return -1;
        }

        size_t j = 0;

        for (size_t i = startIndex; i < arraySize + startIndex; i++)
        {
            if (i < startIndex + newSize)
            {
                array[j] = array[i];
            }
            else
            {
                array[j] = 0;
            }

            j++;
        }

        return 0;
    }

private:
    const char *TAG = "SIGNAL-ANALISYS";

    float_t _mean(float_t data[], int16_t len);
    float_t _stddev(float_t data[], int16_t len);
};
