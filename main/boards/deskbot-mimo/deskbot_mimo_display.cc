#include "deskbot_mimo_display.h"
#include <esp_log.h>
#include <esp_random.h>
#include <cstring>
#include <cmath>
#include "gifs.h"
#include "application.h"
#include "board.h"
#include "assets/lang_config.h"
#include "assets/monolog_config.h"

#define TAG "MimoDisplay"

// Simple 5x7 Font data for space to Z (ASCII 32-90)
static const uint8_t font5x7[59][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5f, 0x00, 0x00}, // '!' (33)
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"' (34)
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // '#' (35)
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // '$' (36)
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%' (37)
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&' (38)
    {0x00, 0x05, 0x03, 0x00, 0x00}, // ''' (39)
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // '(' (40)
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // ')' (41)
    {0x14, 0x08, 0x3e, 0x08, 0x14}, // '*' (42)
    {0x08, 0x08, 0x3e, 0x08, 0x08}, // '+' (43)
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ',' (44)
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-' (45)
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.' (46)
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/' (47)
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // '0' (48)
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // '1' (49)
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2' (50)
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // '3' (51)
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // '4' (52)
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5' (53)
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // '6' (54)
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7' (55)
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8' (56)
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // '9' (57)
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':' (58)
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';' (59)
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<' (60)
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '=' (61)
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>' (62)
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?' (63)
    {0x32, 0x49, 0x79, 0x41, 0x3e}, // '@' (64)
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // 'A' (65)
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // 'B' (66)
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // 'C' (67)
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // 'D' (68)
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // 'E' (69)
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // 'F' (70)
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // 'G' (71)
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // 'H' (72)
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // 'I' (73)
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // 'J' (74)
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // 'K' (75)
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // 'L' (76)
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // 'M' (77)
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // 'N' (78)
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // 'O' (79)
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // 'P' (80)
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // 'Q' (81)
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // 'R' (82)
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S' (83)
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // 'T' (84)
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // 'U' (85)
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // 'V' (86)
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // 'W' (87)
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X' (88)
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y' (89)
    {0x61, 0x51, 0x49, 0x45, 0x43}  // 'Z' (90)
};

// Frame counts from gifs.h
#define SCARE_FRAMES 33
#define BUXUE_FRAMES 33
#define SAD_GIF_FRAMES 33
#define ANGER_FRAMES 30
#define HAPPY_GIF_FRAMES 33
#define STATIC_GIF_FRAMES 35
#define SLEEPY_GIF_FRAMES 7

#define ANIMATION_FRAME_DELAY_MS 42 

MimoEmojiDisplay::MimoEmojiDisplay(ssd1306_t* oled_dev, int width, int height, bool mirror_x, bool mirror_y)
    : oled_dev_(oled_dev) {
    width_ = width;
    height_ = height;
}

MimoEmojiDisplay::~MimoEmojiDisplay() {
    if (animation_timer_) {
        esp_timer_stop(animation_timer_);
        esp_timer_delete(animation_timer_);
    }
}

void MimoEmojiDisplay::SetupUI() {
    Display::SetupUI();

    esp_timer_create_args_t timer_args = {
        .callback = AnimationTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "animation_timer",
        .skip_unhandled_events = false
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &animation_timer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(animation_timer_, ANIMATION_FRAME_DELAY_MS * 1000));

    SetEmotion("static");
}

void MimoEmojiDisplay::SetEmotion(const char* emotion) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Any activity should wake the display and reset the idle timer
    idle_frames_counter_ = 0;
    is_monolog_animating_ = false;
    if (display_off_) {
        display_off_ = false;
        ssd1306_display_on(oled_dev_, true);
    }

    expression_t next_exp;
    if (std::strcmp(emotion, "happy") == 0) {
        next_exp = EXPRESSION_GIF_HAPPY;
    } else if (std::strcmp(emotion, "sad") == 0) {
        next_exp = EXPRESSION_GIF_SAD;
    } else if (std::strcmp(emotion, "angry") == 0) {
        next_exp = EXPRESSION_GIF_ANGER;
    } else if (std::strcmp(emotion, "scare") == 0) {
        next_exp = EXPRESSION_GIF_SCARE;
    } else if (std::strcmp(emotion, "buxue") == 0) {
        next_exp = EXPRESSION_BUXUE;
    } else if (std::strcmp(emotion, "cool") == 0) {
        next_exp = EXPRESSION_BUXUE;
    } else if (std::strcmp(emotion, "confident") == 0) {
        next_exp = EXPRESSION_BUXUE;
    } else if (std::strcmp(emotion, "blink") == 0) {
        next_exp = EXPRESSION_GIF_BLINK;
    } else if (std::strcmp(emotion, "look_right") == 0) {
        next_exp = EXPRESSION_GIF_LOOK_RIGHT;
    } else if (std::strcmp(emotion, "look_left") == 0) {
        next_exp = EXPRESSION_GIF_LOOK_LEFT;
    } else if (std::strcmp(emotion, "sleepy") == 0) {
        next_exp = EXPRESSION_GIF_SLEEPY;
    } else if (std::strcmp(emotion, "gear") == 0) {
        next_exp = EXPRESSION_WIFI;
    } else if (std::strcmp(emotion, "fear") == 0) {
        next_exp = EXPRESSION_GIF_SCARE; 
    } else if (std::strcmp(emotion, "cloud clash") == 0) {
        next_exp = EXPRESSION_FATAL_ERROR;
    } else {
        next_exp = EXPRESSION_GIF_STATIC;
    }
    
    if (next_exp != current_expression_) {
        current_expression_ = next_exp;
        frame_index_ = 0;
        static_wait_ticks_ = 0;
    }
    debug_text_ = emotion;
}

void MimoEmojiDisplay::SetStatus(const char* status) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Any activity should wake the display and reset the idle timer
    idle_frames_counter_ = 0;
    is_monolog_animating_ = false;
    if (display_off_) {
        display_off_ = false;
        ssd1306_display_on(oled_dev_, true);
    }
    
    expression_t next_exp = current_expression_;
    if (std::strcmp(status, "Initializing") == 0) {
        next_exp = EXPRESSION_GIF_STATIC;
    } else if (std::strstr(status, "gear") != nullptr) {
        next_exp = EXPRESSION_WIFI;
    } else if (std::strstr(status, "Connecting") != nullptr) {
        next_exp = EXPRESSION_CONNECTING;
    } else if (std::strstr(status, "Listening") != nullptr) {
        next_exp = EXPRESSION_LISTENING;
    } else if (std::strstr(status, "Activating") != nullptr) {
        next_exp = EXPRESSION_BUXUE;
    } else if (std::strstr(status, "Upgrading") != nullptr) {
        next_exp = EXPRESSION_UPGRADING;
    } else if (std::strstr(status, "Error") != nullptr) {
        next_exp = EXPRESSION_FATAL_ERROR;
    }
    else if (std::strstr(status, "Checking for") != nullptr) {
        next_exp = EXPRESSION_CONNECTING;
    }
    else if (std::strstr(status, "Logging in") != nullptr) {
        next_exp = EXPRESSION_CONNECTING;
    }
    else if (std::strstr(status, "cloud clash") != nullptr){
        next_exp = EXPRESSION_FATAL_ERROR;
    }

 
    if (next_exp != current_expression_) {
        current_expression_ = next_exp;
        frame_index_ = 0;
        static_wait_ticks_ = 0;
    }
    debug_text_ = status; 
}

void MimoEmojiDisplay::SetPowerSaveMode(bool on) {
    Display::SetPowerSaveMode(on);
    std::lock_guard<std::mutex> lock(mutex_);
    if (on) {
        if (animation_timer_ != nullptr) {
            esp_timer_stop(animation_timer_);
        }
        ssd1306_display_on(oled_dev_, false);
    } else {
        ssd1306_display_on(oled_dev_, true);
        if (animation_timer_ != nullptr) {
            esp_timer_start_periodic(animation_timer_, ANIMATION_FRAME_DELAY_MS * 1000);
        }
    }
}

void MimoEmojiDisplay::UpdateStatusBar(bool update_all) {
    auto& app = Application::GetInstance();
    auto& board = Board::GetInstance();
    
    int battery_level;
    bool charging, discharging;
    if (board.GetBatteryLevel(battery_level, charging, discharging)) {
        // Jika level 0%, asumsikan tidak ada baterai (selalu dicolok USB), jadi abaikan.
        if (battery_level > 0 && battery_level <= 20 && discharging) {
            if (!is_low_battery_notified_) {
                is_low_battery_notified_ = true;
                ESP_LOGW(TAG, "Low battery detected! Level: %d%%", battery_level);
                
                // Set wajah menjadi sedih sementara
                SetEmotion("sad");
                app.Schedule([&app]() {
                    app.PlaySound(Lang::Sounds::OGG_LOW_BATTERY);
                });
            }
        } else {
            is_low_battery_notified_ = false;
        }
    }
}

bool MimoEmojiDisplay::Lock(int timeout_ms) {
    return mutex_.try_lock();
}

void MimoEmojiDisplay::Unlock() {
    mutex_.unlock();
}

void MimoEmojiDisplay::AnimationTimerCallback(void* arg) {
    MimoEmojiDisplay* display = (MimoEmojiDisplay*)arg;
    display->UpdateAnimation();
}

void MimoEmojiDisplay::UpdateAnimation() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (is_monolog_animating_ && frame_index_ >= 250) {
        is_monolog_animating_ = false;
        current_expression_ = EXPRESSION_GIF_STATIC;
        frame_index_ = 0;
    }

    // Auto-sleep logic: Neutral -> Sleepy (40s) -> Monologue (another 20s)
    if (current_expression_ == EXPRESSION_GIF_STATIC || current_expression_ == EXPRESSION_GIF_SLEEPY) {
        idle_frames_counter_++;
        // 40 seconds = 40,000ms / 42ms per frame ≈ 952 frames
        if (current_expression_ == EXPRESSION_GIF_STATIC && idle_frames_counter_ >= 952) {
            current_expression_ = EXPRESSION_GIF_SLEEPY;
            frame_index_ = 0;
        }
        // Total 1 minute for random monologue (1428 frames)
        if (idle_frames_counter_ >= 1428) {
            // Override display off with random monologue
            idle_frames_counter_ = 0;
            
            int rand_exp = esp_random() % 3;
            if (rand_exp == 0) current_expression_ = EXPRESSION_BUXUE;
            else if (rand_exp == 1) current_expression_ = EXPRESSION_LISTENING;
            else current_expression_ = EXPRESSION_GIF_HAPPY;
            
            frame_index_ = 0;
            is_monolog_animating_ = true;

            if (!Lang::Monolog::SOUNDS.empty()) {
                int rand_idx;
                if (Lang::Monolog::SOUNDS.size() > 1) {
                    do {
                        rand_idx = esp_random() % Lang::Monolog::SOUNDS.size();
                    } while (rand_idx == last_monolog_idx_);
                } else {
                    rand_idx = 0;
                }
                last_monolog_idx_ = rand_idx;
                
                auto sound_info = Lang::Monolog::SOUNDS[rand_idx];
                auto& app = Application::GetInstance();
                app.Schedule([&app, sound_info]() {
                    printf("{\"event\": \"play_monolog\", \"file\": \"%s\"}\n", sound_info.filename);
                    fflush(stdout);

                    // Send UI Trigger for PLAY_MONOLOGUE
                    char signal_name[64];
                    strncpy(signal_name, sound_info.filename, sizeof(signal_name));
                    signal_name[sizeof(signal_name) - 1] = '\0';
                    char* dot = strrchr(signal_name, '.');
                    if (dot) *dot = '\0';

                    app.SendUITrigger("PLAY_MONOLOGUE", signal_name);

                    float volume_mult = 1.0f;
                    int t_num = 0;
                    if (sscanf(signal_name, "T%d", &t_num) == 1) {
                        if (t_num >= 13 && t_num <= 24) {
                            volume_mult = 4.0f;
                        }
                    }

                    app.PlaySound(sound_info.data, volume_mult);
                });
            }
            return;
        }
    } else {
        idle_frames_counter_ = 0;
        if (display_off_) {
            display_off_ = false;
            ssd1306_display_on(oled_dev_, true);
        }
    }

    if (display_off_) return;

    const uint8_t* gif_frame = nullptr;

    // Static animation idle logic: hold frame 0 for a random duration
    if (current_expression_ == EXPRESSION_GIF_STATIC && static_wait_ticks_ > 0) {
        static_wait_ticks_--;
        gif_frame = gif_staticstate_data[0];
    } else {
        switch (current_expression_) {
            case EXPRESSION_BUXUE: {
                int display_index = frame_index_ / 2;
                if (display_index >= 26) {
                    // Ping-pong loop between frames 9-27 (indices 8-26)
                    // Start backward from max_index (26) for seamless entry
                    int rel = (display_index - 26) % 36;
                    if (rel < 18) display_index = 26 - rel;
                    else display_index = 8 + (rel - 18);
                }
                gif_frame = gif_buxue_data[display_index % BUXUE_FRAMES];
                break;
            }
            case EXPRESSION_GIF_HAPPY: {
                int display_index = frame_index_ / 2;
                if (display_index >= 24) {
                    // Ping-pong loop frames 7 to 25 (indices 6-24), slowed by 1.5x
                    // Start backward from max_index (24) for seamless entry
                    int rel = ((display_index - 24) * 2 / 3) % 36;
                    if (rel < 18) display_index = 24 - rel;
                    else display_index = 6 + (rel - 18);
                }
                gif_frame = gif_happy_data[display_index % HAPPY_GIF_FRAMES];
                break;
            }
            case EXPRESSION_GIF_SAD: {
                int display_index = frame_index_ / 2;
                if (display_index >= 24) {
                    // Cukup 3x loop (1 loop = 34 ping-pong frame) lalu kembali ke idle
                    if (display_index >= 24 + 34 * 3) {
                        current_expression_ = EXPRESSION_GIF_STATIC;
                        frame_index_ = 0;
                        idle_frames_counter_ = 0;
                        break;
                    }

                    // Ping-pong loop between frames 8-25 (indices 7-24)
                    // Start backward from max_index (24) for seamless entry
                    int rel = (display_index - 24) % 34;
                    if (rel < 17) display_index = 24 - rel;
                    else display_index = 7 + (rel - 17);
                }
                gif_frame = gif_sad_data[display_index % SAD_GIF_FRAMES];
                break;
            }
            case EXPRESSION_GIF_ANGER: {
                int display_index = frame_index_ / 2;
                if (display_index >= 24) {
                    // Ping-pong loop frames 9 to 25 (indices 8-24)
                    // Start backward from max_index (24) for seamless entry
                    int rel = (display_index - 24) % 32;
                    if (rel < 16) display_index = 24 - rel;
                    else display_index = 8 + (rel - 16);
                }
                gif_frame = gif_anger_data[display_index % ANGER_FRAMES];
                break;
            }
            case EXPRESSION_GIF_SCARE: {
                int display_index = frame_index_ / 2;
                if (display_index >= 25) {
                    // Ping-pong loop frames 8 to 26 (indices 7-25)
                    // Start backward from max_index (25) for seamless entry
                    int rel = (display_index - 25) % 36;
                    if (rel < 18) display_index = 25 - rel;
                    else display_index = 7 + (rel - 18);
                }
                gif_frame = gif_scare_data[display_index % SCARE_FRAMES];
                break;
            }
            case EXPRESSION_GIF_SLEEPY: {
                int display_index = frame_index_ / 10; // Restore base speed
                if (display_index >= 6) {
                    // Ping-pong loop frames 4 to 7 (indices 3-6), slowed by 3x
                    // Start backward from max_index (6) for seamless entry
                    int rel = ((display_index - 6) / 3) % 6;
                    if (rel < 3) display_index = 6 - rel;
                    else display_index = 3 + (rel - 3);
                }
                gif_frame = gif_sleepy_data[display_index % SLEEPY_GIF_FRAMES];
                break;
            }
            case EXPRESSION_GIF_STATIC: {
                // Determine if we need to pick a new sub-animation
                int sub_frames;
                if (idle_sub_exp_ == EXPRESSION_GIF_BLINK) sub_frames = 6;
                else if (idle_sub_exp_ == EXPRESSION_GIF_LOOK_RIGHT || idle_sub_exp_ == EXPRESSION_GIF_LOOK_LEFT) sub_frames = 10;
                else sub_frames = 1;

                if ((frame_index_ / idle_speed_divisor_) >= sub_frames) {
                    frame_index_ = 0;
                    int r = esp_random() % 100;
                    if (r < 30) { // 30% Stay Idle
                        idle_sub_exp_ = EXPRESSION_GIF_STATIC;
                        static_wait_ticks_ = 20 + (esp_random() % 60);
                        idle_speed_divisor_ = 2;
                    } else if (r < 70) { // 40% Blink
                        idle_sub_exp_ = EXPRESSION_GIF_BLINK;
                        idle_speed_divisor_ = 2 + (esp_random() % 2); 
                    } else if (r < 80) { // 10% Look Right
                        idle_sub_exp_ = EXPRESSION_GIF_LOOK_RIGHT;
                        idle_speed_divisor_ = 3 + (esp_random() % 2);
                    } else if (r < 90) { // 10% Look Left
                        idle_sub_exp_ = EXPRESSION_GIF_LOOK_LEFT;
                        idle_speed_divisor_ = 3 + (esp_random() % 2);
                    } else { // 10% Sleepy
                        idle_sub_exp_ = EXPRESSION_GIF_SLEEPY;
                        idle_speed_divisor_ = 4 + (esp_random() % 2);
                    }
                }

                if (static_wait_ticks_ > 0) {
                    static_wait_ticks_--;
                    gif_frame = gif_staticstate_data[0];
                } else {
                    switch (idle_sub_exp_) {
                        case EXPRESSION_GIF_BLINK:
                            gif_frame = gif_staticstate_data[(frame_index_ / idle_speed_divisor_) % 6];
                            break;
                        case EXPRESSION_GIF_LOOK_RIGHT:
                            gif_frame = gif_staticstate_data[6 + (frame_index_ / idle_speed_divisor_) % 10];
                            break;
                        case EXPRESSION_GIF_LOOK_LEFT:
                            gif_frame = gif_staticstate_data[15 + (frame_index_ / idle_speed_divisor_) % 10];
                            break;
                        default:
                            gif_frame = gif_staticstate_data[0];
                            break;
                    }
                }
                break;
            }
            case EXPRESSION_GIF_BLINK:
                gif_frame = gif_staticstate_data[(frame_index_ / 5) % 6];
                break;
            case EXPRESSION_GIF_LOOK_RIGHT:
                gif_frame = gif_staticstate_data[6 + (frame_index_ / 6) % 10];
                break;
            case EXPRESSION_GIF_LOOK_LEFT:
                gif_frame = gif_staticstate_data[15 + (frame_index_ / 6) % 10];
                break;
            default:
                break;
        }
    }

    if (gif_frame) {
        std::memcpy(oled_dev_->buffer, gif_frame, 1024);
    } else {
        DrawProcedural(current_expression_, frame_index_);
    }
    
    if (!debug_text_.empty()) {
        DrawString(0, 0, debug_text_.c_str());
    }

    ssd1306_refresh(oled_dev_);
    frame_index_++;
}

void MimoEmojiDisplay::DrawFilledCircle(int xm, int ym, int r) {
    if (r <= 0) return;
    for (int j = -r; j <= r; j++) {
        for (int i = -r; i <= r; i++) {
            if (i * i + j * j <= r * r) {
                ssd1306_draw_pixel(oled_dev_, xm + i, ym + j, 1);
            }
        }
    }
}

void MimoEmojiDisplay::DrawRect(int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++) {
        for (int j = y; j < y + h; j++) {
            ssd1306_draw_pixel(oled_dev_, i, j, 1);
        }
    }
}

void MimoEmojiDisplay::DrawChar(int x, int y, char c) {
    if (c < ' ' || c > 'Z') return;
    const uint8_t *bits = font5x7[c - ' '];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 7; j++) {
            if (bits[i] & (1 << j)) {
                ssd1306_draw_pixel(oled_dev_, x + i, y + j, 1);
            } else {
                ssd1306_draw_pixel(oled_dev_, x + i, y + j, 0); 
            }
        }
    }
}

void MimoEmojiDisplay::DrawString(int x, int y, const char* str) {
    while (*str) {
        // Convert to uppercase for the limited font
        char c = *str++;
        if (c >= 'a' && c <= 'z') c -= 32;
        DrawChar(x, y, c);
        x += 6;
        if (x + 6 > width_) break;
    }
}

void MimoEmojiDisplay::DrawProcedural(expression_t exp, int frame_idx) {
    ssd1306_clear(oled_dev_);
    switch (exp) {
        case EXPRESSION_WIFI: {
            int cx = 64, cy = 45;
            int n = (frame_idx / 3) % 4;
            DrawFilledCircle(cx, cy, 2);
            for (int i = 1; i <= n; i++) {
                int r = i * 10;
                for (int a = -45; a <= 45; a++) {
                    float rad = a * 0.01745f;
                    int px = cx + (int)(r * sinf(rad));
                    int py = cy - (int)(r * cosf(rad));
                    ssd1306_draw_pixel(oled_dev_, px, py, 1);
                    ssd1306_draw_pixel(oled_dev_, px, py - 1, 1);
                }
            }
            break;
        }
        case EXPRESSION_CONNECTING: {
            int p = (frame_idx / 3) % 4;
            for (int i = 0; i < 3; i++) {
                int r = (i == p) ? 6 : 3;
                DrawFilledCircle(44 + i * 20, 32, r);
            }
            break;
        }
        case EXPRESSION_LISTENING: {
            for (int i = 0; i < 7; i++) {
                float w = sinf((frame_idx * 0.5f) + i);
                int h = 10 + (int)(15 * (w < 0 ? -w : w));
                DrawRect(34 + i * 10, 32 - h/2, 4, h);
            }
            break;
        }
        case EXPRESSION_UPGRADING: {
            int cx = 64, cy = 25;
            DrawRect(cx - 2, cy - 10, 4, 15);
            for (int i = 0; i < 10; i++) {
                ssd1306_draw_pixel(oled_dev_, cx - i, cy + 5 - i, 1);
                ssd1306_draw_pixel(oled_dev_, cx + i, cy + 5 - i, 1);
            }
            DrawRect(34, 50, 60, 2);
            int progress = (frame_idx % 60);
            DrawRect(34, 50, progress, 2);
            break;
        }
        case EXPRESSION_FATAL_ERROR: {
            int cx = 64, cy = 32, esp = 25;
            for (int i = -10; i <= 10; i++) {
                ssd1306_draw_pixel(oled_dev_, cx - esp + i, cy + i, 1);
                ssd1306_draw_pixel(oled_dev_, cx - esp + i, cy - i, 1);
                ssd1306_draw_pixel(oled_dev_, cx + esp + i, cy + i, 1);
                ssd1306_draw_pixel(oled_dev_, cx + esp + i, cy - i, 1);
            }
            break;
        }
        default: break;
    }
}
