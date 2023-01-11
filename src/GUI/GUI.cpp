#include "GUI.h"

/***********************************
 *      VARIABLES DECALRATIONS     *
 ***********************************/
LGFX tft;
lv_obj_t *gui_MainScreen = NULL;
lv_obj_t *gui_AccelChart = NULL;
lv_chart_series_t *serAccX = NULL;
lv_chart_series_t *serAccY = NULL;
lv_chart_series_t *serAccZ = NULL;
lv_obj_t *gui_TestLabel = NULL;

uint16_t accX_sample[3000] = {0};
uint16_t accY_sample[3000] = {0};
uint16_t accZ_sample[3000] = {0};

static QueueHandle_t _xQueueCom2Sys = NULL;
static fft_chart_data *_pFFTOuput;

static int16_t testCnt = 0;

/************************************************************************/
/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    // tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* Read the touchpad */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY);

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;
        if (cnt > 1)
            cnt = 0;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, cnt ? "RUN" : "STOP");

        command_data command;
        if (cnt == 1)
        {

            command.command = APP_CMD;
            command.value = POS_SEARCH;
        }
        else
        {
            command.command = APP_CMD;
            command.value = IDLE;
        }

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

/**
 * Create a button with a label and react on click event.
 */
void lv_example_get_started_1(void)
{
    gui_MainScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_MainScreen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *btn = lv_btn_create(gui_MainScreen); /*Add a button the current screen*/
    // lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 120, 50);                              /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "Button");     /*Set the labels text*/
    lv_obj_center(label);
}

void btn_test_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        testCnt = 0;
        lv_label_set_text_fmt(gui_TestLabel, "%d", testCnt);
        gpio_intr_enable(GPIO_OPT_SENSOR);
    }
}

uint8_t gui_init(QueueHandle_t xQueueCom2Sys_handle, fft_chart_data *pFFTOuput)
{
    uint8_t ret = 0;

    if ((xQueueCom2Sys_handle == NULL) || (pFFTOuput == NULL))
    {
        return ESP_FAIL;
    }

    _xQueueCom2Sys = xQueueCom2Sys_handle;
    _pFFTOuput = pFFTOuput;

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              true, LV_FONT_DEFAULT);

    lv_disp_set_theme(dispp, theme);
    gui_MainScreen_init();
    lv_disp_load_scr(gui_MainScreen);

    return ret;
}

void gui_MainScreen_init(void)
{
    /* Create MAIN SCREEN */
    gui_MainScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_MainScreen, LV_OBJ_FLAG_SCROLLABLE);

    /* Create a chart with 3 traces */
    gui_AccelChart = lv_chart_create(gui_MainScreen);
    lv_obj_set_width(gui_AccelChart, 200);
    lv_obj_set_height(gui_AccelChart, 150);
    lv_obj_set_x(gui_AccelChart, 10);
    lv_obj_set_y(gui_AccelChart, 10);
    lv_obj_set_style_bg_color(gui_AccelChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_AccelChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_AccelChart, lv_color_hex(0x3E3E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_AccelChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_AccelChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(gui_AccelChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(gui_AccelChart, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_update_mode(gui_AccelChart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(gui_AccelChart, LV_CHART_AXIS_PRIMARY_Y, -16384, 16384);

    serAccX = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    serAccY = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    serAccZ = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);

    uint32_t pcnt = sizeof(accX_sample) / sizeof(accX_sample[0]);
    lv_chart_set_point_count(gui_AccelChart, pcnt);
    lv_chart_set_ext_y_array(gui_AccelChart, serAccX, (lv_coord_t *)accX_sample);
    lv_chart_set_ext_y_array(gui_AccelChart, serAccY, (lv_coord_t *)accY_sample);
    lv_chart_set_ext_y_array(gui_AccelChart, serAccZ, (lv_coord_t *)accZ_sample);

    /* Create X-ScrollBar */
    lv_obj_t *gui_AccelChart_Slider;
    gui_AccelChart_Slider = lv_slider_create(gui_MainScreen);
    lv_slider_set_range(gui_AccelChart_Slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(gui_AccelChart_Slider, slider_x_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(gui_AccelChart_Slider, 200, 10);
    lv_obj_align_to(gui_AccelChart_Slider, gui_AccelChart, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    /* Create Y-ScrollBar */
    gui_AccelChart_Slider = lv_slider_create(gui_MainScreen);
    lv_slider_set_range(gui_AccelChart_Slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(gui_AccelChart_Slider, slider_y_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(gui_AccelChart_Slider, 10, 150);
    lv_obj_align_to(gui_AccelChart_Slider, gui_AccelChart, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    /* Create start-stop button */
    lv_obj_t *btn = lv_btn_create(gui_MainScreen); /*Add a button the current screen*/
    lv_obj_align_to(btn, gui_AccelChart_Slider, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_obj_set_size(btn, 50, 50);                               /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "STOP");       /*Set the labels text*/
    lv_obj_center(label);

    /* Create interrupt count button */
    btn = lv_btn_create(gui_MainScreen); /*Add a button the current screen*/
    lv_obj_align_to(btn, gui_AccelChart_Slider, LV_ALIGN_OUT_RIGHT_BOTTOM, 20, 0);
    lv_obj_set_size(btn, 50, 50);                                    /*Set its size*/
    lv_obj_add_event_cb(btn, btn_test_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    gui_TestLabel = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text_fmt(gui_TestLabel, "%d", testCnt);
    lv_obj_center(gui_TestLabel);
}

void slider_x_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    lv_chart_set_zoom_x(gui_AccelChart, v);
}

void slider_y_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    lv_chart_set_zoom_y(gui_AccelChart, v);
}

void gui_chart_update(void)
{
    int16_t chartData[3] = {0};

    /* while (uxQueueMessagesWaiting(_xQueueData2GUI) != 0)
    {
        if (xQueueReceive(_xQueueData2GUI,
                          chartData,
                          (TickType_t)0) == pdPASS)
        {
            lv_chart_set_next_value(gui_AccelChart, serAccX, (lv_coord_t)chartData[0]);
            lv_chart_set_next_value(gui_AccelChart, serAccY, (lv_coord_t)chartData[1]);
            lv_chart_set_next_value(gui_AccelChart, serAccZ, (lv_coord_t)chartData[2]);
        }
    } */
}

void gui_testvalue_increment(void)
{
    testCnt++;
    lv_label_set_text_fmt(gui_TestLabel, "%d", testCnt);
}
