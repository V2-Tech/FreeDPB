#include "tasks.h"

/*********************
 *      VARIABLES
 *********************/
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

static StaticQueue_t xStaticQueueAcc2gui;
uint8_t ucQueueAcc2guiStorageArea[QUEUE_ACC2GUI_LENGTH * ITEM_ACC2GUI_SIZE];
QueueHandle_t xQueueAcc2guiHandle;

static StaticQueue_t xStaticQueueAcc2guiCommands;
uint8_t ucQueueAcc2guiCommandsStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueAcc2guiCommandsHandle;

static StaticQueue_t xStaticQueueGui2AccCommand;
uint8_t ucQueueGui2AccCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueGui2AccCommandsHandle;

static StaticQueue_t xStaticQueueGui2SysCommand;
uint8_t ucQueueGui2SysCommandStorageArea[QUEUE_COMMANDS_LENGTH * ITEM_COMMANDS_SIZE];
QueueHandle_t xQueueGui2SysCommandsHandle;

static Motor esc(4, DSHOT300);

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
    xQueueGui2SysCommandsHandle = xQueueCreateStatic(QUEUE_COMMANDS_LENGTH,
                                                     ITEM_COMMANDS_SIZE,
                                                     ucQueueGui2SysCommandStorageArea,
                                                     &xStaticQueueGui2SysCommand);
    configASSERT(xQueueGui2SysCommandsHandle);

    /* Wait until queues have been created */
    while ((xQueueAcc2guiHandle == NULL) && (xQueueGui2AccCommandsHandle == NULL) && (xQueueAcc2guiCommandsHandle == NULL) && (xQueueGui2SysCommandsHandle == NULL))
        ;

    /* Create DPB first page interface */
    gui_init(xQueueAcc2guiHandle, xQueueGui2AccCommandsHandle, xQueueAcc2guiCommandsHandle, xQueueGui2SysCommandsHandle);

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
 *          ACCELERATIONS TASK          *
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

    acceleration_init(xQueueAcc2guiHandle, xQueueAcc2guiCommandsHandle, xQueueGui2AccCommandsHandle);

    while (1)
    {
        acceleration_update();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

/********************************
 *          FILTER TASK         *
 ********************************/
void filterTask(void *pvParameter)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelete(NULL);
}

/****************************
 *          ESC TASK        *
 ****************************/
void motorTask(void *pvParameter)
{
    static uint16_t thro = 0;
    command_data command;

    while ((xQueueGui2SysCommandsHandle == NULL))
        ;

    esc.arm();

    while (1)
    {
        static uint8_t status = 0;
        static uint8_t rev = 0;

        if (xQueueReceive(xQueueGui2SysCommandsHandle,
                          &command,
                          0) == pdPASS)
        {
            if (command.value == 0x0A)
            {
                status = 1;
            }
            else
            {
                status = 0;
            }
        }

        if (status == 1)
        {
            if (rev == 1)
            {
                thro -= 10;
            }
            else
            {
                thro += 10;
            }

            if (thro > 1000)
            {
                rev = 1;
            }
            if (thro < 100)
            {
                rev = 0;
            }

            esc.set_throttle(thro);
        }
        else
        {
            esc.set_throttle(0);
            thro = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}