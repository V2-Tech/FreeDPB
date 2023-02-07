#include "GUI.h"

//************************/
//*      VARIABLES       */
//************************/
//* Inter-pages usage
static LGFX tft;
DPBShared &_xShared = DPBShared::getInstance();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];
static QueueHandle_t _xQueueCom2Sys = NULL;
static QueueHandle_t _xQueueSys2Comp = NULL;
static uint8_t _gui_init_done = 0;
static dpb_page_t _gui_act_page = LOADING_PAGE;
static uint8_t _peak_draw_done = 0;

//* Pages handler
lv_obj_t *gui_LoadingScreen = NULL;
lv_obj_t *gui_MainScreen = NULL;
lv_obj_t *gui_NerdScreen = NULL;
lv_obj_t *gui_FFTScreen = NULL;

//* Main screen object
lv_obj_t *gui_StartBut = NULL;
lv_obj_t *gui_StartButLabel = NULL;
lv_obj_t *gui_SettingsBut = NULL;
lv_obj_t *gui_NerdBut = NULL;
lv_obj_t *gui_RPMLabel = NULL;
lv_obj_t *gui_RPMLabelText = NULL;
lv_obj_t *gui_UnbalanceAngleTab = NULL;
lv_obj_t *gui_UnbalanceAngleLine = NULL;
lv_obj_t *gui_UnbalanceAngleLineEnd = NULL;
lv_obj_t *gui_UnbalanceLabValue = NULL;

//* Nerd screen object
lv_obj_t *gui_page_signal_x = NULL;
lv_obj_t *gui_page_signal_y = NULL;
lv_obj_t *gui_page_fft_x = NULL;
lv_obj_t *gui_page_fft_y = NULL;
lv_obj_t *gui_action_list_nerd = NULL;

//* Idle screen objects
// Acceleration charts
lv_obj_t *gui_AccelXChart = NULL;
lv_obj_t *gui_AccelYChart = NULL;
lv_chart_series_t *serAccX = NULL;
lv_chart_series_t *serAccY = NULL;
int16_t accX_sample[ACC_CHART_POINT_COUNT] = {0};
int16_t accY_sample[ACC_CHART_POINT_COUNT] = {0};
lv_obj_t *gui_AccelChart_Xslider = NULL;
lv_obj_t *gui_AccelChart_Yslider = NULL;

//* FFT screen objects
// Fourier charts
lv_obj_t *gui_FFTXChart = NULL;
lv_obj_t *gui_FFTYChart = NULL;
lv_chart_series_t *serFFTX = NULL;
lv_chart_series_t *serFFTY = NULL;
int16_t fftX_sample[FFT_DATA_BUFFER_SIZE] = {0};
int16_t fftY_sample[FFT_DATA_BUFFER_SIZE] = {0};
// Buttons

// Labels
lv_obj_t *gui_FundLabel = NULL;

//* Images declarations
LV_IMG_DECLARE(prop);

//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
//?         FUNCTIONS DEFINITION        /
//?^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/
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

    _display_init();

    lv_disp_t *disp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              true, LV_FONT_DEFAULT);

    lv_disp_set_theme(disp, theme);
    gui_LoadingScreen_init();
    gui_MainScreen_init();
    gui_NerdScreen_init();
    gui_show_page(LOADING_PAGE);

    _gui_init_done = 1;

    return ret;
}

void gui_LoadingScreen_init(void)
{
    //* Create LOADING SCREEN object
    gui_LoadingScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_LoadingScreen, LV_OBJ_FLAG_SCROLLABLE);

    //* Set screen style
    lv_obj_set_style_bg_color(gui_LoadingScreen, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);

    //* Create spinner
    lv_obj_t *spinner = lv_spinner_create(gui_LoadingScreen, 1000, 60);
    lv_obj_clear_flag(spinner, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(spinner, 25, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 25, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x11A3AB), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(spinner, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_size(spinner, 200, 200);
    lv_obj_center(spinner);
}

void gui_MainScreen_init(void)
{
    //* Create MAIN SCREEN object
    gui_MainScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_MainScreen, LV_OBJ_FLAG_SCROLLABLE);
    // Set screen style
    lv_obj_set_style_bg_color(gui_MainScreen, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);

    //* Create toolbars
    _create_toolbars_main();

    //* Create angle chart
    _create_anglechart_main();

    //* Create settings button
    gui_SettingsBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_SettingsBut, 25);
    lv_obj_set_height(gui_SettingsBut, 25);

    // Add default styles
    static lv_style_t styleSettingsBut;
    lv_style_init(&styleSettingsBut);
    lv_style_set_radius(&styleSettingsBut, 3);
    lv_style_set_bg_opa(&styleSettingsBut, LV_OPA_TRANSP);
    lv_style_set_border_opa(&styleSettingsBut, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&styleSettingsBut, LV_OPA_COVER);
    lv_style_set_outline_color(&styleSettingsBut, DEFAULT_ELEMENT_ACCENT_COLOR);

    // Add pressed styles
    static lv_style_t styleSettingsBut_pressed;
    lv_style_init(&styleSettingsBut_pressed);
    lv_style_set_outline_width(&styleSettingsBut_pressed, 10);
    lv_style_set_outline_opa(&styleSettingsBut_pressed, LV_OPA_TRANSP);

    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
    lv_style_set_transition(&styleSettingsBut_pressed, &trans);

    lv_obj_remove_style_all(gui_SettingsBut); /*Remove the style coming from the theme*/
    lv_obj_add_style(gui_SettingsBut, &styleSettingsBut, 0);
    lv_obj_add_style(gui_SettingsBut, &styleSettingsBut_pressed, LV_STATE_PRESSED);
    lv_obj_align(gui_SettingsBut, LV_ALIGN_TOP_LEFT, 290, 0);

    // Add icon
    lv_obj_t *gui_SettingsButImg = lv_img_create(gui_SettingsBut);
    lv_img_set_src(gui_SettingsButImg, &settings_icon);
    lv_obj_center(gui_SettingsButImg);

    //* Create start button
    gui_StartBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_StartBut, 100);
    lv_obj_set_height(gui_StartBut, 75);

    // Add default styles
    static lv_style_t styleStartBut;
    lv_style_init(&styleStartBut);
    lv_style_set_radius(&styleStartBut, 3);
    lv_style_set_bg_opa(&styleStartBut, LV_OPA_TRANSP);
    lv_style_set_border_color(&styleStartBut, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_style_set_border_width(&styleStartBut, 2);
    lv_style_set_border_opa(&styleStartBut, LV_OPA_100);
    lv_style_set_outline_opa(&styleStartBut, LV_OPA_COVER);
    lv_style_set_outline_color(&styleStartBut, SECONDARY_ELEMENT_ACCENT_COLOR);
    lv_style_set_shadow_width(&styleStartBut, 5);
    lv_style_set_shadow_color(&styleStartBut, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_style_set_shadow_opa(&styleStartBut, LV_OPA_100);

    // Add pressed styles
    static lv_style_t styleStartBut_pressed;
    lv_style_init(&styleStartBut_pressed);
    lv_style_set_outline_width(&styleStartBut_pressed, 15);
    lv_style_set_outline_opa(&styleStartBut_pressed, LV_OPA_TRANSP);

    static lv_style_transition_dsc_t trans_start_but;
    static lv_style_prop_t props_start_but[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans_start_but, props_start_but, lv_anim_path_linear, 300, 0, NULL);
    lv_style_set_transition(&styleStartBut_pressed, &trans_start_but);
    lv_obj_add_style(gui_StartBut, &styleStartBut, 0);
    lv_obj_add_style(gui_StartBut, &styleStartBut_pressed, LV_STATE_PRESSED);
    lv_obj_align(gui_StartBut, LV_ALIGN_TOP_LEFT, 210, 45);

    // Add event
    lv_obj_add_event_cb(gui_StartBut, start_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    gui_StartButLabel = lv_label_create(gui_StartBut);
    lv_label_set_text(gui_StartButLabel, LV_SYMBOL_PLAY);
    static lv_style_t styleStartButLabel;
    lv_style_set_text_font(&styleStartButLabel, &lv_font_montserrat_36);
    lv_style_set_text_color(&styleStartButLabel, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_obj_add_style(gui_StartButLabel, &styleStartButLabel, 0);
    lv_obj_center(gui_StartButLabel);

    //* Create magnitude label

    //* Create unbalance angle label
}

void gui_NerdScreen_init(void)
{
    //* Create NERD SCREEN object
    gui_NerdScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_NerdScreen, LV_OBJ_FLAG_SCROLLABLE);
    // Set screen style
    lv_obj_set_style_bg_color(gui_NerdScreen, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);

    //* Create sub-pages
    _create_pages_nerd();

    //* Create toolbars
    _create_toolbars_nerd();
}

// void gui_IdleScreen_init(void)
//{
//     //* Create IDLE SCREEN object
//     gui_IdleScreen = lv_obj_create(NULL);
//     lv_obj_clear_flag(gui_IdleScreen, LV_OBJ_FLAG_SCROLLABLE);
//
//     //* Create a chart with 2 traces for AccX e AccY
//     gui_AccelXChart = lv_chart_create(gui_IdleScreen);
//     lv_obj_set_width(gui_AccelXChart, 280);
//     lv_obj_set_height(gui_AccelXChart, 140);
//     lv_obj_set_x(gui_AccelXChart, 15);
//     lv_obj_set_y(gui_AccelXChart, 20);
//     lv_obj_set_style_bg_color(gui_AccelXChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_opa(gui_AccelXChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_shadow_color(gui_AccelXChart, lv_color_hex(0x3E3E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_shadow_opa(gui_AccelXChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_shadow_width(gui_AccelXChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_shadow_spread(gui_AccelXChart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_size(gui_AccelXChart, 0, LV_PART_INDICATOR); // Do not display points on the data
//     lv_chart_set_update_mode(gui_AccelXChart, LV_CHART_UPDATE_MODE_SHIFT);
//     lv_chart_set_range(gui_AccelXChart, LV_CHART_AXIS_PRIMARY_Y, -4096, 4096);
//     // lv_chart_set_zoom_x(gui_AccelXChart, 1400);                                         // Zoom in a little in X
//     lv_obj_add_event_cb(gui_AccelXChart, AccelChart_draw_event_cb, LV_EVENT_ALL, NULL); //? Event to be able to draw the peak points
//
//     serAccX = lv_chart_add_series(gui_AccelXChart, ACCX_TRACE_COLOR, LV_CHART_AXIS_PRIMARY_Y);
//     serAccY = lv_chart_add_series(gui_AccelXChart, ACCY_TRACE_COLOR, LV_CHART_AXIS_PRIMARY_Y);
//
//     lv_chart_set_point_count(gui_AccelXChart, ACC_CHART_POINT_COUNT);
//     lv_chart_set_ext_y_array(gui_AccelXChart, serAccX, (lv_coord_t *)accX_sample);
//     lv_chart_set_ext_y_array(gui_AccelXChart, serAccY, (lv_coord_t *)accY_sample);
//
//     //* Create X-ScrollBar
//     gui_AccelChart_Xslider = lv_slider_create(gui_IdleScreen);
//     lv_slider_set_range(gui_AccelChart_Xslider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
//     lv_obj_add_event_cb(gui_AccelChart_Xslider, chart_slider_x_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
//     lv_obj_set_size(gui_AccelChart_Xslider, 280, 10);
//     lv_obj_align_to(gui_AccelChart_Xslider, gui_AccelXChart, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
//     // lv_slider_set_value(gui_AccelChart_Xslider, 1400, LV_ANIM_OFF);
//
//     //* Create Y-ScrollBar
//     gui_AccelChart_Yslider = lv_slider_create(gui_IdleScreen);
//     lv_slider_set_range(gui_AccelChart_Yslider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
//     lv_obj_add_event_cb(gui_AccelChart_Yslider, chart_slider_y_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
//     lv_obj_set_size(gui_AccelChart_Yslider, 10, 150);
//     lv_obj_align_to(gui_AccelChart_Yslider, gui_AccelXChart, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//
//     //* Create start-stop button
//     lv_obj_t *btn = lv_btn_create(gui_IdleScreen); /*Add a button the current screen*/
//     lv_obj_set_size(btn, 120, 50);                 /*Set its size*/
//     lv_obj_align_to(btn, gui_AccelChart_Xslider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
//     lv_obj_add_event_cb(btn, start_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/
//
//     gui_StartButLabel = lv_label_create(btn);      /*Add a label to the button*/
//     lv_label_set_text(gui_StartButLabel, "START"); /*Set the labels text*/
//     lv_obj_center(gui_StartButLabel);
//
//     //* Create FFT button
//     btn = lv_btn_create(gui_IdleScreen); /*Add a button the current screen*/
//     lv_obj_set_size(btn, 50, 50);        /*Set its size*/
//     lv_obj_align_to(btn, gui_AccelChart_Xslider, LV_ALIGN_OUT_BOTTOM_LEFT, 130, 10);
//     lv_obj_add_event_cb(btn, fft_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/
//
//     lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
//     lv_label_set_text(label, "FFT");        /*Set the labels text*/
//     lv_obj_center(label);
//
//     //* Create RPM label
//     gui_RPMLabel = lv_label_create(gui_IdleScreen); /*Add a label to the button*/
//     static lv_style_t label_style;
//     lv_style_init(&label_style);
//     lv_style_set_bg_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
//     // lv_style_set_bg_opa(&label_style, LV_OPA_100);
//     lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_AMBER));
//     lv_style_set_text_font(&label_style, &lv_font_montserrat_14);
//
//     lv_obj_add_style(gui_RPMLabel, &label_style, 0);
//     lv_obj_set_pos(gui_RPMLabel, 15, 2);
//     lv_obj_set_size(gui_RPMLabel, 35, 20);
//     lv_label_set_text(gui_RPMLabel, "0rpm"); /*Set the labels text*/
//
//     //* Create unbalance X angle
//     gui_angleXLabel = lv_label_create(gui_IdleScreen); /*Add a label to the button*/
//     lv_style_init(&label_style);
//     lv_style_set_bg_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
//     // lv_style_set_bg_opa(&label_style, LV_OPA_100);
//     lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_TEAL));
//     lv_style_set_text_font(&label_style, &lv_font_montserrat_14);
//
//     lv_obj_add_style(gui_angleXLabel, &label_style, 0);
//     lv_obj_align_to(gui_angleXLabel, gui_RPMLabel, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//     lv_obj_set_size(gui_angleXLabel, 100, 20);
//     lv_label_set_text(gui_angleXLabel, "X-angle: NaN°"); /*Set the labels text*/
//
//     //* Create unbalance X angle
//     gui_angleYLabel = lv_label_create(gui_IdleScreen); /*Add a label to the button*/
//     lv_style_init(&label_style);
//     lv_style_set_bg_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
//     // lv_style_set_bg_opa(&label_style, LV_OPA_100);
//     lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_PURPLE));
//     lv_style_set_text_font(&label_style, &lv_font_montserrat_14);
//
//     lv_obj_add_style(gui_angleYLabel, &label_style, 0);
//     lv_obj_align_to(gui_angleYLabel, gui_angleXLabel, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
//     lv_obj_set_size(gui_angleYLabel, 100, 20);
//     lv_label_set_text(gui_angleYLabel, "Y-angle: NaN°"); /*Set the labels text*/
// }

void gui_FFTScreen_init(void)
{
    //* Create FFT SCREEN object
    gui_FFTScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_FFTScreen, LV_OBJ_FLAG_SCROLLABLE);

    //* Create chart with traces for FFTX
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

    lv_chart_set_point_count(gui_FFTXChart, FFT_DATA_BUFFER_SIZE);
    lv_chart_set_ext_y_array(gui_FFTXChart, serFFTX, (lv_coord_t *)fftX_sample);

    //* Create chart with traces for FFTY
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
    lv_chart_set_axis_tick(gui_FFTYChart, LV_CHART_AXIS_PRIMARY_X, 12, 5, FFT_MAJOR_TICK_COUNT, 10, true, 20);
    lv_obj_add_event_cb(gui_FFTYChart, FFTYChart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL); //? Event to be able to customized the tick's labels

    serFFTY = lv_chart_add_series(gui_FFTYChart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(gui_FFTYChart, FFT_DATA_BUFFER_SIZE);
    lv_chart_set_ext_y_array(gui_FFTYChart, serFFTY, (lv_coord_t *)fftX_sample);

    //! Hidden
    lv_obj_add_flag(gui_FFTYChart, LV_OBJ_FLAG_HIDDEN);

    //* Create back button
    lv_obj_t *btn = lv_btn_create(gui_FFTScreen); /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);                /*Set its size*/
    lv_obj_set_pos(btn, 10, screenHeight - (50 + 5));
    // lv_obj_align_to(btn, gui_FFTYChart, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_event_cb(btn, root_back_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "Back");       /*Set the labels text*/
    lv_obj_center(label);

    //* Create fundamental freq label
    gui_FundLabel = lv_label_create(gui_FFTScreen); /*Add a label to the button*/
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_bg_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_bg_opa(&label_style, LV_OPA_100);
    lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_text_font(&label_style, &lv_font_montserrat_14);

    lv_obj_add_style(gui_FundLabel, &label_style, 0);
    lv_obj_set_pos(gui_FundLabel, screenWidth - 100, 2);
    lv_obj_set_size(gui_FundLabel, 100, 20);
    lv_label_set_text(gui_FundLabel, "0Hz"); /*Set the labels text*/
}

void chart_slider_x_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    lv_chart_set_zoom_x((lv_obj_t *)lv_event_get_user_data(e), v);
}

void chart_slider_y_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    lv_chart_set_zoom_y((lv_obj_t *)lv_event_get_user_data(e), v);
}

void start_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data_t command;
        command.command = START_BUT_CMD;
        command.value.ull = 0;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void nerd_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        gui_show_page(NERD_PAGE);
    }
}

void fft_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        gui_show_page(FFT_PAGE);

        command_data_t command;
        command.command = FFT_REQUEST_CMD;
        command.value.ull = 1;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void filter_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data_t command;
        command.command = LPF_REQUEST_CMD;
        command.value.ull = 1;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void root_back_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        gui_show_page(MAIN_PAGE);
    }
}

void FFTXChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

        if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
        {
            return;
        }

        if (dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text)
        {
            uint16_t band = _xShared.getBandWidth();
            int16_t labels[FFT_MAJOR_TICK_COUNT];
            for (size_t i = 0; i < FFT_MAJOR_TICK_COUNT; i++)
            {
                labels[i] = i * (band / (FFT_MAJOR_TICK_COUNT - 1));
            }
            lv_snprintf(dsc->text, dsc->text_length, "%d", labels[dsc->value]);
        }
    }
}

void FFTYChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

        if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
        {
            return;
        }

        if (dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text)
        {
            uint16_t band = _xShared.getBandWidth();
            int16_t labels[FFT_MAJOR_TICK_COUNT];
            for (size_t i = 0; i < FFT_MAJOR_TICK_COUNT; i++)
            {
                labels[i] = i * (band / (FFT_MAJOR_TICK_COUNT - 1));
            }
            lv_snprintf(dsc->text, dsc->text_length, "%d", labels[dsc->value]);
        }
    }
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
        static uint8_t serCount;

        //? Show points only when chart is pressed
        int32_t id = lv_chart_get_pressed_point(chart);
        if (id == LV_CHART_POINT_NONE)
        {
            return;
        }

        lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
        serCount = 0;
        while (ser)
        {
            if (_peak_draw_done)
            {
                continue;
            }

            size_t peakCount = _xShared.getXPeakCount();
            if (peakCount == 0)
            {
                ser = lv_chart_get_series_next(chart, ser);
                serCount++;
                continue;
            }

            size_t rotDoneIndex[ACC_CHART_POINT_COUNT] = {0};
            size_t rotCount = 0;
            uint8_t *pRotDoneBuf = _xShared.getDPBRotDoneBuffer_us();

            _xShared.lockDPBDataAcc();
            for (size_t i = 0, j = 0; i < ACC_CHART_POINT_COUNT; i++)
            {
                if (pRotDoneBuf[i] == 1)
                {
                    rotDoneIndex[j++] = i;
                    rotCount++;
                }
            }
            _xShared.unlockDPBDataAcc();

            lv_point_t *p = new lv_point_t[peakCount]();
            lv_point_t *r = new lv_point_t[rotCount]();
            size_t *pPeakBuf;

            if (serCount == 1)
            {
                pPeakBuf = _xShared.getXPeaksIndexPointer_us();
            }
            else
            {
                pPeakBuf = _xShared.getYPeaksIndexPointer_us();
            }

            _xShared.lockPeaksIndex();
            for (size_t i = 0; i < peakCount; i++)
            {
                lv_chart_get_point_pos_by_id(chart, ser, pPeakBuf[i], &p[i]);
            }
            _xShared.unlockPeaksIndex();

            for (size_t i = 0; i < rotCount; i++)
            {
                lv_chart_get_point_pos_by_id(chart, ser, rotDoneIndex[i], &r[i]);
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

            for (size_t i = 0; i < rotCount; i++)
            {
                lv_draw_rect_dsc_t draw_rect_dsc;
                lv_draw_rect_dsc_init(&draw_rect_dsc);
                draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);
                draw_rect_dsc.bg_opa = LV_OPA_100;
                draw_rect_dsc.radius = 1;

                lv_area_t a;
                a.x1 = chart->coords.x1 + r[i].x - 2;
                a.x2 = chart->coords.x1 + r[i].x + 2;
                a.y1 = chart->coords.y1 + r[i].y - 2;
                a.y2 = chart->coords.y1 + r[i].y + 2;

                lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
                lv_draw_rect(draw_ctx, &draw_rect_dsc, &a);
            }

            ser = lv_chart_get_series_next(chart, ser);
            serCount++;

            delete[] p;
            delete[] r;
        }

        //_peak_draw_done = 1;
    }
    else if (code == LV_EVENT_RELEASED)
    {
        lv_obj_invalidate(chart);
    }
}

void chart_switch_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!lv_obj_has_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN) || !lv_obj_has_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN))
        {
            if (lv_obj_has_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN))
            {
                lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

                // Change icon
                lv_img_set_src(lv_obj_get_child(obj, 0), &chart_ico);
            }
            else
            {
                lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);

                // Change icon
                lv_img_set_src(lv_obj_get_child(obj, 0), &fft_ico);
            }
        }
        else
        {
            if (lv_obj_has_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN))
            {
                lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);

                // Change icon
                lv_img_set_src(lv_obj_get_child(obj, 0), &chart_ico);
            }
            else
            {
                lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
                
                // Change icon
                lv_img_set_src(lv_obj_get_child(obj, 0), &fft_ico);
            }
        }
    }
}

void list_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (lv_obj_has_flag(gui_action_list_nerd, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(gui_action_list_nerd, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(gui_action_list_nerd, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void btn_show_x_charts_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *parent = lv_obj_get_parent(obj);
        for (size_t i = 0; i < lv_obj_get_child_cnt(parent); i++)
        {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if (child == obj)
            {
                lv_obj_add_state(child, LV_STATE_CHECKED);
            }
            else
            {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
            }
        }
    }
}

void btn_show_y_charts_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *parent = lv_obj_get_parent(obj);
        for (size_t i = 0; i < lv_obj_get_child_cnt(parent); i++)
        {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if (child == obj)
            {
                lv_obj_add_state(child, LV_STATE_CHECKED);
            }
            else
            {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
            }
        }
    }
}

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
    command_data_t command;

    if (xQueueReceive(_xQueueSys2Comp, &command, (TickType_t)0) == pdPASS)
    {
        gui_exe(command);
    }
}

void gui_exe(command_data_t command)
{
    switch (command.command)
    {
    case GUI_ACC_CHART_UPDATE_CMD:
        gui_charts_update();
        break;
    case GUI_FFT_CHART_UPDATE_CMD:
        gui_fft_update();
        break;
    case GUI_UNBALANCE_UPDATE_CMD:
        gui_unbalance_arrow_update();
        break;
    case GUI_INIT_COMPLETE_CMD:
        // gui_show_page(IDLE_PAGE);
        gui_show_page(MAIN_PAGE);
        break;
    default:
        break;
    }
}

void gui_show_page(dpb_page_t page)
{
    switch (page)
    {
    case LOADING_PAGE:
        // lv_disp_load_scr(gui_LoadingScreen);
        lv_disp_load_scr(gui_MainScreen);
        break;
    case MAIN_PAGE:
        lv_disp_load_scr(gui_MainScreen);
        break;
    case NERD_PAGE:
        lv_disp_load_scr(gui_NerdScreen);
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
    _update_but_labels();
    _update_rpm();
    _update_fund();
}

void gui_charts_update(void)
{
    int16_t min_x = 0;
    int16_t max_x = 0;
    int16_t min_y = 0;
    int16_t max_y = 0;

    int16_t *pAccXBuff = nullptr;
    int16_t *pAccYBuff = nullptr;

    _xShared.lockDPBDataFltAcc();
    pAccXBuff = _xShared.getDPBDataFltAccXBuffer_us();
    pAccYBuff = _xShared.getDPBDataFltAccYBuffer_us();

    for (size_t i = 0; i < ACC_CHART_POINT_COUNT; i++)
    {
        accX_sample[i] = pAccXBuff[i];
        accY_sample[i] = pAccYBuff[i];
        if (accX_sample[i] > max_x)
        {
            max_x = accX_sample[i];
        }
        if (accX_sample[i] < min_x)
        {
            min_x = accX_sample[i];
        }
        if (accY_sample[i] > max_y)
        {
            max_y = accY_sample[i];
        }
        if (accY_sample[i] < min_y)
        {
            min_y = accY_sample[i];
        }
    }
    _xShared.unlockDPBDataFltAcc();

    lv_chart_set_range(gui_AccelXChart, LV_CHART_AXIS_PRIMARY_Y, min_x * 1.1, max_x * 1.1);
    lv_chart_refresh(gui_AccelXChart);
    lv_chart_set_range(gui_AccelYChart, LV_CHART_AXIS_PRIMARY_Y, min_y * 1.1, max_y * 1.1);
    lv_chart_refresh(gui_AccelYChart);
    _ask_peak_draw();
}

void gui_fft_update(void)
{
    int16_t min_x = 0;
    int16_t max_x = 0;
    int16_t min_y = 0;
    int16_t max_y = 0;

    float_t *pFFTXBuff = nullptr;
    float_t *pFFTYBuff = nullptr;

    _xShared.lockFFT();
    pFFTXBuff = _xShared.getFFTXBuffer_us();
    pFFTYBuff = _xShared.getFFTYBuffer_us();

    for (size_t i = 0; i < FFT_DATA_BUFFER_SIZE; i++)
    {
        fftX_sample[i] = static_cast<int16_t>(pFFTXBuff[i]);
        fftY_sample[i] = static_cast<int16_t>(pFFTYBuff[i]);
        if (fftX_sample[i] > max_x)
        {
            max_x = fftX_sample[i];
        }
        if (fftX_sample[i] < min_x)
        {
            min_x = fftX_sample[i];
        }
        if (fftY_sample[i] > max_y)
        {
            max_y = fftY_sample[i];
        }
        if (fftY_sample[i] < min_y)
        {
            min_y = fftY_sample[i];
        }
    }

    _xShared.unlockFFT();

    lv_chart_set_range(gui_FFTXChart, LV_CHART_AXIS_PRIMARY_Y, min_x, max_x);
    lv_chart_refresh(gui_FFTXChart);
    lv_chart_set_range(gui_FFTYChart, LV_CHART_AXIS_PRIMARY_Y, min_y, max_y);
    lv_chart_refresh(gui_FFTYChart);
}

void gui_unbalance_arrow_update(void)
{
    _update_unbalance();
}

void _display_init(void)
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
}

void _create_toolbars_main(void)
{
    //* Create top divider line
    lv_obj_t *top_line;
    top_line = lv_line_create(gui_MainScreen);

    // Create an array for the points of the line
    static lv_point_t top_line_points[] = {{0, DEFAULT_TOOLBAR_HEIGHT}, {100, DEFAULT_TOOLBAR_HEIGHT}, {120, 0}, {270, 0}, {290, DEFAULT_TOOLBAR_HEIGHT}, {340, DEFAULT_TOOLBAR_HEIGHT}};

    // Create style for top line
    static lv_style_t style_top_line;
    lv_style_init(&style_top_line);
    lv_style_set_line_width(&style_top_line, 1);
    lv_style_set_line_color(&style_top_line, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_style_set_line_rounded(&style_top_line, true);

    // Apply the style
    lv_line_set_points(top_line, top_line_points, 6); /*Set the points*/
    lv_obj_add_style(top_line, &style_top_line, 0);

    //* Create bottom divider line
    lv_obj_t *bottom_line;
    bottom_line = lv_line_create(gui_MainScreen);

    // Create an array for the points of the line
    static lv_point_t bottom_line_points[] = {{0, screenHeight - DEFAULT_TOOLBAR_HEIGHT}, {100, screenHeight - DEFAULT_TOOLBAR_HEIGHT}, {120, 240}};

    // Create style for top line
    static lv_style_t style_bottom_line;
    lv_style_init(&style_bottom_line);
    lv_style_set_line_width(&style_bottom_line, 1);
    lv_style_set_line_color(&style_bottom_line, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_style_set_line_rounded(&style_bottom_line, true);

    // Apply the style
    lv_line_set_points(bottom_line, bottom_line_points, 3); /*Set the points*/
    lv_obj_add_style(bottom_line, &style_bottom_line, 0);

    //* Create nerd stuff button
    gui_NerdBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_NerdBut, 25);
    lv_obj_set_height(gui_NerdBut, 25);

    // Add default styles
    static lv_style_t styleNerdBut;
    lv_style_init(&styleNerdBut);
    lv_style_set_radius(&styleNerdBut, 3);
    lv_style_set_bg_opa(&styleNerdBut, LV_OPA_TRANSP);
    lv_style_set_border_opa(&styleNerdBut, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&styleNerdBut, LV_OPA_COVER);
    lv_style_set_outline_color(&styleNerdBut, DEFAULT_ELEMENT_ACCENT_COLOR);

    // Add pressed styles
    static lv_style_t styleNerdBut_pressed;
    lv_style_init(&styleNerdBut_pressed);
    lv_style_set_outline_width(&styleNerdBut_pressed, 10);
    lv_style_set_outline_opa(&styleNerdBut_pressed, LV_OPA_TRANSP);

    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
    lv_style_set_transition(&styleNerdBut_pressed, &trans);

    lv_obj_remove_style_all(gui_NerdBut); /*Remove the style coming from the theme*/
    lv_obj_add_style(gui_NerdBut, &styleNerdBut, 0);
    lv_obj_add_style(gui_NerdBut, &styleNerdBut_pressed, LV_STATE_PRESSED);
    lv_obj_align(gui_NerdBut, LV_ALIGN_TOP_LEFT, 5, 0);

    // Add icon
    lv_obj_t *gui_NerdButImg = lv_img_create(gui_NerdBut);
    lv_img_set_src(gui_NerdButImg, &nerd_face_icon);
    lv_obj_center(gui_NerdButImg);

    // Add callback funtion
    lv_obj_add_event_cb(gui_NerdBut, nerd_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    //* Create rpm label
    gui_RPMLabel = lv_obj_create(gui_MainScreen);
    lv_obj_set_width(gui_RPMLabel, 100);
    lv_obj_set_height(gui_RPMLabel, 25);
    lv_obj_clear_flag(gui_RPMLabel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(gui_RPMLabel, LV_ALIGN_TOP_LEFT, 0, 213);

    // Add default style
    lv_obj_set_style_bg_color(gui_RPMLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_RPMLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(gui_RPMLabel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_RPMLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add icon
    lv_obj_t *gui_RPMImg = lv_img_create(gui_RPMLabel);
    lv_img_set_src(gui_RPMImg, &motor_icon);
    lv_obj_align(gui_RPMImg, LV_ALIGN_LEFT_MID, -15, 0);

    // Add text label
    gui_RPMLabelText = lv_label_create(gui_RPMLabel); /*Add a label to the button*/
    lv_obj_set_width(gui_RPMLabelText, 70);
    lv_obj_set_height(gui_RPMLabelText, 25);
    // lv_obj_set_style_text_font(gui_RPMLabelText, &gui_font_med, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style to text
    static lv_style_t rpm_text_style;
    lv_style_init(&rpm_text_style);
    lv_style_set_bg_opa(&rpm_text_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&rpm_text_style, lv_palette_main(LV_PALETTE_AMBER));
    lv_style_set_text_font(&rpm_text_style, &lv_font_montserrat_14);
    lv_style_set_text_align(&rpm_text_style, LV_ALIGN_CENTER);

    lv_obj_add_style(gui_RPMLabelText, &rpm_text_style, 0);
    lv_obj_align_to(gui_RPMLabelText, gui_RPMImg, LV_ALIGN_OUT_RIGHT_MID, 5, 5);
    lv_label_set_text(gui_RPMLabelText, "---- rpm"); /*Set the labels text*/
}

void _create_anglechart_main(void)
{
    //* Create angle chart object
    gui_UnbalanceAngleTab = lv_obj_create(gui_MainScreen);
    lv_obj_set_width(gui_UnbalanceAngleTab, 280);
    lv_obj_set_height(gui_UnbalanceAngleTab, 240);
    lv_obj_clear_flag(gui_UnbalanceAngleTab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(gui_UnbalanceAngleTab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_UnbalanceAngleTab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_UnbalanceAngleTab, LV_ALIGN_LEFT_MID, 0, 0);

    // Add background propeller image
    lv_obj_t *gui_PropBaseImg = lv_img_create(gui_UnbalanceAngleTab);
    lv_img_set_src(gui_PropBaseImg, &propeller_img_png);
    lv_img_set_angle(gui_PropBaseImg, 90 * 10);
    // lv_img_set_pivot(prop_img, 0, 0);  //To zoom from the left top corner
    float_t k = 256 * 0.8;
    lv_img_set_zoom(gui_PropBaseImg, (uint16_t)k);
    lv_obj_align(gui_PropBaseImg, LV_ALIGN_LEFT_MID, -45, 0);

    //* Create unbalance angle line
    gui_UnbalanceAngleLine = lv_line_create(gui_UnbalanceAngleTab);
    lv_line_set_y_invert(gui_UnbalanceAngleLine, true);
    static lv_style_t style_unbalance_line;
    lv_style_init(&style_unbalance_line);
    lv_style_set_line_width(&style_unbalance_line, 3);
    lv_style_set_line_color(&style_unbalance_line, SECONDARY_ELEMENT_ACCENT_COLOR);
    lv_style_set_line_rounded(&style_unbalance_line, true);
    lv_obj_add_style(gui_UnbalanceAngleLine, &style_unbalance_line, 0);
    lv_obj_align(gui_UnbalanceAngleLine, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // Create end-point circle
    gui_UnbalanceAngleLineEnd = lv_obj_create(gui_UnbalanceAngleTab);
    lv_obj_set_width(gui_UnbalanceAngleLineEnd, 10);
    lv_obj_set_height(gui_UnbalanceAngleLineEnd, 10);
    lv_obj_clear_flag(gui_UnbalanceAngleLineEnd, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(gui_UnbalanceAngleLineEnd, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_UnbalanceAngleLineEnd, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_UnbalanceAngleLineEnd, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_UnbalanceAngleLineEnd, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Create angle label
    gui_UnbalanceLabValue = lv_label_create(gui_UnbalanceAngleTab);
    lv_obj_set_width(gui_UnbalanceLabValue, 50);
    lv_obj_set_height(gui_UnbalanceLabValue, 16);
    lv_label_set_long_mode(gui_UnbalanceLabValue, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_recolor(gui_UnbalanceLabValue, true);
    lv_obj_set_style_text_align(gui_UnbalanceLabValue, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(gui_UnbalanceLabValue, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(gui_UnbalanceLabValue, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_UnbalanceLabValue, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_UnbalanceLabValue, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_UnbalanceLabValue, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_UnbalanceLabValue, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    _create_unbalance_arrow(-180.0, 0, 0);

    //* Create sensor symbol
    lv_obj_t *gui_SensorImg = lv_img_create(gui_UnbalanceAngleTab);
    lv_img_set_src(gui_SensorImg, &sensor_img);
    lv_obj_align(gui_SensorImg, LV_ALIGN_LEFT_MID, 0, 25);
}

void _create_unbalance_arrow(float_t angle_value, uint8_t lenght, uint8_t mirrored)
{
    lv_point_t startPoint = {98, 102};
    lv_point_t endPoint = startPoint;
    float_t gui_angle_value = 0;
    float_t label_angle_value = 0;

    //* Create arrow
    // Check mirror flag
    if (mirrored == 1)
    {
        gui_angle_value = angle_value + 180.0;
    }
    else
    {
        gui_angle_value = angle_value;
    }

    // Add offset cause of prop image rotation
    label_angle_value = angle_value + 180.0;
    label_angle_value = fmod(label_angle_value, 360.0);

    // Keep values positive
    if (angle_value > 360.0)
    {
        angle_value = fmod(angle_value, 360.0);
    }
    if (angle_value < 0.0)
    {
        angle_value += 360;
    }
    if (gui_angle_value > 360.0)
    {
        gui_angle_value = fmod(gui_angle_value, 360.0);
    }
    if (gui_angle_value < 0.0)
    {
        gui_angle_value += 360;
    }

    // Calc end-point coord
    endPoint.x = startPoint.x + (lv_coord_t)((float_t)lenght * cos(gui_angle_value * M_PI / 180.0));
    endPoint.y = startPoint.y + (lv_coord_t)((float_t)lenght * sin(gui_angle_value * M_PI / 180.0));

    // Apply angle change
    static lv_point_t angle_line_points[] = {startPoint, endPoint};
    lv_line_set_points(gui_UnbalanceAngleLine, angle_line_points, 2);

    // Change end-point position
    lv_obj_align(gui_UnbalanceAngleLineEnd, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 5, -endPoint.y + 5);

    // Change label value
    lv_label_set_text_fmt(gui_UnbalanceLabValue, "%.1f°", label_angle_value);

    // Change label position
    if (angle_value <= 90)
    {
        lv_obj_align(gui_UnbalanceLabValue, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 25, -endPoint.y + 16 + 8);
    }
    else if ((angle_value > 90) && (angle_value <= 180))
    {
        lv_obj_align(gui_UnbalanceLabValue, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 25, -endPoint.y + 16 + 8);
    }
    else if ((angle_value > 180) && (angle_value <= 270))
    {
        lv_obj_align(gui_UnbalanceLabValue, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 25, -endPoint.y - 8);
    }
    else
    {
        lv_obj_align(gui_UnbalanceLabValue, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 25, -endPoint.y - 8);
    }
}

void _create_pages_nerd(void)
{
    //* Acceleration-X signal chart
    gui_page_signal_x = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_signal_x, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_signal_x, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_signal_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_signal_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_signal_chart(&gui_AccelXChart, accX_sample, AccelChart_draw_event_cb, ACC_CHART_POINT_COUNT, &gui_AccelChart_Xslider, &gui_AccelChart_Yslider, gui_page_signal_x, 240, 145, {20, 25});
    lv_obj_align(gui_page_signal_x, LV_ALIGN_TOP_MID, 0, 0);

    //* Acceleration-Y signal chart
    gui_page_signal_y = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_signal_y, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_signal_y, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_signal_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_signal_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_signal_chart(&gui_AccelYChart, accY_sample, AccelChart_draw_event_cb, ACC_CHART_POINT_COUNT, &gui_AccelChart_Xslider, &gui_AccelChart_Yslider, gui_page_signal_y, 240, 145, {20, 25});

    // Hide. Only one page must be active at the same time...
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);

    //* Acceleration-Y fft chart
    gui_page_fft_x = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_fft_x, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_fft_x, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_fft_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_fft_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_analisys_chart(&gui_FFTXChart, fftX_sample, FFTXChart_draw_event_cb, FFT_DATA_BUFFER_SIZE, gui_page_fft_x, 240, 145, {20, 25});

    // Hide. Only one page must be active at the same time...
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

    //* Acceleration-Y fft chart
    gui_page_fft_y = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_fft_y, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_fft_y, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_fft_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_fft_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_analisys_chart(&gui_FFTYChart, fftY_sample, FFTYChart_draw_event_cb, FFT_DATA_BUFFER_SIZE, gui_page_fft_y, 240, 145, {20, 25});

    // Hide. Only one page must be active at the same time...
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
}

void _create_toolbars_nerd(void)
{
    static lv_obj_t *currentButton = NULL;

    //* Create bottom divider line
    lv_obj_t *bottom_line;
    bottom_line = lv_line_create(gui_NerdScreen);

    // Create an array for the points of the line
    static lv_point_t bottom_line_points[] = {{0, screenHeight},
                                              {DEFAULT_TOOLBAR_HEIGHT, screenHeight},
                                              {2 * DEFAULT_TOOLBAR_HEIGHT, screenHeight - DEFAULT_TOOLBAR_HEIGHT},
                                              {screenWidth - 2 * DEFAULT_TOOLBAR_HEIGHT, screenHeight - DEFAULT_TOOLBAR_HEIGHT},
                                              {screenWidth - DEFAULT_TOOLBAR_HEIGHT, screenHeight},
                                              {screenWidth, screenHeight}};

    // Create style for top line
    static lv_style_t style_bottom_line;
    lv_style_init(&style_bottom_line);
    lv_style_set_line_width(&style_bottom_line, 1);
    lv_style_set_line_color(&style_bottom_line, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_style_set_line_rounded(&style_bottom_line, true);

    // Apply the style
    lv_line_set_points(bottom_line, bottom_line_points, 6);
    lv_obj_add_style(bottom_line, &style_bottom_line, 0);

    //* Create top divider line
    lv_obj_t *top_line;
    top_line = lv_line_create(gui_NerdScreen);

    // Create an array for the points of the line
    static lv_point_t top_line_points[] = {{0, DEFAULT_TOOLBAR_HEIGHT},
                                           {DEFAULT_TOOLBAR_HEIGHT, DEFAULT_TOOLBAR_HEIGHT},
                                           {2 * DEFAULT_TOOLBAR_HEIGHT, 0},
                                           {screenWidth - 2 * DEFAULT_TOOLBAR_HEIGHT, 0},
                                           {screenWidth - DEFAULT_TOOLBAR_HEIGHT, DEFAULT_TOOLBAR_HEIGHT},
                                           {screenWidth, DEFAULT_TOOLBAR_HEIGHT}};

    // Create style for top line
    static lv_style_t style_top_line;
    lv_style_init(&style_top_line);
    lv_style_set_line_width(&style_top_line, 1);
    lv_style_set_line_color(&style_top_line, DEFAULT_ELEMENT_ACCENT_COLOR);
    lv_style_set_line_rounded(&style_top_line, true);

    // Apply the style
    lv_line_set_points(top_line, top_line_points, 6);
    lv_obj_add_style(top_line, &style_top_line, 0);

    //* Create bottom toolbar object
    lv_obj_t *bottom_bar;
    bottom_bar = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(bottom_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Add style
    lv_obj_set_size(bottom_bar, screenWidth - 4 * DEFAULT_TOOLBAR_HEIGHT, DEFAULT_TOOLBAR_HEIGHT - 2);
    lv_obj_set_style_bg_opa(bottom_bar, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(bottom_bar, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);

    //* Add "chart switch" button
    lv_obj_t *chart_switch_btn = lv_btn_create(bottom_bar);
    lv_obj_set_width(chart_switch_btn, 25);
    lv_obj_set_height(chart_switch_btn, 25);

    // Add style
    lv_obj_set_style_bg_opa(chart_switch_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_color(chart_switch_btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_opa(chart_switch_btn, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_side(chart_switch_btn, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_width(chart_switch_btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(chart_switch_btn, LV_ALIGN_CENTER, 0, 0);

    // Add callback
    lv_obj_add_event_cb(chart_switch_btn, chart_switch_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *chartSwitchImg = lv_img_create(chart_switch_btn);
    lv_img_set_src(chartSwitchImg, &fft_ico);
    lv_obj_center(chartSwitchImg);

    //* Create back button
    lv_obj_t *back_btn = lv_btn_create(gui_NerdScreen);
    lv_obj_set_width(back_btn, 25);
    lv_obj_set_height(back_btn, 25);

    // Add style
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -2, 2);

    // Add callback
    lv_obj_add_event_cb(back_btn, root_back_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *back_btn_label = lv_label_create(back_btn);
    lv_label_set_text(back_btn_label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(back_btn_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(back_btn_label);

    //* Create show-hide list button
    lv_obj_t *list_btn = lv_btn_create(gui_NerdScreen);
    lv_obj_set_width(list_btn, 25);
    lv_obj_set_height(list_btn, 25);

    // Add style
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(list_btn, LV_ALIGN_TOP_LEFT, 2, 2);

    // Add callback
    lv_obj_add_event_cb(list_btn, list_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *list_btn_label = lv_label_create(list_btn);
    lv_label_set_text(list_btn_label, LV_SYMBOL_LIST);
    lv_obj_set_style_text_color(list_btn_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(list_btn_label);

    //* Create action list
    gui_action_list_nerd = lv_list_create(gui_NerdScreen);
    lv_obj_set_size(gui_action_list_nerd, (2 * DEFAULT_TOOLBAR_HEIGHT) - 5, screenHeight - 2 * DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_action_list_nerd, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_action_list_nerd, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_action_list_nerd, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(gui_action_list_nerd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_action_list_nerd, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(gui_action_list_nerd, LV_OBJ_FLAG_HIDDEN);

    // Add buttons to the list
    lv_obj_t *btn;
    lv_list_add_text(gui_action_list_nerd, "Chart");
    btn = lv_list_add_btn(gui_action_list_nerd, NULL, "X-axis");
    lv_obj_add_event_cb(btn, btn_show_x_charts_event_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(gui_action_list_nerd, NULL, "Y-axis");
    lv_obj_add_event_cb(btn, btn_show_y_charts_event_cb, LV_EVENT_CLICKED, NULL);

    // Add style to the text
    currentButton = lv_obj_get_child(gui_action_list_nerd, 0);
    lv_obj_set_style_bg_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style to the X-axis button
    currentButton = lv_obj_get_child(gui_action_list_nerd, 1);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_add_state(currentButton, LV_STATE_CHECKED);

    // Add style to the X-axis button
    currentButton = lv_obj_get_child(gui_action_list_nerd, 2);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);
}

void _create_signal_chart(lv_obj_t **chart_handler, int16_t *data_array, lv_event_cb_t event_cb, size_t point_num, lv_obj_t **sliderX_handler, lv_obj_t **sliderY_handler, lv_obj_t *parent, lv_coord_t width, lv_coord_t height, lv_point_t position)
{
    *chart_handler = lv_chart_create(parent);
    lv_obj_set_width(*chart_handler, width);
    lv_obj_set_height(*chart_handler, height);
    lv_obj_set_x(*chart_handler, position.x);
    lv_obj_set_y(*chart_handler, position.y);
    lv_obj_set_style_bg_color(*chart_handler, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(*chart_handler, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(*chart_handler, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_update_mode(*chart_handler, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(*chart_handler, LV_CHART_AXIS_PRIMARY_Y, -4096, 4096);
    // lv_chart_set_zoom_x(gui_AccelXChart, 1400);                                         // Zoom in a little in X
    lv_obj_add_event_cb(*chart_handler, event_cb, LV_EVENT_ALL, NULL); //? Event to be able to draw the peak points

    lv_chart_series_t *ser = lv_chart_add_series(*chart_handler, DEFAULT_ELEMENT_ACCENT_COLOR, LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(*chart_handler, point_num);
    lv_chart_set_ext_y_array(*chart_handler, ser, (lv_coord_t *)data_array);

    lv_obj_align(*chart_handler, LV_ALIGN_TOP_MID, position.x, position.y);

    //* Create X-ScrollBar
    *sliderX_handler = lv_slider_create(parent);
    lv_slider_set_range(*sliderX_handler, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(*sliderX_handler, chart_slider_x_event_cb, LV_EVENT_VALUE_CHANGED, chart_handler);
    lv_obj_set_size(*sliderX_handler, width, 5);
    lv_obj_align_to(*sliderX_handler, *chart_handler, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    //* Create Y-ScrollBar
    *sliderY_handler = lv_slider_create(parent);
    lv_slider_set_range(*sliderY_handler, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(*sliderY_handler, chart_slider_y_event_cb, LV_EVENT_VALUE_CHANGED, chart_handler);
    lv_obj_set_size(*sliderY_handler, 5, height);
    lv_obj_align_to(*sliderY_handler, *chart_handler, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
}

void _create_analisys_chart(lv_obj_t **chart_handler, int16_t *data_array, lv_event_cb_t event_cb, size_t point_num, lv_obj_t *parent, lv_coord_t width, lv_coord_t height, lv_point_t position)
{
    *chart_handler = lv_chart_create(parent);
    lv_obj_set_width(*chart_handler, width);
    lv_obj_set_height(*chart_handler, height);
    lv_obj_set_x(*chart_handler, position.x);
    lv_obj_set_y(*chart_handler, position.y);
    lv_obj_set_style_bg_color(*chart_handler, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(*chart_handler, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(*chart_handler, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_chart_set_update_mode(*chart_handler, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(*chart_handler, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_axis_tick(*chart_handler, LV_CHART_AXIS_PRIMARY_X, 7, 3, FFT_MAJOR_TICK_COUNT, 5, true, 50);
    lv_obj_add_event_cb(*chart_handler, event_cb, LV_EVENT_ALL, NULL); //? Event to be able to draw the peak points

    lv_chart_series_t *ser = lv_chart_add_series(*chart_handler, DEFAULT_ELEMENT_ACCENT_COLOR, LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(*chart_handler, point_num);
    lv_chart_set_ext_y_array(*chart_handler, ser, (lv_coord_t *)data_array);

    lv_obj_align(*chart_handler, LV_ALIGN_TOP_MID, position.x, position.y);
}

void _chart_Y_autorange(lv_obj_t *chart_obj, lv_chart_series_t *ser)
{
    //! INCORRECT !//
    // TODO modificare la funzione utilizzando la funzione lv_chart_get_y_array
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

void _update_rpm(void)
{
    app_steps_e status = _xShared.getAppStatus();
    if (status == IDLE)
    {
        lv_label_set_text_fmt(gui_RPMLabelText, "%d rpm", 0);
    }
    else
    {
        lv_label_set_text_fmt(gui_RPMLabelText, "%d rpm", _xShared.getRPM());
    }
}

void _update_fund(void)
{
    if (_gui_act_page == FFT_PAGE)
    {
        lv_label_set_text_fmt(gui_FundLabel, "%.1fHz", _xShared.getUnbalanceFreq());
    }
}

void _update_but_labels(void)
{
    app_steps_e status = _xShared.getAppStatus();
    if (status == IDLE)
    {
        lv_label_set_text(gui_StartButLabel, LV_SYMBOL_PLAY);
    }
    else
    {
        lv_label_set_text(gui_StartButLabel, LV_SYMBOL_STOP);
    }
}

void _update_unbalance(void)
{
    _create_unbalance_arrow(_xShared.getUnbalanceXAngle(), 50, 0);
}
