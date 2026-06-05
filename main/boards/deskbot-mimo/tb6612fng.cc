#include "tb6612fng.h"
#include <esp_log.h>

#define TAG "Tb6612fng"

Tb6612fngMotor::Tb6612fngMotor(gpio_num_t pwm_pin, gpio_num_t in1_pin, gpio_num_t in2_pin, 
                               ledc_channel_t channel, ledc_timer_t timer)
    : in1_pin_(in1_pin), in2_pin_(in2_pin), channel_(channel) {
    
    // Configure GPIOs
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << in1_pin) | (1ULL << in2_pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // Configure PWM
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pwm_pin,
        .speed_mode     = mode_,
        .channel        = channel_,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = timer,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void Tb6612fngMotor::SetSpeed(int speed) {
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    uint32_t duty = (uint32_t)(abs(speed) * 8191 / 100);

    if (speed > 0) {
        gpio_set_level(in1_pin_, 1);
        gpio_set_level(in2_pin_, 0);
    } else if (speed < 0) {
        gpio_set_level(in1_pin_, 0);
        gpio_set_level(in2_pin_, 1);
    } else {
        gpio_set_level(in1_pin_, 0);
        gpio_set_level(in2_pin_, 0);
    }

    ledc_set_duty(mode_, channel_, duty);
    ledc_update_duty(mode_, channel_);
}

void Tb6612fngMotor::Stop() {
    SetSpeed(0);
}

Tb6612fngDriver::Tb6612fngDriver(gpio_num_t pwma, gpio_num_t ain1, gpio_num_t ain2,
                                 gpio_num_t pwmb, gpio_num_t bin1, gpio_num_t bin2,
                                 gpio_num_t stby)
    : motor_a_(pwma, ain1, ain2, LEDC_CHANNEL_0, LEDC_TIMER_0),
      motor_b_(pwmb, bin1, bin2, LEDC_CHANNEL_1, LEDC_TIMER_0),
      stby_pin_(stby) {
    
    // Configure Timer once for both channels
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure STBY pin
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << stby_pin_);
    gpio_config(&io_conf);
    
    Stop();
}

void Tb6612fngDriver::Move(int speed_left, int speed_right) {
    gpio_set_level(stby_pin_, 1);
    motor_a_.SetSpeed(speed_left);
    motor_b_.SetSpeed(speed_right);
}

void Tb6612fngDriver::Stop() {
    motor_a_.Stop();
    motor_b_.Stop();
    gpio_set_level(stby_pin_, 0);
}
