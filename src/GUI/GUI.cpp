#include "GUI.h"
/***********************************
 *      VARIABLES DECALRATIONS     *
 ***********************************/
LGFX tft;
static QueueHandle_t _xQueueCom2Sys = NULL;
static QueueHandle_t _xQueueSys2Comp = NULL;
static uint8_t _gui_init_done = 0;
DPBShared &_xShared = DPBShared::getInstance();
static dpb_page _gui_act_page = IDLE_PAGE;
static uint8_t _peak_draw_done = 0;

lv_obj_t *gui_IdleScreen = NULL;
lv_obj_t *gui_FFTScreen = NULL;

lv_obj_t *gui_AccelChart = NULL;
lv_obj_t *gui_AccelChart_Xslider = NULL;
lv_obj_t *gui_AccelChart_Yslider = NULL;
lv_chart_series_t *serAccX = NULL;
lv_chart_series_t *serAccY = NULL;

lv_obj_t *gui_FFTXChart = NULL;
lv_obj_t *gui_FFTYChart = NULL;
lv_chart_series_t *serFFTX = NULL;
lv_chart_series_t *serFFTY = NULL;
int16_t fftX_sample[ACC_DATA_BUFFER_SIZE / 2] = {0};
int16_t fftY_sample[ACC_DATA_BUFFER_SIZE / 2] = {0};

lv_obj_t *gui_RPMLabel = NULL;

lv_obj_t *gui_StartButLabel = NULL;

size_t pcnt;
int16_t accX_sample[ACC_CHART_POINT_COUNT] = {0};
int16_t accY_sample[ACC_CHART_POINT_COUNT] = {0};

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
    gui_FFTScreen_init();
    gui_show_page(IDLE_PAGE);

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
    //lv_chart_set_zoom_x(gui_AccelChart, 1400);                                         // Zoom in a little in X
    lv_obj_add_event_cb(gui_AccelChart, AccelChart_draw_event_cb, LV_EVENT_ALL, NULL); //? Event to be able to draw the peak points

    serAccY = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    serAccX = lv_chart_add_series(gui_AccelChart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

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
    //lv_slider_set_value(gui_AccelChart_Xslider, 1400, LV_ANIM_OFF);

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
    lv_obj_set_size(btn, 50, 50);        /*Set its size*/
    lv_obj_align_to(btn, gui_AccelChart_Xslider, LV_ALIGN_OUT_BOTTOM_LEFT, 130, 10);
    lv_obj_add_event_cb(btn, fft_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "FFT");        /*Set the labels text*/
    lv_obj_center(label);

    /* Create LPF button */
    btn = lv_btn_create(gui_IdleScreen); /*Add a button the current screen*/
    lv_obj_set_size(btn, 50, 50);        /*Set its size*/
    lv_obj_align_to(btn, gui_AccelChart_Xslider, LV_ALIGN_OUT_BOTTOM_LEFT, 190, 10);
    lv_obj_add_event_cb(btn, filter_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    label = lv_label_create(btn);    /*Add a label to the button*/
    lv_label_set_text(label, "LPF"); /*Set the labels text*/
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

    /* Create a 2 bar chart with traces for FFTX e FFTY */
    gui_FFTXChart = lv_chart_create(gui_FFTScreen);
    lv_obj_set_width(gui_FFTXChart, 280);
    lv_obj_set_height(gui_FFTXChart, 120);
    lv_obj_set_x(gui_FFTXChart, 15);
    lv_obj_set_y(gui_FFTXChart, 20);
    // lv_chart_set_type(gui_FFTXChart, LV_CHART_TYPE_BAR);
    lv_obj_set_style_bg_color(gui_FFTXChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_FFTXChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_FFTXChart, lv_color_hex(0x3E3E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_FFTXChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_FFTXChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(gui_FFTXChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(gui_FFTXChart, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_range(gui_FFTXChart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_axis_tick(gui_FFTXChart, LV_CHART_AXIS_PRIMARY_X, 10, 5, FFT_MAJOR_TICK_COUNT, 5, true, 50);
    lv_obj_add_event_cb(gui_FFTXChart, FFTXChart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL); //? Event to be able to customized the tick's labels

    serFFTX = lv_chart_add_series(gui_FFTXChart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(gui_FFTXChart, ACC_DATA_BUFFER_SIZE / 2);
    lv_chart_set_ext_y_array(gui_FFTXChart, serFFTX, (lv_coord_t *)fftX_sample);

    gui_FFTYChart = lv_chart_create(gui_FFTScreen);
    lv_obj_set_width(gui_FFTYChart, 280);
    lv_obj_set_height(gui_FFTYChart, 70);
    lv_obj_align_to(gui_FFTYChart, gui_FFTXChart, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_chart_set_type(gui_FFTYChart, LV_CHART_TYPE_BAR);
    lv_obj_set_style_bg_color(gui_FFTYChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_FFTYChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_FFTYChart, lv_color_hex(0x3E3E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_FFTYChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_FFTYChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(gui_FFTYChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(gui_FFTYChart, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_range(gui_FFTYChart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_axis_tick(gui_FFTYChart, LV_CHART_AXIS_PRIMARY_X, 10, 5, FFT_MAJOR_TICK_COUNT, 5, true, 50);
    lv_obj_add_event_cb(gui_FFTYChart, FFTYChart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL); //? Event to be able to customized the tick's labels

    serFFTY = lv_chart_add_series(gui_FFTYChart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(gui_FFTYChart, pcnt / 2);
    lv_chart_set_ext_y_array(gui_FFTYChart, serFFTY, (lv_coord_t *)fftX_sample);

    //! Hidden
    lv_obj_add_flag(gui_FFTYChart, LV_OBJ_FLAG_HIDDEN);

    /* Create back button */
    lv_obj_t *btn = lv_btn_create(gui_FFTScreen); /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);                /*Set its size*/
    lv_obj_set_pos(btn, 10, screenHeight - (50 + 5));
    // lv_obj_align_to(btn, gui_FFTYChart, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_event_cb(btn, back_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "Back");       /*Set the labels text*/
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
        gui_show_page(FFT_PAGE);

        command_data command;
        command.command = FFT_REQUEST_CMD;
        command.value.ull = 1;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void filter_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data command;
        command.command = LPF_REQUEST_CMD;
        command.value.ull = 1;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void back_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        gui_show_page(IDLE_PAGE);
    }
}

void FFTXChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

        if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
            return;

        if (dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text)
        {
            uint16_t sampleRate = _xShared.getSampleRate();
            const char *labels[FFT_MAJOR_TICK_COUNT] = {"0", "50", "100", "150", "200", "250", "300", "350", "400", "450", "500"};
            lv_snprintf(dsc->text, dsc->text_length, "%s", labels[dsc->value]);
        }
    }
}

void FFTYChart_draw_event_cb(lv_event_t *e)
{
    ;
}

void AccelChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_invalidate(chart);
    }
    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        lv_coord_t *s = (lv_coord_t *)lv_event_get_param(e);
        *s = LV_MAX(*s, 20);
    }
    else if (code == LV_EVENT_DRAW_POST_END)
    {
        lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
        while (ser && !_peak_draw_done)
        {
            size_t peakCount = 0;

            for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE; i++)
            {
                size_t v = _xShared.getPeakIndex(i);
                if ((v == 0) || (v > ACC_CHART_POINT_COUNT - 1))
                {
                    break;
                }
                peakCount++;
            }

            if (peakCount == 0)
            {
                ser = lv_chart_get_series_next(chart, ser);
                continue;
            }

            lv_point_t *p = new lv_point_t[peakCount]();

            for (size_t i = 0; i < peakCount; i++)
            {
                lv_chart_get_point_pos_by_id(chart, ser, _xShared.getPeakIndex(i), &p[i]);
            }

            for (size_t i = 0; i < peakCount; i++)
            {
                lv_draw_rect_dsc_t draw_rect_dsc;
                lv_draw_rect_dsc_init(&draw_rect_dsc);
                draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_GREEN);
                draw_rect_dsc.bg_opa = LV_OPA_100;
                draw_rect_dsc.radius = LV_RADIUS_CIRCLE;

                lv_area_t a;
                a.x1 = chart->coords.x1 + p[i].x - 2;
                a.x2 = chart->coords.x1 + p[i].x + 2;
                a.y1 = chart->coords.y1 + p[i].y - 2;
                a.y2 = chart->coords.y1 + p[i].y + 2;

                lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
                lv_draw_rect(draw_ctx, &draw_rect_dsc, &a);
            }

            ser = lv_chart_get_series_next(chart, ser);

            delete[] p;
        }

        //_peak_draw_done = 1;
    }
    else if (code == LV_EVENT_RELEASED)
    {
        lv_obj_invalidate(chart);
    }
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
    case IDLE_PAGE:
        lv_disp_load_scr(gui_IdleScreen);
        break;
    case FFT_PAGE:
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
    int16_t min = 0;
    int16_t max = 0;

    for (size_t i = 0; i < pcnt; i++)
    {
        _xShared.getDPBAccDataFiltered(&d, i);
        accX_sample[i] = d.accel_data.acc_x;
        accY_sample[i] = d.accel_data.acc_y;
        if ((accX_sample[i] > max) || (accY_sample[i] > max))
        {
            (accX_sample[i] > accY_sample[i]) ? max = accX_sample[i] : max = accY_sample[i];
        }
        if ((accX_sample[i] < min) || (accY_sample[i] < min))
        {
            (accX_sample[i] < accY_sample[i]) ? min = accX_sample[i] : min = accY_sample[i];
        }
    }

    lv_chart_set_range(gui_AccelChart, LV_CHART_AXIS_PRIMARY_Y, min * 1.1, max * 1.1);
    lv_chart_refresh(gui_AccelChart);
}

void gui_fft_update(void)
{
    int16_t min = 0;
    int16_t max = 0;

    for (size_t i = 0; i < ACC_DATA_BUFFER_SIZE / 2; i++)
    {
        fftX_sample[i] = static_cast<int16_t>(_xShared.getFFTX(i));
        fftY_sample[i] = static_cast<int16_t>(_xShared.getFFTY(i));
        if ((fftX_sample[i] > max) || (fftY_sample[i] > max))
        {
            (fftX_sample[i] > fftY_sample[i]) ? max = fftX_sample[i] : max = fftY_sample[i];
        }
        if ((fftX_sample[i] < min) || (fftY_sample[i] < min))
        {
            (fftX_sample[i] < fftY_sample[i]) ? min = fftX_sample[i] : min = fftY_sample[i];
        }
    }

    lv_chart_set_range(gui_FFTXChart, LV_CHART_AXIS_PRIMARY_Y, min, max);
    lv_chart_refresh(gui_FFTXChart);
    lv_chart_set_range(gui_FFTYChart, LV_CHART_AXIS_PRIMARY_Y, min, max);
    lv_chart_refresh(gui_FFTYChart);
}

void _chart_Y_autorange(lv_obj_t *chart_obj, lv_chart_series_t *ser)
{
    int16_t min = 0;
    int16_t max = 0;

    lv_chart_t *chart = (lv_chart_t *)chart_obj;
    lv_coord_t *ser_array = lv_chart_get_y_array(chart_obj, ser);

    for (size_t i = 0; i < chart->point_cnt; i++)
    {
        int16_t v;
        v = ser_array[i];
        if (v > max)
        {
            max = v;
        }
        if (v < min)
        {
            min = v;
        }
    }

    lv_chart_set_range(chart_obj, LV_CHART_AXIS_PRIMARY_Y, min, max);
}

void _ask_peak_draw(void)
{
    _peak_draw_done = 0;
}
