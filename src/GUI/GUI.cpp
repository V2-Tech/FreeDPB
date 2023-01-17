#include "GUI.h"
/***********************************
 *      VARIABLES DECALRATIONS     *
 ***********************************/
LGFX tft;
static QueueHandle_t _xQueueCom2Sys = NULL;
static QueueHandle_t _xQueueSys2Comp = NULL;
static uint8_t _gui_init_done = 0;
DPBShared &_xShared = DPBShared::getInstance();
static dpb_page _gui_act_page = IDLE;

lv_obj_t *gui_IdleScreen = NULL;
lv_obj_t *gui_FFTScreen = NULL;

lv_obj_t *gui_AccelChart = NULL;
lv_obj_t *gui_AccelChart_Xslider = NULL;
lv_obj_t *gui_AccelChart_Yslider = NULL;
lv_chart_series_t *serAccX = NULL;
lv_chart_series_t *serAccY = NULL;

lv_obj_t *gui_FFTChart = NULL;
lv_chart_series_t *serFFT = NULL;
int16_t fft_sample[ACC_DATA_BUFFER_SIZE / 2] = {0};

lv_obj_t *gui_RPMLabel = NULL;

lv_obj_t *gui_StartButLabel = NULL;

size_t pcnt;
int16_t accX_sample[ACC_DATA_BUFFER_SIZE] = {0};
int16_t accY_sample[ACC_DATA_BUFFER_SIZE] = {0};

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

uint8_t gui_init(QueueHandle_t xQueueComp2Sys_handle, QueueHandle_t xQueueSys2Comp_handle)
{
    uint8_t ret = 0;

    if ((xQueueComp2Sys_handle == NULL) || (xQueueSys2Comp_handle == NULL))
    {
        return ESP_FAIL;
    }

    _xQueueCom2Sys = xQueueComp2Sys_handle;
    _xQueueSys2Comp = xQueueSys2Comp_handle;

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              true, LV_FONT_DEFAULT);

    lv_disp_set_theme(dispp, theme);
    gui_IdleScreen_init();
    gui_show_page(IDLE);

    _gui_init_done = 1;

    return ret;
}

void gui_IdleScreen_init(void)
{
    /* Create IDLE SCREEN object */
    gui_IdleScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_IdleScreen, LV_OBJ_FLAG_SCROLLABLE);

    /* Create a chart with 2 traces for AccX e AccY */
    gui_AccelChart = lv_chart_create(gui_IdleScreen);
    lv_obj_set_width(gui_AccelChart, 280);
    lv_obj_set_height(gui_AccelChart, 140);
    lv_obj_set_x(gui_AccelChart, 15);
    lv_obj_set_y(gui_AccelChart, 20);
    lv_obj_set_style_bg_color(gui_AccelChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_AccelChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_AccelChart, lv_color_hex(0x3E3E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_AccelChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_AccelChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(gui_AccelChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(gui_AccelChart, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_update_mode(gui_AccelChart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(gui_AccelChart, LV_CHART_AXIS_PRIMARY_Y, -4096, 4096);

    serAccX = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    serAccY = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    pcnt = sizeof(accX_sample) / sizeof(accX_sample[0]);
    lv_chart_set_point_count(gui_AccelChart, pcnt);
    lv_chart_set_ext_y_array(gui_AccelChart, serAccX, (lv_coord_t *)accX_sample);
    lv_chart_set_ext_y_array(gui_AccelChart, serAccY, (lv_coord_t *)accY_sample);

    /* Create X-ScrollBar */
    gui_AccelChart_Xslider = lv_slider_create(gui_IdleScreen);
    lv_slider_set_range(gui_AccelChart_Xslider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(gui_AccelChart_Xslider, slider_x_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(gui_AccelChart_Xslider, 280, 10);
    lv_obj_align_to(gui_AccelChart_Xslider, gui_AccelChart, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    /* Create Y-ScrollBar */
    gui_AccelChart_Yslider = lv_slider_create(gui_IdleScreen);
    lv_slider_set_range(gui_AccelChart_Yslider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(gui_AccelChart_Yslider, slider_y_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(gui_AccelChart_Yslider, 10, 150);
    lv_obj_align_to(gui_AccelChart_Yslider, gui_AccelChart, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    /* Create start-stop button */
    lv_obj_t *btn = lv_btn_create(gui_IdleScreen); /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);                 /*Set its size*/
    lv_obj_align_to(btn, gui_AccelChart_Xslider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_event_cb(btn, start_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    gui_StartButLabel = lv_label_create(btn);      /*Add a label to the button*/
    lv_label_set_text(gui_StartButLabel, "START"); /*Set the labels text*/
    lv_obj_center(gui_StartButLabel);

    /* Create FFT button */
    btn = lv_btn_create(gui_IdleScreen); /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);       /*Set its size*/
    lv_obj_align_to(btn, gui_AccelChart_Xslider, LV_ALIGN_OUT_BOTTOM_LEFT, 160, 10);
    lv_obj_add_event_cb(btn, start_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "FFT");        /*Set the labels text*/
    lv_obj_center(label);

    /* Create RPM label */
    gui_RPMLabel = lv_label_create(gui_IdleScreen); /*Add a label to the button*/
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_bg_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_bg_opa(&label_style, LV_OPA_100);
    lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_AMBER));
    lv_style_set_text_font(&label_style, &lv_font_montserrat_14);

    lv_obj_add_style(gui_RPMLabel, &label_style, 0);
    lv_obj_set_pos(gui_RPMLabel, 15, 2);
    lv_obj_set_size(gui_RPMLabel, 100, 20);
    lv_label_set_text(gui_RPMLabel, "0"); /*Set the labels text*/
}

void gui_FFTScreen_init(void)
{
    /* Create IDLE SCREEN object */
    gui_FFTScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_FFTScreen, LV_OBJ_FLAG_SCROLLABLE);

    /* Create a chart with 2 traces for AccX e AccY */
    gui_FFTChart = lv_chart_create(gui_IdleScreen);
    lv_obj_set_width(gui_FFTChart, 280);
    lv_obj_set_height(gui_FFTChart, 140);
    lv_obj_set_x(gui_FFTChart, 15);
    lv_obj_set_y(gui_FFTChart, 20);
    lv_chart_set_type(gui_FFTChart, LV_CHART_TYPE_BAR);
    lv_obj_set_style_bg_color(gui_FFTChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_FFTChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_FFTChart, lv_color_hex(0x3E3E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_FFTChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_FFTChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(gui_FFTChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(gui_FFTChart, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_range(gui_FFTChart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);

    serFFT = lv_chart_add_series(gui_FFTChart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(gui_FFTChart, pcnt / 2);
    lv_chart_set_ext_y_array(gui_FFTChart, serFFT, (lv_coord_t *)fft_sample);

    /* Create back button */
    lv_obj_t *btn = lv_btn_create(gui_FFTScreen); /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);                 /*Set its size*/
    lv_obj_align_to(btn, gui_FFTChart, LV_ALIGN_OUT_BOTTOM_LEFT, 160, 10);
    lv_obj_add_event_cb(btn, back_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "Back");        /*Set the labels text*/
    lv_obj_center(label);
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

void start_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data command;
        command.command = START_BUT_CMD;
        command.value.ull = 0;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void fft_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data command;
        command.command = FFT_REQUEST_CMD;
        command.value.ull = 1;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void back_btn_event_cb(lv_event_t *e)
{
    gui_show_page(IDLE);
}

// lv_chart_set_next_value(gui_AccelChart, serAccZ, (lv_coord_t)chartData[2]);
void gui_update(void)
{
    if (!_gui_init_done)
    {
        return;
    }

    gui_check_commands();
    gui_values_update();
}

void gui_check_commands(void)
{
    command_data command;

    if (xQueueReceive(_xQueueSys2Comp, &command, (TickType_t)0) == pdPASS)
    {
        gui_exe(command);
    }
}

void gui_exe(command_data command)
{
    switch (command.command)
    {
    case ACC_CHART_UPDATE_CMD:
        gui_charts_update();
        break;
    case FFT_CHART_UPDATE_CMD:
        gui_fft_update();
        break;
    default:
        break;
    }
}

void gui_show_page(dpb_page page)
{
    switch (page)
    {
    case IDLE:
        lv_disp_load_scr(gui_IdleScreen);
        break;
    case FFT:
        lv_disp_load_scr(gui_FFTScreen);
        break;
    default:
        break;
    }

    _gui_act_page = page;
}

void gui_values_update(void)
{
    app_steps status = _xShared.getAppStatus();
    lv_label_set_text_fmt(gui_StartButLabel, status == IDLE ? "START" : "STOP");
    if (status == IDLE)
    {
        lv_label_set_text_fmt(gui_RPMLabel, "%d", 0);
    }
    else
    {
        lv_label_set_text_fmt(gui_RPMLabel, "%d", _xShared.getRPM());
    }
}

void gui_charts_update(void)
{
    dpb_acc_data d;

    for (size_t i = 0; i < pcnt; i++)
    {
        _xShared.getAccData(&d, i);
        accX_sample[i] = d.accel_data.acc_x;
        accY_sample[i] = d.accel_data.acc_y;
    }

    lv_chart_refresh(gui_AccelChart);
}

void gui_fft_update(void)
{
    if (_gui_act_page != FFT)
    {
        gui_show_page(FFT);
    }
    // TODO
}
