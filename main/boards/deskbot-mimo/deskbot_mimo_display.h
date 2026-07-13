#pragma once

#include "display/display.h"
#include "ssd1306.h"
#include <mutex>
#include <esp_timer.h>
#include <string>

typedef enum {
    EXPRESSION_GIF_HAPPY,
    EXPRESSION_GIF_SAD,
    EXPRESSION_GIF_ANGER,
    EXPRESSION_GIF_SCARE,
    EXPRESSION_GIF_STATIC,
    EXPRESSION_GIF_BLINK,
    EXPRESSION_GIF_LOOK_RIGHT,
    EXPRESSION_GIF_LOOK_LEFT,
    EXPRESSION_BUXUE,
    EXPRESSION_WIFI,
    EXPRESSION_CONNECTING,
    EXPRESSION_LISTENING,
    EXPRESSION_UPGRADING,
    EXPRESSION_GIF_SLEEPY,
    EXPRESSION_FATAL_ERROR
} expression_t;

class MimoEmojiDisplay : public Display {
public:
    MimoEmojiDisplay(ssd1306_t* oled_dev, int width, int height, bool mirror_x, bool mirror_y);
    virtual ~MimoEmojiDisplay();

    virtual void SetStatus(const char* status) override;
    virtual void SetEmotion(const char* emotion) override;
    virtual void SetPowerSaveMode(bool on) override;
    virtual void UpdateStatusBar(bool update_all = false) override;
    virtual void SetupUI() override;

private:
    ssd1306_t* oled_dev_;
    expression_t current_expression_ = EXPRESSION_GIF_STATIC;
    int frame_index_ = 0;
    int static_wait_ticks_ = 0;
    int idle_speed_divisor_ = 2;
    int idle_frames_counter_ = 0;
    bool display_off_ = false;
    bool is_monolog_animating_ = false;
    int last_monolog_idx_ = -1;
    expression_t idle_sub_exp_ = EXPRESSION_GIF_STATIC;
    bool is_low_battery_notified_ = false;
    std::mutex mutex_;
    esp_timer_handle_t animation_timer_ = nullptr;
    std::string debug_text_;

    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;

    void UpdateAnimation();
    static void AnimationTimerCallback(void* arg);
    void DrawProcedural(expression_t exp, int frame_idx);
    void DrawFilledCircle(int xm, int ym, int r);
    void DrawRect(int x, int y, int w, int h);
    void DrawString(int x, int y, const char* str);
    void DrawChar(int x, int y, char c);
};
