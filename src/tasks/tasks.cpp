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

    /* Create queue to send command to system task */
    xQueueComp2SysCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                     ITEM_COMMANDS_SIZE,
                                                     ucQueueComp2SysCommandStorageArea,
                                                     &xStaticQueueComp2SysCommand);
    configASSERT(xQueueComp2SysCommandsHandle);

    /* Wait until queues have been created */
    while ((xQueueAcc2guiHandle == NULL) || (xQueueGui2AccCommandsHandle == NULL) || (xQueueAcc2guiCommandsHandle == NULL) || (xQueueComp2SysCommandsHandle == NULL))
        ;

    /* Create DPB first page interface */
    gui_init(xQueueAcc2guiHandle, xQueueGui2AccCommandsHandle, xQueueAcc2guiCommandsHandle, xQueueComp2SysCommandsHandle);

    while (1)
    {
        gui_chart_update();

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
    /* Create queue to share acceleration value to gui */
    xQueueAcc2guiHandle = xQueueCreateStatic(QUEUE_ACC2GUI_LENGTH,
                                             ITEM_ACC2GUI_SIZE,
                                             ucQueueAcc2guiStorageArea,
                                             &xStaticQueueAcc2gui);
    configASSERT(xQueueAcc2guiHandle);

    /* Create queue to recive command from gui */
    xQueueGui2AccCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                     ITEM_COMMANDS_SIZE,
                                                     ucQueueGui2AccCommandStorageArea,
                                                     &xStaticQueueGui2AccCommand);
    configASSERT(xQueueGui2AccCommandsHandle);

    /* Create queue to seend command to gui */
    xQueueAcc2guiCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                     ITEM_COMMANDS_SIZE,
                                                     ucQueueAcc2guiCommandsStorageArea,
                                                     &xStaticQueueAcc2guiCommands);
    configASSERT(xQueueAcc2guiCommandsHandle);

    acceleration_init(xQueueAcc2guiHandle, xQueueComp2SysCommandsHandle, xQueueGui2AccCommandsHandle);

    while (1)
    {
        acceleration_update();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

/********************************
 *        POSITION TASK         *
 ********************************/
void senseTask(void *pvParameter)
{
    uint32_t ulInterruptStatus;

    while (1)
    {
        ulTaskNotifyTakeIndexed(0,      /* Use the 0th notification */
                                pdTRUE, /* Clear the notification value
                                           before exiting. */
                                portMAX_DELAY);
        acceleration_start_read();
        gui_testvalue_increment();
        gpio_intr_disable(GPIO_OPT_SENSOR);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

/****************************
 *          APP TASK        *
 ****************************/
void application(void *pvParameter)
{
    /* Create queue to send command from system app to app components */
    xQueueSys2CompCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                     ITEM_COMMANDS_SIZE,
                                                     ucQueueSys2CompCommandStorageArea,
                                                     &xStaticQueueSys2CompCommand);
    configASSERT(xQueueSys2CompCommandsHandle);

    while ((xQueueComp2SysCommandsHandle == NULL) || (xQueueSys2CompCommandsHandle == NULL))
        ;

    ESP_ERROR_CHECK(app_init(&esc, GPIO_OPT_SENSOR, xQueueComp2SysCommandsHandle, xQueueSys2CompCommandsHandle, &accDataBuffer));

    esc.arm();

    while (1)
    {
        app_loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}