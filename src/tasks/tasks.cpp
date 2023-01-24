#include "tasks.h"

//**************************************/
//*         VARIABLES DECLARATIONS      /
//**************************************/
static StaticQueue_t xStaticQueueComp2SysCommand;
uint8_t ucQueueComp2SysCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueComp2SysCommandsHandle;

static StaticQueue_t xStaticQueueSys2CompCommand;
uint8_t ucQueueSys2CompCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueSys2CompCommandsHandle;

#ifdef USE_BMX055
static BMX055 accel;
#elif USE_ADXL345
static ADXL345 accel;
#endif

static DPB sys(GPIO_ESC_OUT, DSHOT300, GPIO_OPT_SENSOR, &accel);

DPBShared& sharedVars = DPBShared::getInstance();

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?             GUI TASK            /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
void guiTask(void *pvParameter)
{
    SemaphoreHandle_t xGuiSemaphore = xSemaphoreCreateMutex();

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Wait until queues have been created */
    while ((xQueueComp2SysCommandsHandle == NULL) || (xQueueSys2CompCommandsHandle == NULL))
        ;

    /* Create DPB first page interface */
    ESP_ERROR_CHECK(gui_init(xQueueComp2SysCommandsHandle, xQueueSys2CompCommandsHandle));

    while (1)
    {
        gui_update();

        /* Try to take the semaphore, call lvgl related function on success */
        if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
        {
            lv_timer_handler();
            xSemaphoreGive(xGuiSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

void IRAM_ATTR lv_tick_task(void *arg)
{
    (void)arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         ACCELEROMETER TASK          /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
void accelTask(void *pvParameter)
{  
    while (1)
    {
        sys.loop_accel();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         ROTATION TASK               /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
void senseTask(void *pvParameter)
{
    while (1)
    {
        sys.loop_rpm();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         APPLICATION TASK            /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
void application(void *pvParameter)
{
    /* Create queue to send command from system app to app components */
    xQueueSys2CompCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                      ITEM_COMMANDS_SIZE,
                                                      ucQueueSys2CompCommandStorageArea,
                                                      &xStaticQueueSys2CompCommand);
    configASSERT(xQueueSys2CompCommandsHandle);
    /* Create queue to send command from components to system */
    xQueueComp2SysCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                      ITEM_COMMANDS_SIZE,
                                                      ucQueueComp2SysCommandStorageArea,
                                                      &xStaticQueueComp2SysCommand);
    configASSERT(xQueueComp2SysCommandsHandle);

    /* Wait until queues have been created */
    while ((xQueueComp2SysCommandsHandle == NULL) || (xQueueSys2CompCommandsHandle == NULL))
        ;

    ESP_ERROR_CHECK(sys.init(xQueueComp2SysCommandsHandle, xQueueSys2CompCommandsHandle, senseTaskHandle, &sharedVars));

    while (1)
    {
        sys.loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}