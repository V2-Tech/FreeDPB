#include "include/SignalProcessing.hpp"

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         FUNCTIONS DEFINITION        /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
float_t SignalProcessing::_mean(float_t data[], int16_t len)
{
    float_t sum = 0.0, mean = 0.0;

    int16_t i;
    for (i = 0; i < len; ++i)
    {
        sum += data[i];
    }

    mean = sum / len;
    return mean;
}

float_t SignalProcessing::_stddev(float_t data[], int16_t len)
{
    float_t the_mean = _mean(data, len);
    float_t standardDeviation = 0.0;

    int16_t i;
    for (i = 0; i < len; ++i)
    {
        standardDeviation += pow(data[i] - the_mean, 2);
    }

    return sqrt(standardDeviation / len);
}