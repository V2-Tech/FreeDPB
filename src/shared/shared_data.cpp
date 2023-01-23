#include "shared_data.h"

DPBShared::DPBShared() : _data(new Data)
{
    _data->_rotCount = 0;
    strcpy(_data->message, "");

    _xSemaphore = xSemaphoreCreateMutex();
}

void DPBShared::setRotCount(uint64_t v)
{
    _lock();
    _data->_rotCount = v;
    _unlock();
}

uint64_t DPBShared::getRotCount()
{
    _lock();
    uint64_t v = _data->_rotCount;
    _unlock();
    return v;
}

void DPBShared::setMessage(const char *msg)
{
    _lock();
    strcpy(_data->message, msg);
    _unlock();
}

const char *DPBShared::getMessage()
{
    _lock();
    const char *msg = _data->message;
    _unlock();
    return msg;
}

void DPBShared::setDPBAccData(int16_t v_x, int16_t v_y, uint8_t rot_flag, uint64_t time, uint32_t index)
{
    _lock();
    _data->_dpb_acc[index].accel_data.acc_x = v_x;
    _data->_dpb_acc[index].accel_data.acc_y = v_y;
    _data->_dpb_acc[index].xRot_done = rot_flag;
    _data->_dpb_acc[index].time_counts = time;
    _unlock();
}

void DPBShared::setAccX(int16_t v, uint32_t index)
{
    _lock();
    _data->_dpb_acc[index].accel_data.acc_x = v;
    _unlock();
}

void DPBShared::setAccY(int16_t v, uint32_t index)
{
    _lock();
    _data->_dpb_acc[index].accel_data.acc_y = v;
    _unlock();
}

void DPBShared::getDPBAccData(dpb_acc_data *v, uint32_t index)
{
    if (v == NULL)
    {
        return;
    }

    _lock();
    *v = _data->_dpb_acc[index];
    _unlock();
}

void DPBShared::setDPBAccDataFiltered(int16_t v_x, int16_t v_y,
                                      uint8_t rot_flag, uint64_t time,
                                      uint32_t index)
{
    _lock();
    _data->_dpb_acc_filtered[index].accel_data.acc_x = v_x;
    _data->_dpb_acc_filtered[index].accel_data.acc_y = v_y;
    _data->_dpb_acc_filtered[index].xRot_done = rot_flag;
    _data->_dpb_acc_filtered[index].time_counts = time;
    _unlock();
}

void DPBShared::setAccXFiltered(int16_t v, uint32_t index)
{
    _lock();
    _data->_dpb_acc_filtered[index].accel_data.acc_x = v;
    _unlock();
}

void DPBShared::setAccYFiltered(int16_t v, uint32_t index)
{
    _lock();
    _data->_dpb_acc_filtered[index].accel_data.acc_y = v;
    _unlock();
}

void DPBShared::getDPBAccDataFiltered(dpb_acc_data *v, uint32_t index)
{
    if (v == NULL)
    {
        return;
    }

    _lock();
    *v = _data->_dpb_acc_filtered[index];
    _unlock();
}

void DPBShared::setRPM(uint16_t v)
{
    _lock();
    _data->_rpm = v;
    _unlock();
}

uint16_t DPBShared::getRPM()
{
    _lock();
    uint16_t v = _data->_rpm;
    _unlock();
    return v;
}

void DPBShared::setAppStatus(app_steps v)
{
    _lock();
    _data->_step = v;
    _unlock();
}

app_steps DPBShared::getAppStatus()
{
    _lock();
    app_steps v = _data->_step;
    _unlock();
    return v;
}

void DPBShared::setFFTX(float_t v, size_t index)
{
    _lock();
    _data->_fft_x[index] = v;
    _unlock();
}

float_t DPBShared::getFFTX(size_t index)
{
    _lock();
    float_t v = _data->_fft_x[index];
    _unlock();
    return v;
}

void DPBShared::setFFTY(float_t v, size_t index)
{
    _lock();
    _data->_fft_y[index] = v;
    _unlock();
}

float_t DPBShared::getFFTY(size_t index)
{
    _lock();
    float_t v = _data->_fft_y[index];
    _unlock();
    return v;
}

void DPBShared::setSampleRate(uint16_t v)
{
    _lock();
    _data->_sampleRate = v;
    _unlock();
}

uint16_t DPBShared::getSampleRate()
{
    _lock();
    uint16_t v = _data->_sampleRate;
    _unlock();
    return v;
}

void DPBShared::setUnbalanceAngle(uint16_t v)
{
    _lock();
    _data->_unbalanceAngle = v;
    _unlock();
}

uint16_t DPBShared::getUnbalanceAngle()
{
    _lock();
    uint16_t v = _data->_unbalanceAngle;
    _unlock();
    return v;
}

void DPBShared::setPeakIndex(size_t v, uint32_t index)
{
    _lock();
    _data->_peak_index[index] = v;
    _unlock();
}

size_t DPBShared::getPeakIndex(uint32_t index)
{
    _lock();
    size_t v = _data->_peak_index[index];
    _unlock();
    return v;
}

int16_t *DPBShared::getPeakIndexPointer()
{
    return _data->_peak_index;
}

void DPBShared::setFFTXPeak(size_t v)
{
    _lock();
    _data->_fft_x_peak = v;
    _unlock();
}

size_t DPBShared::getFFTXPeak(void)
{
    _lock();
    size_t v = _data->_fft_x_peak;
    _unlock();
    return v;
}

void DPBShared::setFFTYPeak(size_t v)
{
    _lock();
    _data->_fft_y_peak = v;
    _unlock();
}

size_t DPBShared::getFFTYPeak(void)
{
    _lock();
    size_t v = _data->_fft_y_peak;
    _unlock();
    return v;
}

void DPBShared::setAccXMaxPeak(size_t v)
{
    _lock();
    _data->_acc_x_max_peak = v;
    _unlock();
}

size_t DPBShared::getAccXMaxPeak(void)
{
    _lock();
    size_t v = _data->_acc_x_max_peak;
    _unlock();
    return v;
}

void DPBShared::setAccYMaxPeak(size_t v)
{
    _lock();
    _data->_acc_y_max_peak = v;
    _unlock();
}

size_t DPBShared::getAccYMaxPeak(void)
{
    _lock();
    size_t v = _data->_acc_y_max_peak;
    _unlock();
    return v;
}

void DPBShared::_lock()
{
    xSemaphoreTake(_xSemaphore, portMAX_DELAY);
}

void DPBShared::_unlock()
{
    xSemaphoreGive(_xSemaphore);
}
