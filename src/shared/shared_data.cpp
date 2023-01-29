#include "shared_data.h"

DPBShared::DPBShared() : _data(new Data)
{
    _data->_rotCount = 0;
    _data->_rpm = 0;
    _data->_step = IDLE;
    _data->_bandWidth = 0;
    _data->_range = 0;
    _data->_unbalanceXAngle = 0;
    _data->_unbalanceYAngle = 0;
    _data->_unbalanceFreq = 0;
    _data->_x_peak_count = 0;
    _data->_y_peak_count = 0;
    _data->_acc_x_max_index = 0;
    _data->_acc_y_max_index = 0;
    _data->_fft_x_max_index = 0;
    _data->_fft_y_max_index = 0;

    _xSemComm = xSemaphoreCreateMutex();
    _xSemDpbData = xSemaphoreCreateMutex();
    _xSemDpbDataFlt = xSemaphoreCreateMutex();
    _xSemPeaksIndex = xSemaphoreCreateMutex();
    _xSemFFT = xSemaphoreCreateMutex();
}

void DPBShared::setRotCount(uint64_t v)
{
    _lockComm();
    _data->_rotCount = v;
    _unlockComm();
}

uint64_t DPBShared::getRotCount()
{
    _lockComm();
    uint64_t v = _data->_rotCount;
    _unlockComm();
    return v;
}

void DPBShared::setDPBData(int16_t v_x, int16_t v_y, uint32_t index)
{
    _lockDpbData();
    _data->_dpb_data.acc_x[index] = v_x;
    _data->_dpb_data.acc_y[index] = v_y;
    _unlockDpbData();
}

void DPBShared::getDPBData(int16_t *v_x, int16_t *v_y, uint32_t index)
{
    if ((v_x == NULL) || (v_y == NULL))
    {
        return;
    }

    _lockDpbData();
    *v_x = _data->_dpb_data.acc_x[index];
    *v_y = _data->_dpb_data.acc_y[index];
    _unlockDpbData();
}

void DPBShared::setDPBRotDone(uint8_t flag, uint32_t index)
{
    _lockDpbData();
    _data->_dpb_rotDone[index] = flag;
    _unlockDpbData();
}

void DPBShared::getDPBRotDone(uint8_t *flag, uint32_t index)
{
    if (flag == NULL)
    {
        return;
    }

    _lockDpbData();
    *flag = _data->_dpb_rotDone[index];
    _unlockDpbData();
}

void DPBShared::setDPBTime(uint64_t count, uint32_t index)
{
    _lockDpbData();
    _data->_dpb_time[index] = count;
    _unlockDpbData();
}

void DPBShared::getDPBTime(uint64_t *count, uint32_t index)
{
    if (count == NULL)
    {
        return;
    }

    _lockDpbData();
    *count = _data->_dpb_time[index];
    _unlockDpbData();
}

void DPBShared::lockDPBDataAcc(void)
{
    _lockDpbData();
}

void DPBShared::unlockDPBDataAcc(void)
{
    _unlockDpbData();
}

int16_t *DPBShared::getDPBDataAccXBuffer_us(void)
{
    return _data->_dpb_data.acc_x;
}

int16_t *DPBShared::getDPBDataAccYBuffer_us(void)
{
    return _data->_dpb_data.acc_y;
}

uint8_t *DPBShared::getDPBRotDoneBuffer_us(void)
{
    return _data->_dpb_rotDone;
}

uint64_t *DPBShared::getDPBTimeBuffer_us(void)
{
    return _data->_dpb_time;
}

void DPBShared::setDPBDataFlt(int16_t v_x, int16_t v_y, uint32_t index)
{
    _lockDpbDataFlt();
    _data->_dpb_data_filtered.acc_x[index] = v_x;
    _data->_dpb_data_filtered.acc_y[index] = v_y;
    _unlockDpbDataFlt();
}

void DPBShared::getDPBDataFlt(int16_t *v_x, int16_t *v_y, uint32_t index)
{
    if ((v_x == NULL) || (v_y == NULL))
    {
        return;
    }

    _lockDpbDataFlt();
    *v_x = _data->_dpb_data_filtered.acc_x[index];
    *v_y = _data->_dpb_data_filtered.acc_y[index];
    _unlockDpbDataFlt();
}

void DPBShared::lockDPBDataFltAcc(void)
{
    _lockDpbDataFlt();
}

void DPBShared::unlockDPBDataFltAcc(void)
{
    _unlockDpbDataFlt();
}

int16_t *DPBShared::getDPBDataFltAccXBuffer_us(void)
{
    return _data->_dpb_data_filtered.acc_x;
}

int16_t *DPBShared::getDPBDataFltAccYBuffer_us(void)
{
    return _data->_dpb_data_filtered.acc_y;
}

void DPBShared::setRPM(uint16_t v)
{
    _lockComm();
    _data->_rpm = v;
    _unlockComm();
}

uint16_t DPBShared::getRPM()
{
    _lockComm();
    uint16_t v = _data->_rpm;
    _unlockComm();
    return v;
}

void DPBShared::setAppStatus(app_steps_e v)
{
    _lockComm();
    _data->_step = v;
    _unlockComm();
}

app_steps_e DPBShared::getAppStatus()
{
    _lockComm();
    app_steps_e v = _data->_step;
    _unlockComm();
    return v;
}

void DPBShared::setFFTX(float_t v, size_t index)
{
    _lockFFT();
    _data->_fft_x[index] = v;
    _unlockFFT();
}

float_t DPBShared::getFFTX(size_t index)
{
    _lockFFT();
    float_t v = _data->_fft_x[index];
    _unlockFFT();
    return v;
}

void DPBShared::setFFTY(float_t v, size_t index)
{
    _lockFFT();
    _data->_fft_y[index] = v;
    _unlockFFT();
}

float_t DPBShared::getFFTY(size_t index)
{
    _lockFFT();
    float_t v = _data->_fft_y[index];
    _unlockFFT();
    return v;
}

void DPBShared::lockFFT(void)
{
    _lockFFT();
}

void DPBShared::unlockFFT(void)
{
    _unlockFFT();
}

float_t *DPBShared::getFFTXBuffer_us(void)
{
    return _data->_fft_x;
}

float_t *DPBShared::getFFTYBuffer_us(void)
{
    return _data->_fft_y;
}

void DPBShared::setBandWidth(uint16_t v)
{
    _lockComm();
    _data->_bandWidth = v;
    _unlockComm();
}

uint16_t DPBShared::getBandWidth()
{
    _lockComm();
    uint16_t v = _data->_bandWidth;
    _unlockComm();
    return v;
}

void DPBShared::setRange(uint16_t v)
{
    _lockComm();
    _data->_range = v;
    _unlockComm();
}

uint16_t DPBShared::getRange()
{
    _lockComm();
    uint16_t v = _data->_range;
    _unlockComm();
    return v;
}

void DPBShared::setUnbalanceXAngle(float_t v)
{
    _lockComm();
    _data->_unbalanceXAngle = v;
    _unlockComm();
}

float_t DPBShared::getUnbalanceXAngle()
{
    _lockComm();
    float_t v = _data->_unbalanceXAngle;
    _unlockComm();
    return v;
}

void DPBShared::setUnbalanceYAngle(float_t v)
{
    _lockComm();
    _data->_unbalanceYAngle = v;
    _unlockComm();
}

float_t DPBShared::getUnbalanceYAngle()
{
    _lockComm();
    float_t v = _data->_unbalanceYAngle;
    _unlockComm();
    return v;
}

void DPBShared::setUnbalanceFreq(float_t v)
{
    _lockComm();
    _data->_unbalanceFreq = v;
    _unlockComm();
}

float_t DPBShared::getUnbalanceFreq()
{
    _lockComm();
    float_t v = _data->_unbalanceFreq;
    _unlockComm();
    return v;
}

void DPBShared::setXPeaksIndex(size_t v, uint32_t index)
{
    _lockComm();
    _data->_x_peak_index[index] = v;
    _unlockComm();
}

size_t DPBShared::getXPeakIndex(uint32_t index)
{
    _lockComm();
    size_t v = _data->_x_peak_index[index];
    _unlockComm();
    return v;
}

size_t *DPBShared::getXPeaksIndexPointer_us()
{
    return _data->_x_peak_index;
}

void DPBShared::setYPeaksIndex(size_t v, uint32_t index)
{
    _lockComm();
    _data->_x_peak_index[index] = v;
    _unlockComm();
}

size_t DPBShared::getYPeakIndex(uint32_t index)
{
    _lockComm();
    size_t v = _data->_y_peak_index[index];
    _unlockComm();
    return v;
}

size_t *DPBShared::getYPeaksIndexPointer_us()
{
    return _data->_y_peak_index;
}

void DPBShared::lockPeaksIndex(void)
{
    _lockPeaksIndex();
}

void DPBShared::unlockPeaksIndex(void)
{
    _unlockPeaksIndex();
}

void DPBShared::setXPeakCount(size_t v)
{
    _lockComm();
    _data->_x_peak_count = v;
    _unlockComm();
}

size_t DPBShared::getXPeakCount(void)
{
    _lockComm();
    size_t v = _data->_x_peak_count;
    _unlockComm();
    return v;
}

void DPBShared::setYPeakCount(size_t v)
{
    _lockComm();
    _data->_y_peak_count = v;
    _unlockComm();
}

size_t DPBShared::getYPeakCount(void)
{
    _lockComm();
    size_t v = _data->_y_peak_count;
    _unlockComm();
    return v;
}

void DPBShared::setFFTXMaxIndex(size_t v)
{
    _lockComm();
    _data->_fft_x_max_index = v;
    _unlockComm();
}

size_t DPBShared::getFFTXMaxIndex(void)
{
    _lockComm();
    size_t v = _data->_fft_x_max_index;
    _unlockComm();
    return v;
}

void DPBShared::setFFTYMaxIndex(size_t v)
{
    _lockComm();
    _data->_fft_y_max_index = v;
    _unlockComm();
}

size_t DPBShared::getFFTYMaxIndex(void)
{
    _lockComm();
    size_t v = _data->_fft_y_max_index;
    _unlockComm();
    return v;
}

void DPBShared::setAccXMaxIndex(size_t v)
{
    _lockComm();
    _data->_acc_x_max_index = v;
    _unlockComm();
}

size_t DPBShared::getAccXMaxIndex(void)
{
    _lockComm();
    size_t v = _data->_acc_x_max_index;
    _unlockComm();
    return v;
}

void DPBShared::setAccYMaxIndex(size_t v)
{
    _lockComm();
    _data->_acc_y_max_index = v;
    _unlockComm();
}

size_t DPBShared::getAccYMaxIndex(void)
{
    _lockComm();
    size_t v = _data->_acc_y_max_index;
    _unlockComm();
    return v;
}

void DPBShared::_lockComm()
{
    xSemaphoreTake(_xSemComm, portMAX_DELAY);
}

void DPBShared::_unlockComm()
{
    xSemaphoreGive(_xSemComm);
}

void DPBShared::_lockDpbData()
{
    xSemaphoreTake(_xSemDpbData, portMAX_DELAY);
}

void DPBShared::_unlockDpbData()
{
    xSemaphoreGive(_xSemDpbData);
}

void DPBShared::_lockDpbDataFlt()
{
    xSemaphoreTake(_xSemDpbDataFlt, portMAX_DELAY);
}

void DPBShared::_unlockDpbDataFlt()
{
    xSemaphoreGive(_xSemDpbDataFlt);
}

void DPBShared::_lockPeaksIndex()
{
    xSemaphoreTake(_xSemPeaksIndex, portMAX_DELAY);
}

void DPBShared::_unlockPeaksIndex()
{
    xSemaphoreGive(_xSemPeaksIndex);
}

void DPBShared::_lockFFT()
{
    xSemaphoreTake(_xSemFFT, portMAX_DELAY);
}

void DPBShared::_unlockFFT()
{
    xSemaphoreGive(_xSemFFT);
}