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

void DPBShared::setAccX(int16_t v, uint8_t rot_flag, uint64_t time, uint32_t index)
{
    _lock();
    _data->_dpb_acc[index].accel_data.acc_x = v;
    _data->_dpb_acc[index].xRot_done = rot_flag;
    _data->_dpb_acc[index].time_counts = time;
    _unlock();
}

void DPBShared::setAccY(int16_t v, uint8_t rot_flag, uint64_t time, uint32_t index)
{
    _lock();
    _data->_dpb_acc[index].accel_data.acc_y = v;
    _data->_dpb_acc[index].xRot_done = rot_flag;
    _data->_dpb_acc[index].time_counts = time;
    _unlock();
}

void DPBShared::getAccData(dpb_acc_data *v, uint32_t index)
{
    if (v == NULL)
    {
        return;
    }

    _lock();
    *v = _data->_dpb_acc[index];
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

void DPBShared::_lock()
{
    xSemaphoreTake(_xSemaphore, portMAX_DELAY);
}

void DPBShared::_unlock()
{
    xSemaphoreGive(_xSemaphore);
}
