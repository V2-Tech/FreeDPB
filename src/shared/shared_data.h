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
    void setAccX(int16_t v, uint8_t rot_flag, uint64_t time, uint32_t index);
    void setAccY(int16_t v, uint8_t rot_flag, uint64_t time, uint32_t index);
    void getAccData(dpb_acc_data *data, uint32_t index);
    void setRPM(uint16_t v);
    uint16_t getRPM();
    void setAppStatus(app_steps v);
    app_steps getAppStatus();

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
        dpb_acc_data _dpb_acc[ACC_DATA_BUFFER_SIZE];
    };
    Data *_data;

    // Semaforo per proteggere l'accesso alla memoria condivisa
    SemaphoreHandle_t _xSemaphore;

    // Metodi per acquisire/rilasciare il semaforo
    void _lock();
    void _unlock();
};

#endif