#ifndef INC_SHARED_H
#define INC_SHARED_H

#include "common_def.h"

class DPBShared
{
public:
    static DPBShared &getInstance()
    {
        static DPBShared instance;
        return instance;
    }

    //* Data access methods
    //! "*_us" = unsafe, if you need a thread-safe access, please call related lock* method before operations and unlock* at the end of operations.
    void setRotCount(uint64_t v);
    uint64_t getRotCount();

    void setDPBData(int16_t v_x, int16_t v_y, uint32_t index);
    void getDPBData(int16_t *v_x, int16_t *v_y, uint32_t index);
    void setDPBRotDone(uint8_t flag, uint32_t index);
    void getDPBRotDone(uint8_t *flag, uint32_t index);
    void setDPBTime(uint64_t count, uint32_t index);
    void getDPBTime(uint64_t *count, uint32_t index);
    void lockDPBDataAcc(void);
    void unlockDPBDataAcc(void);
    int16_t *getDPBDataAccXBuffer_us(void);
    int16_t *getDPBDataAccYBuffer_us(void);
    uint8_t *getDPBRotDoneBuffer_us(void);
    uint64_t *getDPBTimeBuffer_us(void);

    void setDPBDataFlt(int16_t v_x, int16_t v_y, uint32_t index);
    void getDPBDataFlt(int16_t *v_x, int16_t *v_y, uint32_t index);
    void lockDPBDataFltAcc(void);
    void unlockDPBDataFltAcc(void);
    int16_t *getDPBDataFltAccXBuffer_us(void);
    int16_t *getDPBDataFltAccYBuffer_us(void);

    void setRPM(uint16_t v);
    uint16_t getRPM();

    void setAppStatus(app_steps_e v);
    app_steps_e getAppStatus();

    void setFFTX(float_t v, size_t index);
    float_t getFFTX(size_t index);
    void setFFTY(float_t v, size_t index);
    float_t getFFTY(size_t index);
    void lockFFT(void);
    void unlockFFT(void);
    float_t *getFFTXBuffer_us(void);
    float_t *getFFTYBuffer_us(void);

    void setBandWidth(uint16_t v);
    uint16_t getBandWidth();
    void setRange(uint16_t v);
    uint16_t getRange();

    void setUnbalanceXAngle(float_t v);
    float_t getUnbalanceXAngle();
    void setUnbalanceYAngle(float_t v);
    float_t getUnbalanceYAngle();
    void setUnbalanceFreq(float_t v);
    float_t getUnbalanceFreq();
    void setUnbalanceMag(float_t v);
    float_t getUnbalanceMag();

    void setXPeaksIndex(size_t v, uint32_t index);
    size_t getXPeakIndex(uint32_t index);
    size_t *getXPeaksIndexPointer_us();
    void setYPeaksIndex(size_t v, uint32_t index);
    size_t getYPeakIndex(uint32_t index);
    size_t *getYPeaksIndexPointer_us();
    void lockPeaksIndex(void);
    void unlockPeaksIndex(void);

    void setXPeakCount(size_t v);
    size_t getXPeakCount(void);
    void setYPeakCount(size_t v);
    size_t getYPeakCount(void);

    void setFFTXMaxIndex(size_t v);
    size_t getFFTXMaxIndex(void);
    void setFFTYMaxIndex(size_t v);
    size_t getFFTYMaxIndex(void);

    void setAccXMaxIndex(size_t v);
    size_t getAccXMaxIndex(void);
    void setAccYMaxIndex(size_t v);
    size_t getAccYMaxIndex(void);

    void setAngleOffset(int16_t v);
    int16_t getAngleOffset();

    void setSearchType(app_search_type_e v);
    app_search_type_e getSearchType();

private:
    DPBShared();
    ~DPBShared() { delete _data; }

    //* Shared variables list
    struct Data
    {
        // Internal
        uint64_t _rotCount;
        uint16_t _rpm;
        app_steps_e _step;
        dpb_acc_data_t<ACC_DATA_BUFFER_SIZE> _dpb_data = {0};
        dpb_acc_data_t<ACC_DATA_BUFFER_SIZE> _dpb_data_filtered = {0};
        uint8_t _dpb_rotDone[ACC_DATA_BUFFER_SIZE] = {0};
        uint64_t _dpb_time[ACC_DATA_BUFFER_SIZE] = {0};
        float_t _fft_x[FFT_DATA_BUFFER_SIZE] = {0};
        float_t _fft_y[FFT_DATA_BUFFER_SIZE] = {0};
        float_t _unbalanceXAngle;
        float_t _unbalanceYAngle;
        float_t _unbalanceFreq;
        float_t _unbalanceMag;
        size_t _x_peak_index[ACC_DATA_BUFFER_SIZE] = {0};
        size_t _y_peak_index[ACC_DATA_BUFFER_SIZE] = {0};
        size_t _x_peak_count;
        size_t _y_peak_count;
        size_t _acc_x_max_index;
        size_t _acc_y_max_index;
        size_t _fft_x_max_index;
        size_t _fft_y_max_index;

        // User
        uint16_t _range;
        uint16_t _bandWidth;
        int16_t _angleOffset;
        app_search_type_e _searchType;
    };
    Data *_data;

    //* Semaphores to protect data access
    SemaphoreHandle_t _xSemComm;
    SemaphoreHandle_t _xSemDpbData;
    SemaphoreHandle_t _xSemDpbDataFlt;
    SemaphoreHandle_t _xSemPeaksIndex;
    SemaphoreHandle_t _xSemFFT;

    //* Semaphores fast access methods
    // Common semaphores used for low prority data
    void _lockComm();
    void _unlockComm();

    // Private semaphores for priority data
    void _lockDpbData();
    void _unlockDpbData();
    void _lockDpbDataFlt();
    void _unlockDpbDataFlt();
    void _lockPeaksIndex();
    void _unlockPeaksIndex();
    void _lockFFT();
    void _unlockFFT();
};

#endif