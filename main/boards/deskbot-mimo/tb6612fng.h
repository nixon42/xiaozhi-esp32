#ifndef TB6612FNG_H
#define TB6612FNG_H

#include <driver/gpio.h>
#include <driver/ledc.h>
#include "motor_driver.h"

class Tb6612fngMotor {
public:
    Tb6612fngMotor(gpio_num_t pwm_pin, gpio_num_t in1_pin, gpio_num_t in2_pin, 
                   ledc_channel_t channel, ledc_timer_t timer);
    
    void SetSpeed(int speed); // Speed from -100 to 100
    void Stop();

private:
    gpio_num_t in1_pin_;
    gpio_num_t in2_pin_;
    ledc_channel_t channel_;
    ledc_mode_t mode_ = LEDC_LOW_SPEED_MODE;
};

class Tb6612fngDriver : public MotorDriver {
public:
    Tb6612fngDriver(gpio_num_t pwma, gpio_num_t ain1, gpio_num_t ain2,
                    gpio_num_t pwmb, gpio_num_t bin1, gpio_num_t bin2,
                    gpio_num_t stby);
    
    void Move(int speed_left, int speed_right) override;
    void Stop() override;

private:
    Tb6612fngMotor motor_a_;
    Tb6612fngMotor motor_b_;
    gpio_num_t stby_pin_;
};

#endif // TB6612FNG_H
