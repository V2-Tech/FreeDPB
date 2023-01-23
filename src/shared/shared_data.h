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

    // Metodi per accedere/modificare i dati
    void setRotCount(uint64_t v);
    uint64_t getRotCount();
    void setMessage(const char *msg);
    const char *getMessage();
    void setDPBAccData(int16_t v_x, int16_t v_y, uint8_t rot_flag, uint64_t time, uint32_t index);
    void setAccX(int16_t v, uint32_t index);
    void setAccY(int16_t v, uint32_t index);
    void getDPBAccData(dpb_acc_data *data, uint32_t index);
    void setDPBAccDataFiltered(int16_t v_x, int16_t v_y, uint8_t rot_flag, uint64_t time, uint32_t index);
    void setAccXFiltered(int16_t v, uint32_t index);
    void setAccYFiltered(int16_t v, uint32_t index);
    void getDPBAccDataFiltered(dpb_acc_data *data, uint32_t index);
    void setRPM(uint16_t v);
    uint16_t getRPM();
    void setAppStatus(app_steps v);
    app_steps getAppStatus();
    void setFFTX(float_t v, size_t index);
    float_t getFFTX(size_t index);
    void setFFTY(float_t v, size_t index);
    float_t getFFTY(size_t index);
    void setSampleRate(uint16_t v);
    uint16_t getSampleRate();
    void setUnbalanceAngle(uint16_t v);
    uint16_t getUnbalanceAngle();
    void setPeakIndex(size_t v, uint32_t index);
    size_t getPeakIndex(uint32_t index);
    int16_t *getPeakIndexPointer();
    void setFFTXPeak(size_t v);
    size_t getFFTXPeak(void);
    void setFFTYPeak(size_t v);
    size_t getFFTYPeak(void);
    void setAccXMaxPeak(size_t v);
    size_t getAccXMaxPeak(void);
    void setAccYMaxPeak(size_t v);
    size_t getAccYMaxPeak(void);

private:
    DPBShared();
    ~DPBShared() { delete _data; }

    struct Data
    {
        // Dati condivisi
        uint64_t _rotCount;
        uint16_t _rpm;
        app_steps _step;
        char message[20];
        dpb_acc_data _dpb_acc[ACC_DATA_BUFFER_SIZE] = {0};
        dpb_acc_data _dpb_acc_filtered[ACC_DATA_BUFFER_SIZE] = {0};
        float_t _fft_x[FFT_DATA_BUFFER_SIZE] = {0};
        float_t _fft_y[FFT_DATA_BUFFER_SIZE] = {0};
        uint16_t _sampleRate;
        uint16_t _unbalanceAngle;
        int16_t _peak_index[ACC_DATA_BUFFER_SIZE] = {0};
        size_t _acc_x_max_peak;
        size_t _acc_y_max_peak;
        size_t _fft_x_peak;
        size_t _fft_y_peak;
    };
    Data *_data;

    // Semaforo per proteggere l'accesso alla memoria condivisa
    SemaphoreHandle_t _xSemaphore;

    // Metodi per acquisire/rilasciare il semaforo
    void _lock();
    void _unlock();
};

#endif