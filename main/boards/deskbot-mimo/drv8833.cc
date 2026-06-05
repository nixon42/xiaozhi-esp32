#include "drv8833.h"
#include <esp_log.h>
#include <cmath>

#define TAG "Drv8833"

Drv8833Motor::Drv8833Motor(gpio_num_t pin1, gpio_num_t pin2, 
                           ledc_channel_t ch1, ledc_channel_t ch2, ledc_timer_t timer)
    : ch1_(ch1), ch2_(ch2) {
    
    // Configure PWM for Pin 1
    ledc_channel_config_t ledc_ch1 = {
        .gpio_num       = pin1,
        .speed_mode     = mode_,
        .channel        = ch1_,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = timer,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_ch1);

    // Configure PWM for Pin 2
    ledc_channel_config_t ledc_ch2 = {
        .gpio_num       = pin2,
        .speed_mode     = mode_,
        .channel        = ch2_,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = timer,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_ch2);
}

void Drv8833Motor::SetSpeed(int speed) {
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    uint32_t duty = (uint32_t)(std::abs(speed) * 8191 / 100);

    if (speed > 0) {
        ledc_set_duty(mode_, ch1_, duty);
        ledc_set_duty(mode_, ch2_, 0);
    } else if (speed < 0) {
        ledc_set_duty(mode_, ch1_, 0);
        ledc_set_duty(mode_, ch2_, duty);
    } else {
        ledc_set_duty(mode_, ch1_, 0);
        ledc_set_duty(mode_, ch2_, 0);
    }

    ledc_update_duty(mode_, ch1_);
    ledc_update_duty(mode_, ch2_);
    ESP_LOGD(TAG, "Motor SetSpeed: %d (Duty: %u)", speed, duty);
}

void Drv8833Motor::Stop() {
    SetSpeed(0);
}

Drv8833Driver::Drv8833Driver(gpio_num_t ain1, gpio_num_t ain2,
                             gpio_num_t bin1, gpio_num_t bin2,
                             gpio_num_t stby_pin) {
    if (stby_pin != GPIO_NUM_NC) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << stby_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(stby_pin, 1);
        ESP_LOGI(TAG, "STBY pin (GPIO %d) set to HIGH", stby_pin);
    }

    // Configure Timer FIRST
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    motor_a_ = new Drv8833Motor(ain1, ain2, LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_TIMER_0);
    motor_b_ = new Drv8833Motor(bin1, bin2, LEDC_CHANNEL_2, LEDC_CHANNEL_3, LEDC_TIMER_0);
    
    Stop();
}

void Drv8833Driver::Move(int speed_left, int speed_right) {
    // ESP_LOGI(TAG, "Driver Move: L=%d, R=%d", speed_left, speed_right);
    motor_a_->SetSpeed(speed_left);
    motor_b_->SetSpeed(speed_right);
}

void Drv8833Driver::Stop() {
    motor_a_->Stop();
    motor_b_->Stop();
}
