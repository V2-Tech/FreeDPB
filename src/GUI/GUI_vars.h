#ifndef INC_GUI_INTERNALS_H
#define INC_GUI_INTERNALS_H

#define LGFX_USE_V1
#include "../../components/LovyanGFX/src/LovyanGFX.hpp"
#include "../../components/lvgl/lvgl.h"

#include "../common/common_def.h"
#include "../shared/shared_data.h"

//************************/
//*      VARIABLES       */
//************************/
static const char *TAG = "GUI";

//* Inter-pages usage
static LGFX tft;
DPBShared &_xShared = DPBShared::getInstance();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];
static QueueHandle_t _xQueueCom2Sys = NULL;
static QueueHandle_t _xQueueSys2Comp = NULL;
static uint8_t _gui_init_done = 0;
static dpb_page_e _gui_act_page = LOADING_PAGE;
static uint8_t _peak_draw_done = 0;
static uint8_t _fftX_label_draw_done = 0;
static uint8_t _fftY_label_draw_done = 0;
static nerd_subpage_e _nerd_act_page = X_RAW;
static settings_subpage_e _settings_act_page = SYSTEM_SETTINGS;
static gui_sys_step_e _gui_act_step = GUI_SYS_STEP_NONE;

//* Page handlers
lv_obj_t *gui_LoadingScreen = NULL;
lv_obj_t *gui_MainScreen = NULL;
lv_obj_t *gui_NerdScreen = NULL;
lv_obj_t *gui_SettingsScreen = NULL;

//* Main screen objects
lv_obj_t *gui_StartBut = NULL;
lv_obj_t *gui_StartButLabel = NULL;
lv_obj_t *gui_SearchTypeSwitch = NULL;
lv_obj_t *gui_ResetBut = NULL;
lv_obj_t *gui_OffsetSpinboxTab = NULL;
lv_obj_t *gui_OffsetSpinbox = NULL;
lv_obj_t *gui_SettingsBut = NULL;
lv_obj_t *gui_NerdBut = NULL;
lv_obj_t *gui_RPMLabelText = NULL;
lv_obj_t *gui_UnbalanceAngleTab = NULL;
lv_obj_t *gui_UnbalanceAngleLine = NULL;
lv_obj_t *gui_UnbalanceAngleLineEnd = NULL;
lv_obj_t *gui_UnbalanceLabValue = NULL;
lv_obj_t *gui_4StepsTab = NULL;
lv_obj_t *gui_Steps1Lab = NULL;
lv_obj_t *gui_Steps2Lab = NULL;
lv_obj_t *gui_Steps3Lab = NULL;
lv_obj_t *gui_Steps4Lab = NULL;
lv_obj_t *gui_StepBackBut = NULL;

//* Nerd screen objects
lv_obj_t *gui_action_list_nerd = NULL;
lv_obj_t *gui_page_signal_x = NULL;
lv_obj_t *gui_page_signal_y = NULL;
lv_obj_t *gui_page_fft_x = NULL;
lv_obj_t *gui_page_fft_y = NULL;

// Acceleration charts
lv_obj_t *gui_AccelXChart = NULL;
lv_obj_t *gui_AccelYChart = NULL;
lv_chart_series_t *serAccX = NULL;
lv_chart_series_t *serAccY = NULL;
int16_t accX_sample[ACC_CHART_POINT_COUNT] = {0};
int16_t accY_sample[ACC_CHART_POINT_COUNT] = {0};
lv_obj_t *gui_AccelChart_Xslider = NULL;
lv_obj_t *gui_AccelChart_Yslider = NULL;

// Fourier charts
lv_obj_t *gui_FFTXChart = NULL;
lv_obj_t *gui_FFTYChart = NULL;
lv_chart_series_t *serFFTX = NULL;
lv_chart_series_t *serFFTY = NULL;
int16_t fftX_sample[FFT_DATA_BUFFER_SIZE] = {0};
int16_t fftY_sample[FFT_DATA_BUFFER_SIZE] = {0};

// Labels
lv_obj_t *gui_FundLabel = NULL;
lv_obj_t *rot_cnt_label = NULL;
lv_obj_t *error_label = NULL;

//* Settings screen objects
lv_obj_t *gui_menu_list_settings = NULL;
lv_obj_t *gui_page_system_settings = NULL;
lv_obj_t *gui_page_accel_settings = NULL;
lv_obj_t *gui_page_filter_settings = NULL;
lv_obj_t *gui_page_info_settings = NULL;

// System
lv_obj_t *gui_unbalance_source_dropdown = NULL;
lv_obj_t *gui_speed_slider = NULL;
lv_obj_t *gui_speed_slider_value_label = NULL;

// Accelerometer
lv_obj_t *gui_range_dropdown = NULL;
lv_obj_t *gui_bandwidth_dropdown = NULL;

// Filter
lv_obj_t *gui_freq_slider = NULL;
lv_obj_t *gui_freq_slider_value_label = NULL;
lv_obj_t *gui_QFactor_spinbox = NULL;

//* Settings temporary variables
int32_t gui_unbalanceSource = 0;
int32_t gui_measureThrottle = 0;
int32_t gui_range = 0;
int32_t gui_bandWidth = 0;
int32_t gui_iirCenterFreq = 0;
int32_t gui_iirQFactor = 0;

#endif