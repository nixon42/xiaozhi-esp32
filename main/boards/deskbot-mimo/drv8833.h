#ifndef DRV8833_H
#define DRV8833_H

#include <driver/gpio.h>
#include <driver/ledc.h>
#include "motor_driver.h"

class Drv8833Motor {
public:
    Drv8833Motor(gpio_num_t pin1, gpio_num_t pin2, 
                 ledc_channel_t ch1, ledc_channel_t ch2, ledc_timer_t timer);
    
    void SetSpeed(int speed);
    void Stop();

private:
    ledc_channel_t ch1_;
    ledc_channel_t ch2_;
    ledc_mode_t mode_ = LEDC_LOW_SPEED_MODE;
};

class Drv8833Driver : public MotorDriver {
public:
    Drv8833Driver(gpio_num_t ain1, gpio_num_t ain2,
                  gpio_num_t bin1, gpio_num_t bin2,
                  gpio_num_t stby_pin);
    
    void Move(int speed_left, int speed_right) override;
    void Stop() override;

private:
    Drv8833Motor* motor_a_;
    Drv8833Motor* motor_b_;
};

#endif // DRV8833_H
