#include "espdet_detect.hpp"
#include "frame_cap_pipeline.hpp"
#include "who_detect_app_lcd.hpp"
#include "who_detect_app_term.hpp"
#include "bsp/esp-bsp.h"

using namespace who::frame_cap;
using namespace who::app;

dl::detect::Detect *get_detect_model()
{
#if defined(CONFIG_ESPDET_DETECT_MODEL_LOCATION)
    return new ESPDetDetect(static_cast<ESPDetDetect::model_type_t>(CONFIG_DEFAULT_ESPDET_DETECT_MODEL), false);
#else
    ESP_LOGE("MAIN", "No detect model component in idf_component.yml");
    return nullptr;
#endif
}

void run_detect_lcd()
{
    WhoFrameCapNode *lcd_disp_frame_cap_node = nullptr;
#if CONFIG_IDF_TARGET_ESP32P4
    auto frame_cap = get_lcd_mipi_csi_ppa_frame_cap_pipeline(&lcd_disp_frame_cap_node);
    // auto frame_cap = get_lcd_mipi_csi_frame_cap_pipeline();
    // auto frame_cap = get_lcd_uvc_frame_cap_pipeline();
#endif
    auto detect_app = new WhoDetectAppLCD({{255, 0, 0}}, frame_cap, lcd_disp_frame_cap_node);
    auto model = get_detect_model();
    model->set_score_thr(0.45, 0); // default 0.25, raise to suppress false positives
    detect_app->set_model(model);
    detect_app->set_fps(10.0f); // Run inference at 10 FPS, not full camera rate.
    detect_app->run();
}

void run_detect_term()
{
#if CONFIG_IDF_TARGET_ESP32P4
    auto frame_cap = get_term_mipi_csi_frame_cap_pipeline();
    // auto frame_cap = get_term_mipi_csi_ppa_frame_cap_pipeline();
    // auto frame_cap = get_term_uvc_frame_cap_pipeline();
#endif
    auto detect_app = new WhoDetectAppTerm(frame_cap);
    auto model = get_detect_model();
    // model->set_score_thr(0.5f, 0);
    detect_app->set_model(model);
    detect_app->set_fps(10.0f);
    detect_app->run();
}

extern "C" void app_main(void)
{
    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), 5);
#if CONFIG_ESPDET_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

    run_detect_lcd();
    // try this if you don't have a lcd.
    // run_detect_term();
}
