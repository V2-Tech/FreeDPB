#include "GUI.h"
#include "GUI_vars.h"

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

    gui_show_page(LOADING_PAGE);

    _gui_init_done = 1;

    return ret;
}

void gui_LoadingScreen_create(void)
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

void gui_MainScreen_create(void)
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

    //* Create 4-steps chart
    _create_4stepschart_main();

    //* Create start button
    gui_StartBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_StartBut, 100);
    lv_obj_set_height(gui_StartBut, 60);
    lv_obj_align(gui_StartBut, LV_ALIGN_TOP_LEFT, 210, 75);

    // Add default styles
    lv_obj_set_style_radius(gui_StartBut, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_StartBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(gui_StartBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(gui_StartBut, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_StartBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_StartBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(gui_StartBut, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_StartBut, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_StartBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_StartBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(gui_StartBut, 15, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(gui_StartBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);

    static lv_style_transition_dsc_t but_trans;
    static lv_style_prop_t but_props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&but_trans, but_props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(gui_StartBut, &but_trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add event
    lv_obj_add_event_cb(gui_StartBut, start_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    gui_StartButLabel = lv_label_create(gui_StartBut);
    lv_obj_set_style_text_font(gui_StartButLabel, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(gui_StartButLabel, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(gui_StartButLabel);

    lv_label_set_text(gui_StartButLabel, LV_SYMBOL_PLAY);

    //* Create search type switch tab
    lv_obj_t *gui_SearchTypeSwitch_Tab = lv_obj_create(gui_MainScreen);
    lv_obj_set_size(gui_SearchTypeSwitch_Tab, 100, 30);
    lv_obj_clear_flag(gui_SearchTypeSwitch_Tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(gui_SearchTypeSwitch_Tab, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(gui_SearchTypeSwitch_Tab, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(gui_SearchTypeSwitch_Tab, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align_to(gui_SearchTypeSwitch_Tab, gui_StartBut, LV_ALIGN_OUT_TOP_MID, 0, -10);

    // Create switch
    gui_SearchTypeSwitch = lv_switch_create(gui_SearchTypeSwitch_Tab);
    lv_obj_set_style_bg_color(gui_SearchTypeSwitch, SECONDARY_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_SearchTypeSwitch, SECONDARY_BACKGROUND_COLOR, LV_PART_INDICATOR | LV_STATE_DEFAULT | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(gui_SearchTypeSwitch, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_SearchTypeSwitch, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_center(gui_SearchTypeSwitch);

    lv_obj_add_event_cb(gui_SearchTypeSwitch, searchType_sw_event_cb, LV_EVENT_ALL, NULL);

    // Add icons
    lv_obj_t *gui_SearchType = lv_img_create(gui_SearchTypeSwitch_Tab);
    lv_img_set_src(gui_SearchType, &sensor_img);
    lv_obj_align(gui_SearchType, LV_ALIGN_CENTER, -40, 17);

    gui_SearchType = lv_img_create(gui_SearchTypeSwitch_Tab);
    lv_img_set_src(gui_SearchType, &weight_icon);
    lv_obj_align(gui_SearchType, LV_ALIGN_CENTER, 40, 0);

    //* Create reset button
    gui_ResetBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_ResetBut, 100);
    lv_obj_set_height(gui_ResetBut, 42);
    lv_obj_align_to(gui_ResetBut, gui_StartBut, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Add default styles
    lv_obj_set_style_radius(gui_ResetBut, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_ResetBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(gui_ResetBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(gui_ResetBut, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_ResetBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_ResetBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(gui_ResetBut, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_ResetBut, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_ResetBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_ResetBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(gui_ResetBut, 15, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(gui_ResetBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_style_transition_dsc_init(&but_trans, but_props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(gui_ResetBut, &but_trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add event
    lv_obj_add_event_cb(gui_ResetBut, reset_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *gui_ResetButLabel = lv_label_create(gui_ResetBut);
    lv_label_set_text(gui_ResetButLabel, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(gui_ResetButLabel, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(gui_ResetButLabel, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(gui_ResetButLabel);

    //* Create angle offset spinbox
    // Create tab
    gui_OffsetSpinboxTab = lv_obj_create(gui_MainScreen);
    lv_obj_set_width(gui_OffsetSpinboxTab, 100);
    lv_obj_set_height(gui_OffsetSpinboxTab, 42);
    lv_obj_clear_flag(gui_OffsetSpinboxTab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    // Add style
    lv_obj_set_style_bg_opa(gui_OffsetSpinboxTab, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(gui_OffsetSpinboxTab, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align_to(gui_OffsetSpinboxTab, gui_ResetBut, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Create spinbox
    gui_OffsetSpinbox = lv_spinbox_create(gui_OffsetSpinboxTab);
    lv_spinbox_set_range(gui_OffsetSpinbox, -3599, 3599);
    lv_spinbox_set_digit_format(gui_OffsetSpinbox, 4, 3);
    lv_spinbox_step_prev(gui_OffsetSpinbox);
    lv_spinbox_set_rollover(gui_OffsetSpinbox, true);
    lv_obj_set_width(gui_OffsetSpinbox, 60);
    lv_obj_set_height(gui_OffsetSpinbox, lv_obj_get_height(gui_OffsetSpinboxTab));
    lv_obj_set_style_pad_hor(gui_OffsetSpinbox, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(gui_OffsetSpinbox, (lv_obj_get_height(gui_OffsetSpinbox) - 14) / 2, LV_PART_MAIN);

    // Add style
    lv_obj_set_style_bg_color(gui_OffsetSpinbox, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_color(gui_OffsetSpinbox, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(gui_OffsetSpinbox, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(gui_OffsetSpinbox, lv_obj_get_style_radius(gui_StartBut, LV_PART_MAIN), LV_PART_MAIN);
    lv_obj_align(gui_OffsetSpinbox, LV_ALIGN_CENTER, 0, 0);

    // Add callback
    lv_obj_add_event_cb(gui_OffsetSpinbox, offset_spinbox_event_cb, LV_EVENT_ALL, NULL);

    // Create increment button
    lv_coord_t h = lv_obj_get_height(gui_OffsetSpinbox);
    lv_obj_t *btn = lv_btn_create(gui_OffsetSpinboxTab);
    lv_obj_set_size(btn, 15, h);

    // Add style
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
    lv_obj_set_style_bg_color(btn, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align_to(btn, gui_OffsetSpinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // Add icon
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_set_style_text_color(label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_center(label);

    // Add callback
    lv_obj_add_event_cb(btn, offset_increment_btn_event_cb, LV_EVENT_ALL, NULL);

    // Create decrement button
    btn = lv_btn_create(gui_OffsetSpinboxTab);
    lv_obj_set_size(btn, 15, h);

    // Add style
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
    lv_obj_set_style_bg_color(btn, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(btn, gui_OffsetSpinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    // Add icon
    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_MINUS);
    lv_obj_set_style_text_color(label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);

    // Add callback
    lv_obj_add_event_cb(btn, offset_decrement_btn_event_cb, LV_EVENT_ALL, NULL);

    //* Create step back button
    gui_StepBackBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_StepBackBut, 100);
    lv_obj_set_height(gui_StepBackBut, 42);
    lv_obj_align_to(gui_StepBackBut, gui_StartBut, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Add default styles
    lv_obj_set_style_radius(gui_StepBackBut, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_StepBackBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(gui_StepBackBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(gui_StepBackBut, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_StepBackBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_StepBackBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(gui_StepBackBut, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(gui_StepBackBut, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(gui_StepBackBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(gui_StepBackBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(gui_StepBackBut, 15, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(gui_StepBackBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_style_transition_dsc_init(&but_trans, but_props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(gui_StepBackBut, &but_trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add event
    lv_obj_add_event_cb(gui_StepBackBut, step_back_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *gui_StepBackButLabel = lv_label_create(gui_StepBackBut);
    lv_label_set_text(gui_StepBackButLabel, LV_SYMBOL_PREV);
    lv_obj_set_style_text_font(gui_StepBackButLabel, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(gui_StepBackButLabel, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(gui_StepBackButLabel);

    // Hide
    lv_obj_add_flag(gui_StepBackBut, LV_OBJ_FLAG_HIDDEN);
}

void gui_NerdScreen_create(void)
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

void gui_SettingsScreen_create(void)
{
    //* Create SETTINGS SCREEN object
    gui_SettingsScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(gui_SettingsScreen, LV_OBJ_FLAG_SCROLLABLE);
    // Set screen style
    lv_obj_set_style_bg_color(gui_SettingsScreen, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);

    //* Create sub-pages
    _create_pages_settings();

    //* Create toolbars
    _create_toolbars_settings();
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

void reset_btn_event_cb(lv_event_t *e)
{
}

void settings_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        _ask_settings_values();
        gui_show_page(SETTINGS_PAGE);
        _settings_page_manager(_settings_act_page);
    }
}

void step_back_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data_t command;
        command.command = APP_STEP_CMD;
        command.value.ll = PREVIOUS;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void offset_spinbox_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        _xShared.setAngleOffset((float_t)lv_spinbox_get_value(obj) / 10);
        _update_unbalance_arrow();
    }
}

void offset_increment_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_increment(gui_OffsetSpinbox);
    }
}

void offset_decrement_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_decrement(gui_OffsetSpinbox);
    }
}

void searchType_sw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        int32_t v = (int32_t)_xShared.getSearchType();
        if (v == (int32_t)SEARCH_OPTICAL)
        {
            v = (int32_t)SEARCH_4_STEPS;
            lv_obj_add_flag(gui_UnbalanceAngleTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(gui_OffsetSpinboxTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(gui_ResetBut, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(gui_StepBackBut, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(gui_4StepsTab, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            v = (int32_t)SEARCH_OPTICAL;
            lv_obj_add_flag(gui_4StepsTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(gui_StepBackBut, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(gui_UnbalanceAngleTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(gui_OffsetSpinboxTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(gui_ResetBut, LV_OBJ_FLAG_HIDDEN);
        }
        _xShared.setSearchType((app_search_type_e)v);

        command_data_t command;
        command.command = SEARCH_TYPE_CMD;
        command.value.ll = v;

        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
    }
}

void nerd_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        gui_show_page(NERD_PAGE);
        _nerd_page_manager(_nerd_act_page);
    }
}

void root_back_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        gui_show_page(MAIN_PAGE);
        _main_page_manager();
    }
}

void FFTXChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);
    void *user_data = lv_event_get_user_data(e);

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
    if (code == LV_EVENT_DRAW_POST_END)
    {
        lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
        lv_point_t p = {0, 0};

        lv_chart_get_point_pos_by_id(chart, ser, _xShared.getFFTXMaxIndex(), &p);

        char buf[32];
        float_t fft_value = _xShared.getFFTX(_xShared.getFFTXMaxIndex());
        uint16_t sample_freq = _xShared.getBandWidth();
        size_t fft_length = FFT_DATA_BUFFER_SIZE;
        static float_t fft_res = 0.0;
        static float_t signal_fundamental = 0.0;

        fft_res = (float_t)sample_freq / (float_t)fft_length;
        signal_fundamental = (float_t)_xShared.getFFTXMaxIndex();
        signal_fundamental *= fft_res;
        lv_snprintf(buf, sizeof(buf), "%.1f dB/Hz\n%.1f Hz\n", fft_value, signal_fundamental);

        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_color = SECONDARY_ELEMENT_ACCENT_COLOR;
        draw_rect_dsc.bg_opa = LV_OPA_70;
        draw_rect_dsc.radius = 3;

        lv_area_t a;
        lv_point_t box_size;
        lv_coord_t box_pad = 10;
        lv_coord_t text_pad = 5;
        lv_txt_get_size(&box_size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if ((chart->coords.x1 + p.x + box_size.x + 10) < chart->coords.x2)
        {
            a.x1 = chart->coords.x1 + p.x + box_pad;
            a.x2 = chart->coords.x1 + p.x + box_size.x + box_pad + text_pad;
            a.y1 = chart->coords.y1 + p.y;
            a.y2 = chart->coords.y1 + p.y + box_size.y + text_pad;
        }
        else
        {
            a.x1 = chart->coords.x1 + p.x - (box_size.x + box_pad + text_pad);
            a.x2 = chart->coords.x1 + p.x - box_pad;
            a.y1 = chart->coords.y1 + p.y;
            a.y2 = chart->coords.y1 + p.y + box_size.y + text_pad;
        }

        lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
        lv_draw_rect(draw_ctx, &draw_rect_dsc, &a);

        lv_draw_label_dsc_t draw_label_dsc;
        lv_draw_label_dsc_init(&draw_label_dsc);
        draw_label_dsc.color = lv_color_white();
        a.x1 += text_pad / 2;
        a.x2 -= text_pad / 2;
        a.y1 += text_pad / 2;
        a.y2 -= text_pad / 2;
        lv_draw_label(draw_ctx, &draw_label_dsc, &a, buf, NULL);
    }
}

void FFTYChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);
    void *user_data = lv_event_get_user_data(e);

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
    if (code == LV_EVENT_DRAW_POST_END)
    {
        lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
        lv_point_t p = {0, 0};

        lv_chart_get_point_pos_by_id(chart, ser, _xShared.getFFTYMaxIndex(), &p);

        char buf[32];
        float_t fft_value = _xShared.getFFTY(_xShared.getFFTYMaxIndex());
        uint16_t sample_freq = _xShared.getBandWidth();
        size_t fft_length = FFT_DATA_BUFFER_SIZE;
        static float_t fft_res = 0.0;
        static float_t signal_fundamental = 0.0;

        fft_res = (float_t)sample_freq / (float_t)fft_length;
        signal_fundamental = (float_t)_xShared.getFFTYMaxIndex();
        signal_fundamental *= fft_res;
        lv_snprintf(buf, sizeof(buf), "%.1f dB/Hz\n%.1f Hz\n", fft_value, signal_fundamental);

        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_color = SECONDARY_ELEMENT_ACCENT_COLOR;
        draw_rect_dsc.bg_opa = LV_OPA_70;
        draw_rect_dsc.radius = 3;

        lv_area_t a;
        lv_point_t box_size;
        lv_coord_t box_pad = 10;
        lv_coord_t text_pad = 5;
        lv_txt_get_size(&box_size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if ((chart->coords.x1 + p.x + box_size.x + 10) < chart->coords.x2)
        {
            a.x1 = chart->coords.x1 + p.x + box_pad;
            a.x2 = chart->coords.x1 + p.x + box_size.x + box_pad + text_pad;
            a.y1 = chart->coords.y1 + p.y;
            a.y2 = chart->coords.y1 + p.y + box_size.y + text_pad;
        }
        else
        {
            a.x1 = chart->coords.x1 + p.x - (box_size.x + box_pad + text_pad);
            a.x2 = chart->coords.x1 + p.x - box_pad;
            a.y1 = chart->coords.y1 + p.y;
            a.y2 = chart->coords.y1 + p.y + box_size.y + text_pad;
        }

        lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
        lv_draw_rect(draw_ctx, &draw_rect_dsc, &a);

        lv_draw_label_dsc_t draw_label_dsc;
        lv_draw_label_dsc_init(&draw_label_dsc);
        draw_label_dsc.color = lv_color_white();
        a.x1 += text_pad / 2;
        a.x2 -= text_pad / 2;
        a.y1 += text_pad / 2;
        a.y2 -= text_pad / 2;
        lv_draw_label(draw_ctx, &draw_label_dsc, &a, buf, NULL);
    }
}

void AccelChart_draw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);
    void *user_data = lv_event_get_user_data(e);

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

        //! Show points only when chart is pressed
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

            size_t peakCount;
            if (user_data == gui_AccelXChart)
            {
                peakCount = _xShared.getXPeakCount();
            }
            else
            {
                peakCount = _xShared.getYPeakCount();
            }

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
            int16_t *pPeakBuf;

            if (user_data == gui_AccelXChart)
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
        switch (_nerd_act_page)
        {
        case X_RAW:
            _nerd_page_manager(X_FFT_RAW);
            break;
        case Y_RAW:
            _nerd_page_manager(Y_FFT_RAW);
            break;
        case X_FILTERED:
            _nerd_page_manager(X_FFT_FILTERED);
            break;
        case Y_FILTERED:
            _nerd_page_manager(Y_FFT_FILTERED);
            break;
        case X_FFT_RAW:
            _nerd_page_manager(X_RAW);
            break;
        case Y_FFT_RAW:
            _nerd_page_manager(Y_RAW);
            break;
        case X_FFT_FILTERED:
            _nerd_page_manager(X_FILTERED);
            break;
        case Y_FFT_FILTERED:
            _nerd_page_manager(Y_FILTERED);
            break;

        default:
            break;
        }
    }
}

void refilter_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        command_data_t command;
        command.command = APP_CMD;
        command.value.ll = SYS_ANALYZE_DATA;
        xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
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
        if (!lv_obj_has_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN) || !lv_obj_has_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN))
        {
            _nerd_page_manager(X_FFT_FILTERED);
        }
        else
        {
            _nerd_page_manager(X_FILTERED);
        }
    }
}

void btn_show_y_charts_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!lv_obj_has_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN) || !lv_obj_has_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN))
        {
            _nerd_page_manager(Y_FFT_FILTERED);
        }
        else
        {
            _nerd_page_manager(Y_FILTERED);
        }
    }
}

void btn_show_raw_x_charts_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!lv_obj_has_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN) || !lv_obj_has_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN))
        {
            _nerd_page_manager(X_FFT_RAW);
        }
        else
        {
            _nerd_page_manager(X_RAW);
        }
    }
}

void btn_show_raw_y_charts_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!lv_obj_has_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN) || !lv_obj_has_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN))
        {
            _nerd_page_manager(Y_FFT_RAW);
        }
        else
        {
            _nerd_page_manager(Y_RAW);
        }
    }
}

void btn_show_sys_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        _settings_page_manager(SYSTEM_SETTINGS);
    }
}

void btn_show_accel_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        _settings_page_manager(ACCEL_SETTINGS);
    }
}

void btn_show_filter_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        _settings_page_manager(FILTER_SETTINGS);
    }
}

void btn_show_info_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        _settings_page_manager(INFO_SETTINGS);
    }
}

void btn_save_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        _ask_settings_save();
    }
}

void btn_store_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        _ask_settings_store();
    }
}

void unbalance_source_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        gui_unbalanceSource = lv_dropdown_get_selected(obj);
    }
}

void motor_speed_slider_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    char buf[5];

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_snprintf(buf, sizeof(buf), "%d%%", (uint16_t)lv_slider_get_value(obj) / 10);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(obj), 1), buf);

        gui_measureThrottle = lv_slider_get_value(obj);
    }
}

void range_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        gui_range = lv_dropdown_get_selected(obj);
    }
}

void bandwidth_settings_event_cb(lv_event_t *e)
{
    // TODO
}

void freq_slider_settings_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[8];

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_snprintf(buf, sizeof(buf), "%dHz", (uint16_t)lv_slider_get_value(obj));
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(obj), 1), buf);

        gui_iirCenterFreq = lv_slider_get_value(obj);
    }
}

void QFactor_spinbox_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        gui_iirQFactor = lv_spinbox_get_value(obj);
    }
}

void QFactor_increment_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_increment(gui_QFactor_spinbox);
    }
}

void QFactor_decrement_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_decrement(gui_QFactor_spinbox);
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
        _exe_accel_charts_update();
        break;
    case GUI_FFT_CHART_UPDATE_CMD:
        _exe_fft_charts_update();
        break;
    case GUI_UNBALANCE_UPDATE_CMD:
        _exe_unbalance_update((gui_unbalance_command_e)command.value.ll);
        break;
    case GUI_INIT_COMPLETED_CMD:
        _exe_init_completed();
        break;
    case GUI_NERD_STATS_UPDATE_CMD:
        _update_nerd_stats();
        break;

    case APP_GET_SOURCE_CMD:
    case ACCEL_GET_BW_CMD:
    case ACCEL_GET_RANGE_CMD:
    case MOTOR_GET_SPEED_CMD:
    case IIR_GET_FREQ_CMD:
    case IIR_GET_Q_CMD:
        _exe_settings_update(command);
        break;
    default:
        break;
    }
}

void gui_show_page(dpb_page_e page)
{
    gui_clear_page(page);

    switch (page)
    {
    case LOADING_PAGE:
        _show_loading_screen();
        break;

    case MAIN_PAGE:
        _show_main_screen();
        break;

    case NERD_PAGE:
        _show_nerd_screen();
        break;

    case SETTINGS_PAGE:
        _show_settings_screen();
        break;

    default:
        break;
    }

    gui_delete_page(page);

    _gui_act_page = page;

    //    lv_mem_monitor_t mon;
    //    lv_mem_monitor(&mon);
    //    printf("used: %6lu (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
    //           mon.used_pct,
    //           mon.frag_pct,
    //           (int)mon.free_biggest_size);
}

void gui_clear_page(dpb_page_e page)
{
    if (page == _gui_act_page)
    {
        //! We do NOT wanna clear the active page...
        return;
    }

    switch (_gui_act_page)
    {
    case LOADING_PAGE:
        _page_clear(gui_LoadingScreen);
        break;
    case MAIN_PAGE:
        _page_clear(gui_MainScreen);
        break;
    case NERD_PAGE:
        _page_clear(gui_NerdScreen);
        break;
    case SETTINGS_PAGE:
        _page_clear(gui_SettingsScreen);
        break;
    default:
        break;
    }
}

void gui_delete_page(dpb_page_e page)
{
    if (page == _gui_act_page)
    {
        //! We cannot delete the active page...
        return;
    }

    switch (_gui_act_page)
    {
    case LOADING_PAGE:
        lv_obj_del(gui_LoadingScreen);
        break;
    case MAIN_PAGE:
        lv_obj_del(gui_MainScreen);
        break;
    case NERD_PAGE:
        lv_obj_del(gui_NerdScreen);
        break;
    case SETTINGS_PAGE:
        lv_obj_del(gui_SettingsScreen);
        break;
    default:
        break;
    }
}

void gui_values_update(void)
{
    if (_gui_act_page != MAIN_PAGE)
    {
        return;
    }
    _update_but_labels();
    _update_rpm();
}

void _exe_accel_charts_update(void)
{
    if (_gui_act_page != NERD_PAGE)
    {
        return;
    }

    uint8_t data_type;

    if ((_nerd_act_page == X_RAW) || (_nerd_act_page == X_FFT_RAW) || (_nerd_act_page == Y_RAW) || (_nerd_act_page == Y_FFT_RAW))
    {
        data_type = RAW_DATA;
    }
    else
    {
        data_type = FILTERED_DATA;
    }

    _update_accel_charts(data_type);
}

void _exe_fft_charts_update(void)
{
    if (_gui_act_page != NERD_PAGE)
    {
        return;
    }

    _update_fft_charts();
}

void _exe_unbalance_update(gui_unbalance_command_e step)
{
    if (_gui_act_page != MAIN_PAGE)
    {
        return;
    }
    _gui_act_step = (gui_sys_step_e)step;
    _main_page_manager();
}

void _exe_init_completed(void)
{
    gui_show_page(MAIN_PAGE);
}

void _exe_settings_update(command_data_t command)
{
    switch (command.command)
    {
    case APP_GET_SOURCE_CMD:
        lv_dropdown_set_selected(gui_unbalance_source_dropdown, (uint16_t)command.value.ull);
        gui_unbalanceSource = (int32_t)command.value.ll;
        break;

    case ACCEL_GET_RANGE_CMD:
        lv_dropdown_set_selected(gui_range_dropdown, (uint16_t)command.value.ull);
        gui_range = (int32_t)command.value.ll;
        break;

    case ACCEL_GET_BW_CMD:
        gui_bandWidth = (int32_t)command.value.ll;
        break;

    case MOTOR_GET_SPEED_CMD:
        lv_slider_set_value(gui_speed_slider, (int32_t)command.value.ll, LV_ANIM_ON);
        lv_label_set_text_fmt(gui_speed_slider_value_label, "%d%%", (uint16_t)command.value.ll / 10);
        gui_measureThrottle = (int32_t)command.value.ll;
        break;

    case IIR_GET_FREQ_CMD:
        lv_slider_set_value(gui_freq_slider, (int32_t)command.value.ll, LV_ANIM_ON);
        lv_label_set_text_fmt(gui_freq_slider_value_label, "%dHz", (uint16_t)command.value.ll);
        gui_iirCenterFreq = (int32_t)command.value.ll;
        break;

    case IIR_GET_Q_CMD:
        lv_spinbox_set_value(gui_QFactor_spinbox, (int32_t)command.value.ll);
        gui_iirQFactor = (int32_t)command.value.ll;
        break;

    default:
        break;
    }
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

void _update_unbalance_arrow(void)
{
    if (_xShared.getUnbalanceSource() == X_AXIS_SOURCE)
    {
        _create_unbalance_arrow(_xShared.getUnbalanceXAngle() + _xShared.getAngleOffset(), _xShared.getUnbalanceMag(), 70, 0);
    }
    else
    {
        _create_unbalance_arrow(_xShared.getUnbalanceYAngle() + _xShared.getAngleOffset(), _xShared.getUnbalanceMag(), 70, 0);
    }
}

void _update_accel_charts(uint8_t data_type)
{
    int16_t min_x = 0;
    int16_t max_x = 0;
    int16_t min_y = 0;
    int16_t max_y = 0;

    int16_t *pAccXBuff = nullptr;
    int16_t *pAccYBuff = nullptr;

    if (data_type == RAW_DATA)
    {
        _xShared.lockDPBDataAcc();
        pAccXBuff = _xShared.getDPBDataAccXBuffer_us();
        pAccYBuff = _xShared.getDPBDataAccYBuffer_us();
    }
    else
    {
        _xShared.lockDPBDataFltAcc();
        pAccXBuff = _xShared.getDPBDataFltAccXBuffer_us();
        pAccYBuff = _xShared.getDPBDataFltAccYBuffer_us();
    }

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

    if (data_type == RAW_DATA)
    {
        _xShared.unlockDPBDataAcc();
    }
    else
    {
        _xShared.unlockDPBDataFltAcc();
    }

    lv_chart_set_range(gui_AccelXChart, LV_CHART_AXIS_PRIMARY_Y, min_x * 1.1, max_x * 1.1);
    lv_chart_refresh(gui_AccelXChart);
    lv_chart_set_range(gui_AccelYChart, LV_CHART_AXIS_PRIMARY_Y, min_y * 1.1, max_y * 1.1);
    lv_chart_refresh(gui_AccelYChart);
    _ask_peak_draw();
}

void _update_fft_charts(void)
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

void _update_nerd_stats(void)
{
    //* Rotation counter
    lv_label_set_text_fmt(rot_cnt_label, LV_SYMBOL_REFRESH "n°: %llu", _xShared.getRotCount());

    //* Unbalance error
    lv_label_set_text_fmt(error_label, LV_SYMBOL_WARNING "+-%.1f°", _xShared.getUnbalanceErr());
}

void _ask_settings_values(void)
{
    command_data_t command;

    //* Unbalance source
    command.command = APP_GET_SOURCE_CMD;
    command.value.ll = -1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Accelerometer range
    command.command = ACCEL_GET_RANGE_CMD;
    command.value.ll = -1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Accelerometer bandwidth
    command.command = ACCEL_GET_BW_CMD;
    command.value.ll = -1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Motor speed
    command.command = MOTOR_GET_SPEED_CMD;
    command.value.ll = -1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Filter center frequency
    command.command = IIR_GET_FREQ_CMD;
    command.value.ll = -1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Filter Q factor
    command.command = IIR_GET_Q_CMD;
    command.value.ll = -1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
}

void _ask_settings_save(void)
{
    command_data_t command;

    //* Unbalance source
    command.command = APP_SET_SOURCE_CMD;
    command.value.ll = (int64_t)gui_unbalanceSource;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Accelerometer range
    command.command = ACCEL_SET_RANGE_CMD;
    command.value.ll = (int64_t)gui_range;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Accelerometer bandwidth
    command.command = ACCEL_SET_BW_CMD;
    command.value.ll = (int64_t)gui_bandWidth;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Motor speed
    command.command = MOTOR_SET_SPEED_CMD;
    command.value.ll = (int64_t)gui_measureThrottle;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Filter center frequency
    command.command = IIR_SET_FREQ_CMD;
    command.value.ll = (int64_t)gui_iirCenterFreq;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    //* Filter Q factor
    command.command = IIR_SET_Q_CMD;
    command.value.ll = (int64_t)gui_iirQFactor;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
}

void _ask_settings_store(void)
{
    command_data_t command;

    command.command = STORE_SETTINGS_CMD;
    command.value.ll = 1;
    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);
}

void _show_loading_screen(void)
{
    gui_LoadingScreen_create();
    lv_disp_load_scr(gui_LoadingScreen);
}

void _show_main_screen(void)
{
    gui_MainScreen_create();
    lv_disp_load_scr(gui_MainScreen);
}

void _show_nerd_screen(void)
{
    gui_NerdScreen_create();
    lv_disp_load_scr(gui_NerdScreen);
}

void _show_settings_screen(void)
{
    gui_SettingsScreen_create();
    lv_disp_load_scr(gui_SettingsScreen);
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

void _page_clear(lv_obj_t *page)
{
    //_all_styles_remove(page); //! Not necessary anymore for LVGL Ver > 7.1.1
    lv_obj_clean(page);
}

void _all_styles_remove(lv_obj_t *obj)
{
    if (obj == NULL)
    {
        return;
    }

    size_t child_cnt = lv_obj_get_child_cnt(obj);
    for (size_t i = 0; i < child_cnt; i++)
    {
        lv_obj_remove_style_all(lv_obj_get_child(obj, i));
        _all_styles_remove(lv_obj_get_child(obj, i));
    }
}

void _main_page_manager(void)
{
    switch (_gui_act_step)
    {
    case gui_sys_step_e::GUI_SYS_STEP_NONE:
        _update_unbalance_arrow();
        lv_spinbox_set_value(gui_OffsetSpinbox, (int32_t)(_xShared.getAngleOffset() * 10.0));
        break;
    case gui_sys_step_e::GUI_SYS_STEP_1:
        break;
    case gui_sys_step_e::GUI_SYS_STEP_2:
        break;
    case gui_sys_step_e::GUI_SYS_STEP_3:
        break;
    case gui_sys_step_e::GUI_SYS_STEP_4:
        break;
    default:
        break;
    }
}

void _create_toolbars_main(void)
{
    //* Create top divider line
    lv_obj_t *top_line = lv_line_create(gui_MainScreen);

    // Create an array for the points of the line
    static lv_point_t top_line_points[] = {{0, DEFAULT_TOOLBAR_HEIGHT}, {100, DEFAULT_TOOLBAR_HEIGHT}, {120, 0}, {270, 0}, {290, DEFAULT_TOOLBAR_HEIGHT}, {340, DEFAULT_TOOLBAR_HEIGHT}};

    // Create style for top line
    lv_obj_set_style_line_width(top_line, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(top_line, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(top_line, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set the shape
    lv_line_set_points(top_line, top_line_points, 6);

    //* Create bottom divider line
    lv_obj_t *bottom_line = lv_line_create(gui_MainScreen);

    // Create an array for the points of the line
    static lv_point_t bottom_line_points[] = {{0, screenHeight - DEFAULT_TOOLBAR_HEIGHT}, {100, screenHeight - DEFAULT_TOOLBAR_HEIGHT}, {120, 240}};

    // Create style for top line
    lv_obj_set_style_line_width(bottom_line, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(bottom_line, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(bottom_line, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set the shape
    lv_line_set_points(bottom_line, bottom_line_points, 3);

    //* Create settings button
    gui_SettingsBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_SettingsBut, 25);
    lv_obj_set_height(gui_SettingsBut, 25);
    lv_obj_align(gui_SettingsBut, LV_ALIGN_TOP_LEFT, 290, 2);

    // Add default styles
    lv_obj_set_style_radius(gui_SettingsBut, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_SettingsBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_SettingsBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_SettingsBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(gui_SettingsBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(gui_SettingsBut, 10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(gui_SettingsBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(gui_SettingsBut, &trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add icon
    lv_obj_t *gui_SettingsButImg = lv_img_create(gui_SettingsBut);
    lv_img_set_src(gui_SettingsButImg, &settings_icon);
    lv_obj_center(gui_SettingsButImg);

    // Add callback funtion
    lv_obj_add_event_cb(gui_SettingsBut, settings_btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    //* Create nerd stuff button
    gui_NerdBut = lv_btn_create(gui_MainScreen);
    lv_obj_set_width(gui_NerdBut, 25);
    lv_obj_set_height(gui_NerdBut, 25);
    lv_obj_align(gui_NerdBut, LV_ALIGN_TOP_LEFT, 5, 2);

    // Add default styles
    lv_obj_set_style_radius(gui_NerdBut, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_NerdBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_NerdBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_NerdBut, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(gui_NerdBut, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(gui_NerdBut, 10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(gui_NerdBut, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(gui_NerdBut, &trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add icon
    lv_obj_t *gui_NerdButImg = lv_img_create(gui_NerdBut);
    lv_img_set_src(gui_NerdButImg, &nerd_face_icon);
    lv_obj_center(gui_NerdButImg);

    // Add callback funtion
    lv_obj_add_event_cb(gui_NerdBut, nerd_btn_event_cb, LV_EVENT_ALL, NULL);

    //* Create rpm label
    lv_obj_t *gui_RPMLabel = lv_obj_create(gui_MainScreen);
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
    lv_obj_align_to(gui_RPMLabelText, gui_RPMImg, LV_ALIGN_OUT_RIGHT_MID, 5, 5);

    // Add style to text
    lv_obj_set_style_bg_opa(gui_RPMLabelText, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(gui_RPMLabelText, lv_palette_main(LV_PALETTE_AMBER), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(gui_RPMLabelText, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(gui_RPMLabelText, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set the labels text
    lv_label_set_text(gui_RPMLabelText, "---- rpm");
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

    //* Create sensor symbol
    lv_obj_t *gui_SensorImg = lv_img_create(gui_UnbalanceAngleTab);
    lv_img_set_src(gui_SensorImg, &sensor_img);
    lv_obj_align(gui_SensorImg, LV_ALIGN_LEFT_MID, 0, 25);

    //* Create unbalance angle line
    gui_UnbalanceAngleLine = lv_line_create(gui_UnbalanceAngleTab);
    lv_line_set_y_invert(gui_UnbalanceAngleLine, true);
    lv_obj_align(gui_UnbalanceAngleLine, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // Add style
    lv_obj_set_style_line_width(gui_UnbalanceAngleLine, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(gui_UnbalanceAngleLine, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(gui_UnbalanceAngleLine, true, LV_PART_MAIN | LV_STATE_DEFAULT);

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
    lv_obj_set_height(gui_UnbalanceLabValue, 34);
    lv_label_set_long_mode(gui_UnbalanceLabValue, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_recolor(gui_UnbalanceLabValue, true);
    lv_obj_set_style_text_align(gui_UnbalanceLabValue, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(gui_UnbalanceLabValue, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(gui_UnbalanceLabValue, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_UnbalanceLabValue, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_UnbalanceLabValue, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_UnbalanceLabValue, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_UnbalanceLabValue, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    _create_unbalance_arrow(-180.0, 0, 0, 0);
}

void _create_unbalance_arrow(float_t angle_value, float_t magnitude_value, uint8_t lenght, uint8_t mirrored)
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
    if (label_angle_value > 360.0)
    {
        label_angle_value = fmod(label_angle_value, 360.0);
    }
    if (label_angle_value < 0.0)
    {
        label_angle_value += 360;
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
    lv_label_set_text_fmt(gui_UnbalanceLabValue, "%.1f°\n%.1f", label_angle_value, magnitude_value);

    // Change label position
    if (angle_value <= 90)
    {
        lv_obj_align(gui_UnbalanceLabValue, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 25, -endPoint.y + 2 * 16 + 8);
    }
    else if ((angle_value > 90) && (angle_value <= 180))
    {
        lv_obj_align(gui_UnbalanceLabValue, LV_ALIGN_BOTTOM_LEFT, endPoint.x - 25, -endPoint.y + 2 * 16 + 8);
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

void _create_4stepschart_main(void)
{
    //* Create 4-steps chart object
    gui_4StepsTab = lv_obj_create(gui_MainScreen);
    lv_obj_set_width(gui_4StepsTab, 280);
    lv_obj_set_height(gui_4StepsTab, 240);
    lv_obj_clear_flag(gui_4StepsTab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    // Add style
    lv_obj_set_style_bg_opa(gui_4StepsTab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_4StepsTab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_4StepsTab, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(gui_4StepsTab, LV_OBJ_FLAG_HIDDEN);

    //* Add background propeller image
    lv_obj_t *gui_PropBaseImg = lv_img_create(gui_4StepsTab);
    lv_img_set_src(gui_PropBaseImg, &propeller_img_png);
    lv_img_set_angle(gui_PropBaseImg, 90 * 10);
    // lv_img_set_pivot(prop_img, 0, 0);  //To zoom from the left top corner
    float_t k = 256 * 0.8;
    lv_img_set_zoom(gui_PropBaseImg, (uint16_t)k);
    lv_obj_align(gui_PropBaseImg, LV_ALIGN_LEFT_MID, -45, 0);

    //* Create 4-steps labels
    lv_point_t centerPoint = {98, 102};

    // Step 1
    gui_Steps1Lab = lv_label_create(gui_4StepsTab);
    lv_obj_set_width(gui_Steps1Lab, 50);
    lv_obj_set_height(gui_Steps1Lab, 34);
    lv_obj_set_pos(gui_Steps1Lab, centerPoint.x - 25, centerPoint.y - 15);
    lv_label_set_long_mode(gui_Steps1Lab, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_recolor(gui_Steps1Lab, true);
    lv_obj_set_style_text_align(gui_Steps1Lab, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(gui_Steps1Lab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(gui_Steps1Lab, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_Steps1Lab, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_Steps1Lab, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_Steps1Lab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_Steps1Lab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add text
    lv_label_set_text(gui_Steps1Lab, "Step 1:\nWeight-free run");

    // Step 2
    gui_Steps2Lab = lv_label_create(gui_4StepsTab);
    lv_obj_set_width(gui_Steps2Lab, 50);
    lv_obj_set_height(gui_Steps2Lab, 34);
    lv_obj_set_pos(gui_Steps2Lab, centerPoint.x - 25 - 60, centerPoint.y - 15);
    lv_label_set_long_mode(gui_Steps2Lab, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_recolor(gui_Steps2Lab, true);
    lv_obj_set_style_text_align(gui_Steps2Lab, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(gui_Steps2Lab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(gui_Steps2Lab, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_Steps2Lab, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_Steps2Lab, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_Steps2Lab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_Steps2Lab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(gui_Steps2Lab, LV_OBJ_FLAG_HIDDEN);

    // Add text
    lv_label_set_text(gui_Steps2Lab, "Step 2:\nPut 1unit weight here");

    // Step 3
    gui_Steps3Lab = lv_label_create(gui_4StepsTab);
    lv_obj_set_width(gui_Steps3Lab, 50);
    lv_obj_set_height(gui_Steps3Lab, 34);
    lv_obj_set_pos(gui_Steps3Lab, centerPoint.x - 8 + 25, centerPoint.y + 50);
    lv_label_set_long_mode(gui_Steps3Lab, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_recolor(gui_Steps3Lab, true);
    lv_obj_set_style_text_align(gui_Steps3Lab, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(gui_Steps3Lab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(gui_Steps3Lab, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_Steps3Lab, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_Steps3Lab, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_Steps3Lab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_Steps3Lab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(gui_Steps3Lab, LV_OBJ_FLAG_HIDDEN);

    // Add text
    lv_label_set_text(gui_Steps3Lab, "Step 3:\nPut 1unit weight here");

    // Step 4
    gui_Steps4Lab = lv_label_create(gui_4StepsTab);
    lv_obj_set_width(gui_Steps4Lab, 50);
    lv_obj_set_height(gui_Steps4Lab, 34);
    lv_obj_set_pos(gui_Steps4Lab, centerPoint.x - 8 + 25, centerPoint.y - (17 + 50));
    lv_label_set_long_mode(gui_Steps4Lab, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_recolor(gui_Steps4Lab, true);
    lv_obj_set_style_text_align(gui_Steps4Lab, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(gui_Steps4Lab, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(gui_Steps4Lab, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_Steps4Lab, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(gui_Steps4Lab, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_Steps4Lab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(gui_Steps4Lab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(gui_Steps4Lab, LV_OBJ_FLAG_HIDDEN);

    // Add text
    lv_label_set_text(gui_Steps4Lab, "Step 4:\nPut 1unit weight here");
}

void _create_pages_nerd(void)
{
    //* Acceleration-X signal chart
    gui_page_signal_x = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_signal_x, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_signal_x, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_signal_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_signal_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_signal_chart(&gui_AccelXChart, accX_sample, AccelChart_draw_event_cb, ACC_CHART_POINT_COUNT, &gui_AccelChart_Xslider, &gui_AccelChart_Yslider, gui_page_signal_x, 245, 145, {40, 25});
    lv_obj_align(gui_page_signal_x, LV_ALIGN_TOP_MID, 0, 0);

    //* Acceleration-Y signal chart
    gui_page_signal_y = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_signal_y, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_signal_y, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_signal_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_signal_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_signal_chart(&gui_AccelYChart, accY_sample, AccelChart_draw_event_cb, ACC_CHART_POINT_COUNT, &gui_AccelChart_Xslider, &gui_AccelChart_Yslider, gui_page_signal_y, 245, 145, {40, 25});

    // Hide. Only one page must be active at the same time...
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);

    //* Acceleration-Y fft chart
    gui_page_fft_x = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_fft_x, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_fft_x, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_fft_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_fft_x, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_fft_chart(&gui_FFTXChart, fftX_sample, FFTXChart_draw_event_cb, FFT_DATA_BUFFER_SIZE, gui_page_fft_x, 245, 145, {40, 25});

    // Hide. Only one page must be active at the same time...
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

    //* Acceleration-Y fft chart
    gui_page_fft_y = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(gui_page_fft_y, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_fft_y, screenWidth, screenHeight - DEFAULT_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(gui_page_fft_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_fft_y, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    _create_fft_chart(&gui_FFTYChart, fftY_sample, FFTYChart_draw_event_cb, FFT_DATA_BUFFER_SIZE, gui_page_fft_y, 245, 145, {40, 25});

    // Hide. Only one page must be active at the same time...
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
}

void _create_toolbars_nerd(void)
{
    lv_obj_t *currentButton = NULL;

    //* Create bottom divider line
    lv_obj_t *bottom_line = lv_line_create(gui_NerdScreen);

    // Create an array for the points of the line
    static lv_point_t bottom_line_points[] = {{0, screenHeight},
                                              {DEFAULT_TOOLBAR_HEIGHT, screenHeight},
                                              {2 * DEFAULT_TOOLBAR_HEIGHT, screenHeight - DEFAULT_TOOLBAR_HEIGHT},
                                              {screenWidth - 2 * DEFAULT_TOOLBAR_HEIGHT, screenHeight - DEFAULT_TOOLBAR_HEIGHT},
                                              {screenWidth - DEFAULT_TOOLBAR_HEIGHT, screenHeight},
                                              {screenWidth, screenHeight}};

    // Create style for top line
    lv_obj_set_style_line_width(bottom_line, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(bottom_line, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(bottom_line, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Apply the shape
    lv_line_set_points(bottom_line, bottom_line_points, 6);

    //* Create top divider line
    lv_obj_t *top_line = lv_line_create(gui_NerdScreen);

    // Create an array for the points of the line
    static lv_point_t top_line_points[] = {{0, DEFAULT_TOOLBAR_HEIGHT},
                                           {DEFAULT_TOOLBAR_HEIGHT, DEFAULT_TOOLBAR_HEIGHT},
                                           {2 * DEFAULT_TOOLBAR_HEIGHT, 0},
                                           {screenWidth - 2 * DEFAULT_TOOLBAR_HEIGHT, 0},
                                           {screenWidth - DEFAULT_TOOLBAR_HEIGHT, DEFAULT_TOOLBAR_HEIGHT},
                                           {screenWidth, DEFAULT_TOOLBAR_HEIGHT}};

    // Create style for top line
    lv_obj_set_style_line_width(top_line, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(top_line, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(top_line, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Apply the shape
    lv_line_set_points(top_line, top_line_points, 6);

    //* Create bottom toolbar object
    lv_obj_t *bottom_bar;
    bottom_bar = lv_obj_create(gui_NerdScreen);
    lv_obj_clear_flag(bottom_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(bottom_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_bar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(bottom_bar, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(bottom_bar, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(bottom_bar, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(bottom_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(bottom_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style
    lv_obj_set_size(bottom_bar, screenWidth - 4 * DEFAULT_TOOLBAR_HEIGHT, DEFAULT_TOOLBAR_HEIGHT - 2);
    lv_obj_set_style_bg_opa(bottom_bar, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(bottom_bar, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);

    //* Add "chart switch" button
    lv_obj_t *chart_switch_btn = lv_btn_create(bottom_bar);
    lv_obj_set_width(chart_switch_btn, 25);
    lv_obj_set_height(chart_switch_btn, 25);

    // Add default styles
    lv_obj_set_style_radius(chart_switch_btn, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(chart_switch_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(chart_switch_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(chart_switch_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(chart_switch_btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(chart_switch_btn, 10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(chart_switch_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(chart_switch_btn, &trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add callback
    lv_obj_add_event_cb(chart_switch_btn, chart_switch_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *chartSwitchImg = lv_img_create(chart_switch_btn);
    lv_img_set_src(chartSwitchImg, &chart_change_icon);
    lv_obj_center(chartSwitchImg);

    //* Add "re-filter" button
    lv_obj_t *refilter_btn = lv_btn_create(bottom_bar);
    lv_obj_set_width(refilter_btn, 25);
    lv_obj_set_height(refilter_btn, 25);

    // Add default styles
    lv_obj_set_style_radius(refilter_btn, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(refilter_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(refilter_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(refilter_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(refilter_btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add pressed styles
    lv_obj_set_style_outline_width(refilter_btn, 10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(refilter_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);
    lv_obj_set_style_transition(refilter_btn, &trans, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add callback
    lv_obj_add_event_cb(refilter_btn, refilter_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *refilter_btn_label = lv_label_create(refilter_btn);
    lv_label_set_text(refilter_btn_label, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_color(refilter_btn_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(refilter_btn_label);

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
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(list_btn, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(list_btn, LV_ALIGN_TOP_LEFT, 2, 2);

    // Add callback
    lv_obj_add_event_cb(list_btn, list_btn_event_cb, LV_EVENT_ALL, NULL);

    // Add icon
    lv_obj_t *list_btn_label = lv_label_create(list_btn);
    lv_label_set_text(list_btn_label, LV_SYMBOL_LIST);
    lv_obj_set_style_text_color(list_btn_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(list_btn_label);

    //* Create rotation count label
    rot_cnt_label = lv_label_create(gui_NerdScreen);
    lv_obj_set_width(rot_cnt_label, 60);
    lv_obj_set_height(rot_cnt_label, 25);
    lv_label_set_text(rot_cnt_label, LV_SYMBOL_REFRESH " n°: 0");
    lv_obj_set_style_text_color(rot_cnt_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(rot_cnt_label, LV_ALIGN_TOP_LEFT, (2 * DEFAULT_TOOLBAR_HEIGHT) + 5, 5);

    //* Create error label
    error_label = lv_label_create(gui_NerdScreen);
    lv_obj_set_width(error_label, 60);
    lv_obj_set_height(error_label, 25);
    lv_label_set_text(error_label, LV_SYMBOL_WARNING "+-0°");
    lv_obj_set_style_text_color(error_label, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(error_label, rot_cnt_label, LV_ALIGN_OUT_RIGHT_TOP, 5, 5);

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
    lv_list_add_text(gui_action_list_nerd, "Raw");
    btn = lv_list_add_btn(gui_action_list_nerd, NULL, "X-axis");
    lv_obj_add_event_cb(btn, btn_show_raw_x_charts_event_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(gui_action_list_nerd, NULL, "Y-axis");
    lv_obj_add_event_cb(btn, btn_show_raw_y_charts_event_cb, LV_EVENT_CLICKED, NULL);
    lv_list_add_text(gui_action_list_nerd, "Filt");
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

    // Add style to the Y-axis button
    currentButton = lv_obj_get_child(gui_action_list_nerd, 2);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    // Add style to the text
    currentButton = lv_obj_get_child(gui_action_list_nerd, 3);
    lv_obj_set_style_bg_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style to the X-axis button
    currentButton = lv_obj_get_child(gui_action_list_nerd, 4);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    // Add style to the Y-axis button
    currentButton = lv_obj_get_child(gui_action_list_nerd, 5);
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
    lv_obj_set_style_size(*chart_handler, 3, LV_PART_INDICATOR); // 0 = Do not display points on the data
    lv_obj_set_style_size(*chart_handler, 1, LV_PART_ITEMS);
    lv_chart_set_update_mode(*chart_handler, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(*chart_handler, LV_CHART_AXIS_PRIMARY_Y, 0, 1);
    lv_chart_set_axis_tick(*chart_handler, LV_CHART_AXIS_PRIMARY_Y, 6, 3, 5, 5, true, 50);
    lv_obj_add_event_cb(*chart_handler, event_cb, LV_EVENT_ALL, *chart_handler); //? Event to be able to draw the peak points

    lv_chart_series_t *ser = lv_chart_add_series(*chart_handler, DEFAULT_ELEMENT_ACCENT_COLOR, LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(*chart_handler, point_num);
    lv_chart_set_ext_y_array(*chart_handler, ser, (lv_coord_t *)data_array);

    lv_obj_align(*chart_handler, LV_ALIGN_TOP_LEFT, position.x, position.y);

    //* Create X-ScrollBar
    *sliderX_handler = lv_slider_create(parent);
    lv_slider_set_range(*sliderX_handler, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(*sliderX_handler, chart_slider_x_event_cb, LV_EVENT_VALUE_CHANGED, *chart_handler);
    lv_obj_set_size(*sliderX_handler, width, 5);
    lv_obj_align_to(*sliderX_handler, *chart_handler, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    //* Create Y-ScrollBar
    *sliderY_handler = lv_slider_create(parent);
    lv_slider_set_range(*sliderY_handler, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    lv_obj_add_event_cb(*sliderY_handler, chart_slider_y_event_cb, LV_EVENT_VALUE_CHANGED, *chart_handler);
    lv_obj_set_size(*sliderY_handler, 5, height);
    lv_obj_align_to(*sliderY_handler, *chart_handler, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
}

void _create_fft_chart(lv_obj_t **chart_handler, int16_t *data_array, lv_event_cb_t event_cb, size_t point_num, lv_obj_t *parent, lv_coord_t width, lv_coord_t height, lv_point_t position)
{
    *chart_handler = lv_chart_create(parent);
    lv_obj_set_width(*chart_handler, width);
    lv_obj_set_height(*chart_handler, height);
    lv_obj_set_x(*chart_handler, position.x);
    lv_obj_set_y(*chart_handler, position.y);
    lv_obj_set_style_bg_color(*chart_handler, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(*chart_handler, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(*chart_handler, 0, LV_PART_INDICATOR); // Do not display points on the data
    lv_obj_set_style_size(*chart_handler, 1, LV_PART_ITEMS);
    lv_chart_set_update_mode(*chart_handler, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(*chart_handler, LV_CHART_AXIS_PRIMARY_Y, 0, 1);
    lv_chart_set_axis_tick(*chart_handler, LV_CHART_AXIS_PRIMARY_X, 7, 3, FFT_MAJOR_TICK_COUNT, 5, true, 50);
    lv_obj_add_event_cb(*chart_handler, event_cb, LV_EVENT_ALL, *chart_handler); //? Event to be able to draw the peak points

    lv_chart_series_t *ser = lv_chart_add_series(*chart_handler, SECONDARY_ELEMENT_ACCENT_COLOR, LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(*chart_handler, point_num);
    lv_chart_set_ext_y_array(*chart_handler, ser, (lv_coord_t *)data_array);

    lv_obj_align(*chart_handler, LV_ALIGN_TOP_LEFT, position.x, position.y);
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

void _set_nerd_page(nerd_subpage_e page)
{
    _nerd_act_page = page;
}

void _nerd_page_manager(nerd_subpage_e page)
{
    switch (page)
    {
    case X_RAW:
        _nerd_show_x_raw();
        break;
    case Y_RAW:
        _nerd_show_y_raw();
        break;
    case X_FILTERED:
        _nerd_show_x_filtered();
        break;
    case Y_FILTERED:
        _nerd_show_y_filtered();
        break;
    case X_FFT_RAW:
        _nerd_show_x_fft_raw();
        break;
    case Y_FFT_RAW:
        _nerd_show_y_fft_raw();
        break;
    case X_FFT_FILTERED:
        _nerd_show_x_fft_filtered();
        break;
    case Y_FFT_FILTERED:
        _nerd_show_y_fft_filtered();
        break;

    default:
        break;
    }

    _set_nerd_page(page);

    for (size_t i = 0; i < lv_obj_get_child_cnt(gui_action_list_nerd); i++)
    {
        lv_obj_t *child = lv_obj_get_child(gui_action_list_nerd, i);
        if ((i == page) || (i == page >> 4))
        {
            lv_obj_add_state(child, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(child, LV_STATE_CHECKED);
        }
    }
}

void _nerd_show_x_raw(void)
{
    _update_accel_charts(RAW_DATA);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_y_raw(void)
{
    _update_accel_charts(RAW_DATA);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_x_filtered(void)
{
    _update_accel_charts(FILTERED_DATA);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_y_filtered(void)
{
    // Request chart values update
    _update_accel_charts(FILTERED_DATA);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_x_fft_raw(void)
{
    // Request chart values update
    command_data_t command;
    command.command = FFT_REQUEST_CMD;
    command.value.ull = RAW_DATA;

    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_y_fft_raw(void)
{
    // Request chart values update
    command_data_t command;
    command.command = FFT_REQUEST_CMD;
    command.value.ull = RAW_DATA;

    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_x_fft_filtered(void)
{
    // Request chart values update
    command_data_t command;
    command.command = FFT_REQUEST_CMD;
    command.value.ull = FILTERED_DATA;

    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);
}

void _nerd_show_y_fft_filtered(void)
{
    // Request chart values update
    command_data_t command;
    command.command = FFT_REQUEST_CMD;
    command.value.ull = FILTERED_DATA;

    xQueueSend(_xQueueCom2Sys, &command, portMAX_DELAY);

    // Change visibility of pages
    lv_obj_add_flag(gui_page_signal_y, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_signal_x, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_fft_x, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_fft_y, LV_OBJ_FLAG_HIDDEN);
}

void _create_toolbars_settings(void)
{
    //* Create top divider line
    lv_obj_t *top_line;
    top_line = lv_line_create(gui_SettingsScreen);

    // Create an array for the points of the line
    static lv_point_t top_line_points[] = {{(5 * DEFAULT_TOOLBAR_HEIGHT) / 2, screenHeight},
                                           {(5 * DEFAULT_TOOLBAR_HEIGHT) / 2, 0},
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
    lv_line_set_points(top_line, top_line_points, 5);
    lv_obj_add_style(top_line, &style_top_line, 0);

    //* Create back button
    lv_obj_t *back_btn = lv_btn_create(gui_SettingsScreen);
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

    //* Create menu list
    gui_menu_list_settings = lv_list_create(gui_SettingsScreen);
    lv_obj_set_size(gui_menu_list_settings, (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, screenHeight);
    lv_obj_set_style_bg_opa(gui_menu_list_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(gui_menu_list_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_menu_list_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(gui_menu_list_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_menu_list_settings, LV_ALIGN_LEFT_MID, 0, 0);

    // Add buttons to the list
    lv_obj_t *btn;
    lv_list_add_text(gui_menu_list_settings, "Settings");
    btn = lv_list_add_btn(gui_menu_list_settings, LV_SYMBOL_SETTINGS, "Sys");
    lv_obj_add_event_cb(btn, btn_show_sys_settings_event_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(gui_menu_list_settings, LV_SYMBOL_SHUFFLE, "Accel");
    lv_obj_add_event_cb(btn, btn_show_accel_settings_event_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(gui_menu_list_settings, LV_SYMBOL_EDIT, "Filter");
    lv_obj_add_event_cb(btn, btn_show_filter_settings_event_cb, LV_EVENT_CLICKED, NULL);
    lv_list_add_text(gui_menu_list_settings, "Other");
    btn = lv_list_add_btn(gui_menu_list_settings, LV_SYMBOL_EYE_OPEN, "Info");
    lv_obj_add_event_cb(btn, btn_show_info_settings_event_cb, LV_EVENT_CLICKED, NULL);
    lv_list_add_text(gui_menu_list_settings, "Actions");
    btn = lv_list_add_btn(gui_menu_list_settings, LV_SYMBOL_OK, "Apply");
    lv_obj_add_event_cb(btn, btn_save_settings_event_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(gui_menu_list_settings, LV_SYMBOL_DOWNLOAD, "Store");
    lv_obj_add_event_cb(btn, btn_store_settings_event_cb, LV_EVENT_CLICKED, NULL);

    // Add style to the text
    lv_obj_t *currentButton = lv_obj_get_child(gui_menu_list_settings, 0);
    lv_obj_set_style_bg_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style to the System button
    currentButton = lv_obj_get_child(gui_menu_list_settings, 1);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_add_state(currentButton, LV_STATE_CHECKED);

    // Add style to the Accelerometer button
    currentButton = lv_obj_get_child(gui_menu_list_settings, 2);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    // Add style to the Filter button
    currentButton = lv_obj_get_child(gui_menu_list_settings, 3);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    // Add style to the text
    currentButton = lv_obj_get_child(gui_menu_list_settings, 4);
    lv_obj_set_style_bg_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style to the Info button
    currentButton = lv_obj_get_child(gui_menu_list_settings, 5);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);

    // Add style to the text
    currentButton = lv_obj_get_child(gui_menu_list_settings, 6);
    lv_obj_set_style_bg_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add style to the Save button
    currentButton = lv_obj_get_child(gui_menu_list_settings, 7);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);

    // Add style to the Store button
    currentButton = lv_obj_get_child(gui_menu_list_settings, 8);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(currentButton, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(currentButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(currentButton, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);

    lv_obj_set_style_bg_color(currentButton, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(currentButton, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_PRESSED);
}

void _create_pages_settings(void)
{
    //* PAGE - System
    gui_page_system_settings = lv_obj_create(gui_SettingsScreen);
    lv_obj_set_flex_flow(gui_page_system_settings, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_set_flex_align(gui_page_system_settings, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(gui_page_system_settings, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_system_settings, screenWidth - (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, screenHeight);
    lv_obj_set_style_bg_opa(gui_page_system_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_system_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(gui_page_system_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(gui_page_system_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(gui_page_system_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(gui_page_system_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(gui_page_system_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_page_system_settings, LV_ALIGN_TOP_LEFT, (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, 0);

    //* Unbalance source
    lv_obj_t *_tab = lv_obj_create(gui_page_system_settings);
    lv_obj_clear_flag(_tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab, LV_SIZE_CONTENT);
    lv_obj_set_height(_tab, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_tab, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Text
    lv_obj_t *_label = lv_label_create(_tab);
    lv_label_set_text(_label, "Unbalance\nsource");
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_width(_label, LV_SIZE_CONTENT);
    lv_obj_set_height(_label, LV_SIZE_CONTENT);

    // Dropdown button
    gui_unbalance_source_dropdown = lv_dropdown_create(_tab);
    lv_obj_set_width(gui_unbalance_source_dropdown, 50);
    lv_obj_set_height(gui_unbalance_source_dropdown, LV_SIZE_CONTENT);
    lv_obj_set_align(gui_unbalance_source_dropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(gui_unbalance_source_dropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_dropdown_set_options(gui_unbalance_source_dropdown, "X\nY");
    lv_obj_set_style_bg_color(gui_unbalance_source_dropdown, SECONDARY_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_unbalance_source_dropdown, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_unbalance_source_dropdown, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(gui_unbalance_source_dropdown, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Dropdown list
    lv_obj_t *_list = lv_dropdown_get_list(gui_unbalance_source_dropdown);
    lv_obj_set_style_bg_color(_list, SECONDARY_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_list, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_list, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(_list, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(_list, LV_OPA_COVER, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(_list, LV_OPA_TRANSP, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(_list, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add callback
    lv_obj_add_event_cb(gui_unbalance_source_dropdown, unbalance_source_settings_event_cb, LV_EVENT_ALL, NULL);

    //* Motor speed
    _tab = lv_obj_create(gui_page_system_settings);
    lv_obj_clear_flag(_tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab, screenWidth - ((6 * DEFAULT_TOOLBAR_HEIGHT) / 2));
    lv_obj_set_height(_tab, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Text
    _label = lv_label_create(_tab);
    lv_label_set_text(_label, "Motor\nspeed");
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_width(_label, LV_SIZE_CONTENT);
    lv_obj_set_height(_label, LV_SIZE_CONTENT);

    // Slider tab
    lv_obj_t *_tab_sub = lv_obj_create(_tab);
    lv_obj_clear_flag(_tab_sub, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab_sub, 180);
    lv_obj_set_height(_tab_sub, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab_sub, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab_sub, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab_sub, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_tab_sub, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(_tab_sub, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(_tab_sub, 15, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Slider
    gui_speed_slider = lv_slider_create(_tab_sub);
    lv_slider_set_range(gui_speed_slider, 100, 1000);
    lv_obj_set_width(gui_speed_slider, 130);

    // Add callback
    lv_obj_add_event_cb(gui_speed_slider, motor_speed_slider_settings_event_cb, LV_EVENT_ALL, NULL);

    // Value
    gui_speed_slider_value_label = lv_label_create(_tab_sub);
    lv_label_set_text(gui_speed_slider_value_label, "25%");
    lv_obj_set_style_text_color(gui_speed_slider_value_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    //* PAGE - Accelerometer
    gui_page_accel_settings = lv_obj_create(gui_SettingsScreen);
    lv_obj_set_flex_flow(gui_page_accel_settings, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_set_flex_align(gui_page_accel_settings, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(gui_page_accel_settings, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_accel_settings, screenWidth - (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, screenHeight);
    lv_obj_set_style_bg_opa(gui_page_accel_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_accel_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(gui_page_accel_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(gui_page_accel_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(gui_page_accel_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(gui_page_accel_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(gui_page_accel_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_page_accel_settings, LV_ALIGN_TOP_LEFT, (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, 0);
    lv_obj_add_flag(gui_page_accel_settings, LV_OBJ_FLAG_HIDDEN);

    //* Range
    _tab = lv_obj_create(gui_page_accel_settings);
    lv_obj_clear_flag(_tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab, LV_SIZE_CONTENT);
    lv_obj_set_height(_tab, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_tab, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Text
    _label = lv_label_create(_tab);
    lv_label_set_text(_label, "Range");
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_width(_label, LV_SIZE_CONTENT);
    lv_obj_set_height(_label, LV_SIZE_CONTENT);

    // Dropdown button
    gui_range_dropdown = lv_dropdown_create(_tab);
    lv_obj_set_width(gui_range_dropdown, 70);
    lv_obj_set_height(gui_range_dropdown, LV_SIZE_CONTENT);
    lv_obj_set_align(gui_range_dropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(gui_range_dropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_dropdown_set_options(gui_range_dropdown, "2G\n4G\n8G\n16G");
    lv_obj_set_style_bg_color(gui_range_dropdown, SECONDARY_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(gui_range_dropdown, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_range_dropdown, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(gui_range_dropdown, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Dropdown list
    _list = lv_dropdown_get_list(gui_range_dropdown);
    lv_obj_set_style_bg_color(_list, SECONDARY_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_list, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_list, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(_list, SECONDARY_ELEMENT_ACCENT_COLOR, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(_list, LV_OPA_COVER, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(_list, LV_OPA_TRANSP, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(_list, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add callback
    lv_obj_add_event_cb(gui_range_dropdown, range_settings_event_cb, LV_EVENT_ALL, NULL);

    //* Bandwidth
    // TODO

    //* PAGE - Filter
    gui_page_filter_settings = lv_obj_create(gui_SettingsScreen);
    lv_obj_set_flex_flow(gui_page_filter_settings, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_set_flex_align(gui_page_filter_settings, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(gui_page_filter_settings, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(gui_page_filter_settings, screenWidth - (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, screenHeight);
    lv_obj_set_style_bg_opa(gui_page_filter_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(gui_page_filter_settings, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(gui_page_filter_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(gui_page_filter_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(gui_page_filter_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(gui_page_filter_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(gui_page_filter_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(gui_page_filter_settings, LV_ALIGN_TOP_LEFT, (5 * DEFAULT_TOOLBAR_HEIGHT) / 2, 0);
    lv_obj_add_flag(gui_page_filter_settings, LV_OBJ_FLAG_HIDDEN);

    //* Q-factor
    _tab = lv_obj_create(gui_page_filter_settings);
    lv_obj_clear_flag(_tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab, screenWidth - ((9 * DEFAULT_TOOLBAR_HEIGHT) / 2));
    lv_obj_set_height(_tab, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Text
    _label = lv_label_create(_tab);
    lv_label_set_text(_label, "Q-Factor");
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_width(_label, LV_SIZE_CONTENT);
    lv_obj_set_height(_label, LV_SIZE_CONTENT);

    // Create spinbox tab
    lv_obj_t *_sub_tab = lv_obj_create(_tab);
    lv_obj_clear_flag(_sub_tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_sub_tab, LV_SIZE_CONTENT);
    lv_obj_set_height(_sub_tab, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_sub_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_sub_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_sub_tab, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_sub_tab, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(_sub_tab, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_sub_tab, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_sub_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_sub_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Create increment button
    lv_obj_t *btn = lv_btn_create(_sub_tab);
    lv_obj_set_size(btn, 15, 30);

    // Add style
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
    lv_obj_set_style_bg_color(btn, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, LV_PART_MAIN);

    // Add icon
    _label = lv_label_create(btn);
    lv_label_set_text(_label, LV_SYMBOL_PLUS);
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_center(_label);

    // Add callback
    lv_obj_add_event_cb(btn, QFactor_increment_btn_event_cb, LV_EVENT_ALL, NULL);

    // Create spinbox
    gui_QFactor_spinbox = lv_spinbox_create(_sub_tab);
    lv_spinbox_set_range(gui_QFactor_spinbox, 1, 2000);
    lv_spinbox_set_digit_format(gui_QFactor_spinbox, 4, 2);
    lv_spinbox_step_prev(gui_QFactor_spinbox);
    lv_spinbox_set_rollover(gui_QFactor_spinbox, false);
    lv_obj_set_width(gui_QFactor_spinbox, 50);
    lv_obj_set_height(gui_QFactor_spinbox, 30);
    lv_obj_set_style_pad_hor(gui_QFactor_spinbox, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(gui_QFactor_spinbox, 4, LV_PART_MAIN);

    // Add style
    lv_obj_set_style_bg_color(gui_QFactor_spinbox, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_color(gui_QFactor_spinbox, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(gui_QFactor_spinbox, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(gui_QFactor_spinbox, 3, LV_PART_MAIN);

    // Add callback
    lv_obj_add_event_cb(gui_QFactor_spinbox, QFactor_spinbox_event_cb, LV_EVENT_ALL, NULL);

    // Create decrement button
    btn = lv_btn_create(_sub_tab);
    lv_obj_set_size(btn, 15, 30);

    // Add style
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
    lv_obj_set_style_bg_color(btn, DEFAULT_BACKGROUND_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add icon
    _label = lv_label_create(btn);
    lv_label_set_text(_label, LV_SYMBOL_MINUS);
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(_label);

    // Add callback
    lv_obj_add_event_cb(btn, QFactor_decrement_btn_event_cb, LV_EVENT_ALL, NULL);

    //* Center frequency
    _tab = lv_obj_create(gui_page_filter_settings);
    lv_obj_clear_flag(_tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab, screenWidth - ((6 * DEFAULT_TOOLBAR_HEIGHT) / 2));
    lv_obj_set_height(_tab, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_tab, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(_tab, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(_tab, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Text
    _label = lv_label_create(_tab);
    lv_label_set_text(_label, "Center\nfreq.");
    lv_obj_set_style_text_color(_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN);
    lv_obj_set_width(_label, LV_SIZE_CONTENT);
    lv_obj_set_height(_label, LV_SIZE_CONTENT);

    // Slider tab
    _tab_sub = lv_obj_create(_tab);
    lv_obj_clear_flag(_tab_sub, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(_tab_sub, screenWidth - ((6 * DEFAULT_TOOLBAR_HEIGHT) / 2));
    lv_obj_set_height(_tab_sub, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_tab_sub, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_tab_sub, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(_tab_sub, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_tab_sub, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(_tab_sub, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(_tab_sub, 15, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Slider
    gui_freq_slider = lv_slider_create(_tab_sub);
    lv_slider_set_range(gui_freq_slider, 1, _xShared.getBandWidth());
    lv_obj_set_width(gui_freq_slider, screenWidth - ((9 * DEFAULT_TOOLBAR_HEIGHT) / 2));

    // Add callback
    lv_obj_add_event_cb(gui_freq_slider, freq_slider_settings_event_cb, LV_EVENT_ALL, NULL);

    // Value
    gui_freq_slider_value_label = lv_label_create(_tab_sub);
    lv_label_set_text_fmt(gui_freq_slider_value_label, "%dHz", (int16_t)(_xShared.getIIRCenterFreq() * 1000 * 2));
    lv_obj_set_style_text_color(gui_freq_slider_value_label, DEFAULT_ELEMENT_ACCENT_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    //* PAGE - Info
    // TODO
}

void _set_settings_page(settings_subpage_e page)
{
    _settings_act_page = page;
}

void _settings_page_manager(settings_subpage_e page)
{
    switch (page)
    {
    case SYSTEM_SETTINGS:
        _settings_show_system();
        break;
    case ACCEL_SETTINGS:
        _settings_show_accel();
        break;
    case FILTER_SETTINGS:
        _settings_show_filter();
        break;
    case INFO_SETTINGS:
        _settings_show_info();
        break;
    default:
        break;
    }

    for (size_t i = 0; i < lv_obj_get_child_cnt(gui_menu_list_settings); i++)
    {
        lv_obj_t *child = lv_obj_get_child(gui_menu_list_settings, i);
        if (i == page)
        {
            lv_obj_add_state(child, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(child, LV_STATE_CHECKED);
        }
    }
}

void _settings_show_system(void)
{
    // TODO load current value

    // Change visibility of pages
    lv_obj_add_flag(gui_page_accel_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_filter_settings, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(gui_page_info_settings, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_system_settings, LV_OBJ_FLAG_HIDDEN);

    _set_settings_page(SYSTEM_SETTINGS);
}

void _settings_show_accel(void)
{
    // TODO load current value

    // Change visibility of pages
    lv_obj_add_flag(gui_page_system_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_filter_settings, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(gui_page_info_settings, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_accel_settings, LV_OBJ_FLAG_HIDDEN);

    _set_settings_page(ACCEL_SETTINGS);
}

void _settings_show_filter(void)
{
    // TODO load current value

    // Change visibility of pages
    lv_obj_add_flag(gui_page_system_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_accel_settings, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(gui_page_info_settings, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(gui_page_filter_settings, LV_OBJ_FLAG_HIDDEN);

    _set_settings_page(FILTER_SETTINGS);
}

void _settings_show_info(void)
{
    // TODO load current value

    // Change visibility of pages
    lv_obj_add_flag(gui_page_system_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_accel_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui_page_filter_settings, LV_OBJ_FLAG_HIDDEN);

    // lv_obj_clear_flag(gui_page_info_settings, LV_OBJ_FLAG_HIDDEN);

    _set_settings_page(INFO_SETTINGS);
}
