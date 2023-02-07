#ifndef INC_GUI_H
#define INC_GUI_H

#define LGFX_USE_V1
#include "../../components/LovyanGFX/src/LovyanGFX.hpp"
#include "../../components/lvgl/lvgl.h"

#include "../common/common_def.h"
#include "../shared/shared_data.h"

//************************/
//*      DEFINES         */
//************************/
#define LV_TICK_PERIOD_MS 1
#define FFT_MAJOR_TICK_COUNT 5 + 1

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

#define DEFAULT_BACKGROUND_COLOR lv_color_hex(0x031417)
#define DEFAULT_ELEMENT_ACCENT_COLOR lv_color_hex(0x169FB1)
#define SECONDARY_ELEMENT_ACCENT_COLOR lv_color_hex(0xFF9900)
#define DEFAULT_TOOLBAR_HEIGHT 30U

LV_IMG_DECLARE(propeller_img_png);
LV_IMG_DECLARE(settings_icon);
LV_IMG_DECLARE(nerd_face_icon);
LV_IMG_DECLARE(motor_icon);
LV_IMG_DECLARE(sensor_img);
LV_IMG_DECLARE(chart_ico);
LV_IMG_DECLARE(fft_ico);

LV_FONT_DECLARE(gui_font_med);

//*******************************/
//*      CLASS DECLARATION      */
//*******************************/
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance; // SPIバスのインスタンス
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();      // バス設定用の構造体を取得します。
      cfg.spi_host = VSPI_HOST;               // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      cfg.spi_mode = 0;                       // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;              // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read = 16000000;               // 受信時のSPIクロック
      cfg.spi_3wire = false;                  // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;                    // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = SPI_DMA_CH_AUTO;      // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      cfg.pin_sclk = 18;                      // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 23;                      // SPIのMOSIピン番号を設定
      cfg.pin_miso = 19;                      // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = 16;                        // SPIのD/Cピン番号を設定  (-1 = disable)
      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = 5;    // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = 17;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      cfg.panel_width = 240;    // 実際に表示可能な幅
      cfg.panel_height = 320;   // 実際に表示可能な高さ
      cfg.offset_x = 0;         // パネルのX方向オフセット量
      cfg.offset_y = 0;         // パネルのY方向オフセット量
      cfg.offset_rotation = 0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel = 8; // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits = 1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable = true;      // データ読出しが可能な場合 trueに設定
      cfg.invert = false;       // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order = false;    // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit = false;   // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared = true;    // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
      _panel_instance.config(cfg);
    }

    //*
    {
      auto cfg = _light_instance.config(); // バックライト設定用の構造体を取得します。

      cfg.pin_bl = 21;     // バックライトが接続されているピン番号
      cfg.invert = false;  // バックライトの輝度を反転させる場合 true
      cfg.freq = 44100;    // バックライトのPWM周波数
      cfg.pwm_channel = 7; // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // バックライトをパネルにセットします。
    }
    //*/

    //*
    {
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;            // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = 239;          // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;            // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = 319;          // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = 38;         // INTが接続されているピン番号
      cfg.bus_shared = true;    // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0;  // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
      cfg.spi_host = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 1000000;       // SPIクロックを設定
      cfg.pin_sclk = 18;        // SCLKが接続されているピン番号
      cfg.pin_mosi = 23;        // MOSIが接続されているピン番号
      cfg.pin_miso = 19;        // MISOが接続されているピン番号
      cfg.pin_cs = 22;          //   CSが接続されているピン番号
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }
    //*/

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

//*******************************/
//*         TYPE DEFINES        */
//*******************************/
enum dpb_page_t
{
  LOADING_PAGE,
  MAIN_PAGE,
  NERD_PAGE,
  FFT_PAGE,
};

//*********************************/
//*     FUNCTIONS DECLARATION     */
//*********************************/
//* TFT utilities
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

//* Initialization funcitons
uint8_t gui_init(QueueHandle_t xQueueComp2Sys_handle, QueueHandle_t xQueueSys2Comp_handle);
void gui_LoadingScreen_init(void);
void gui_MainScreen_init(void);
void gui_NerdScreen_init(void);
void gui_FFTScreen_init(void);

//* GUI management
void gui_update(void);
void gui_check_commands(void);
void gui_exe(command_data_t command);
void gui_show_page(dpb_page_t page);
void gui_values_update(void);
void gui_charts_update(void);
void gui_fft_update(void);
void gui_unbalance_arrow_update(void);

void _ask_peak_draw(void);
void _update_rpm(void);
void _update_fund(void);
void _update_but_labels(void);
void _update_unbalance(void);

//* Widget callbacks
void chart_slider_x_event_cb(lv_event_t *e);
void chart_slider_y_event_cb(lv_event_t *e);
void start_btn_event_cb(lv_event_t *e);
void nerd_btn_event_cb(lv_event_t *e);
void fft_btn_event_cb(lv_event_t *e);
void filter_btn_event_cb(lv_event_t *e);
void root_back_btn_event_cb(lv_event_t *e);
void FFTXChart_draw_event_cb(lv_event_t *e);
void FFTYChart_draw_event_cb(lv_event_t *e);
void AccelChart_draw_event_cb(lv_event_t *e);
void chart_switch_btn_event_cb(lv_event_t *e);
void list_btn_event_cb(lv_event_t *e);
void btn_show_x_charts_event_cb(lv_event_t *e);
void btn_show_y_charts_event_cb(lv_event_t *e);

//* Utilities
void _display_init(void);
void _create_toolbars_main(void);
void _create_anglechart_main(void);
void _create_unbalance_arrow(float_t angle_value, uint8_t lenght, uint8_t mirrored);
void _create_pages_nerd(void);
void _create_toolbars_nerd(void);
void _create_signal_chart(lv_obj_t **chart_handler, int16_t *data_array, lv_event_cb_t event_cb, size_t point_num, lv_obj_t **sliderX_handler, lv_obj_t **sliderY_handler, lv_obj_t *parent, lv_coord_t width, lv_coord_t height, lv_point_t position);
void _create_analisys_chart(lv_obj_t **chart_handler, int16_t *data_array, lv_event_cb_t event_cb, size_t point_num, lv_obj_t *parent, lv_coord_t width, lv_coord_t height, lv_point_t position);
void _chart_Y_autorange(lv_obj_t *chart_obj, lv_chart_series_t *ser);
#endif