#ifndef INC_UTILS_H
#define INC_UTILS_H

#include <array>
#include <stdexcept>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//**********************************/
//*      FUNCTION DEFINITIONS       /
//**********************************/
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

    if (array == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < *arraySize; i++)
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
int16_t array_map_incr(size_t *inputArray, T *refArray, size_t *output, size_t arraySize)
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
int16_t peaks_finder(T *inputArray, size_t *localPeaks, size_t *peaksCount, size_t *absMaxPeak, size_t inArraySize, size_t outArraySize)
{
    size_t _peakCount = 0, _maxPeak = 0;
    T last, actual, next;

    if ((inputArray == NULL) || (localPeaks == NULL) || (absMaxPeak == NULL) || (inArraySize == 0) || (outArraySize == 0))
    {
        return -1;
    }

    for (size_t i = 1; i < inArraySize - 1; i++)
    {
        // Find all local peaks index
        if (inputArray[i] > inputArray[i - 1] && inputArray[i] > inputArray[i + 1])
        {
            localPeaks[_peakCount] = i;

            if (_peakCount == 0)
            {
                _maxPeak = i;
            }

            _peakCount++;
        }

        // Find absolute peak index
        if ((inputArray[i] > inputArray[_maxPeak]))
        {
            _maxPeak = i;
        }
    }

    *peaksCount = _peakCount;
    *absMaxPeak = _maxPeak;

    return 0;
}


#endif