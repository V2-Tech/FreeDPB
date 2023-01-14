#include "tasks_def.h"
#include "tasks.h"

/******************************************
 *              GUI TASK
 ******************************************/
void guiTask(void *pvParameter)
{
    /* TFT init */
    tft.begin();
    /* Landscape orientation, flipped */
    tft.setRotation(1);
    tft.setBrightness(255);
    /* Calibrate touchscreen */
    uint16_t calData[] = {3754, 454, 3694, 3862, 352, 453, 314, 3883};
    tft.setTouchCalibrate(calData);

    /* Initialize the display */
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    (void)pvParameter;

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
    ESP_ERROR_CHECK(gui_init(xQueueComp2SysCommandsHandle, xQueueSys2CompCommandsHandle, FFTOuput));

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

/****************************************
 *          ACCELEROMETER TASK          *
 ****************************************/
void accelTask(void *pvParameter)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

/********************************
 *        POSITION TASK         *
 ********************************/
void senseTask(void *pvParameter)
{
    while (1)
    {
        sys.loop_rpm();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

/****************************
 *          APP TASK        *
 ****************************/
void application(void *pvParameter)
{
    const char *TAG = "APP-TASK";

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

    ESP_ERROR_CHECK(sys.init(xQueueComp2SysCommandsHandle, xQueueSys2CompCommandsHandle, senseTaskHandle));

    while (1)
    {
        sys.loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}