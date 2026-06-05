#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/adc.h>
#include <driver/gpio.h>

#define MOTOR_TYPE_TB6612FNG 0
#define MOTOR_TYPE_DRV8833   1

// Pilih tipe motor driver di sini
#define DESKBOT_MOTOR_TYPE MOTOR_TYPE_DRV8833

struct HardwareConfig {
    gpio_num_t audio_i2s_gpio_ws;
    gpio_num_t audio_i2s_gpio_bclk;
    gpio_num_t audio_i2s_gpio_din;
    gpio_num_t audio_i2s_gpio_dout;

    gpio_num_t display_mosi_pin;
    gpio_num_t display_clk_pin;
    gpio_num_t display_dc_pin;
    gpio_num_t display_rst_pin;
    gpio_num_t display_cs_pin;

    gpio_num_t i2c_sda_pin;
    gpio_num_t i2c_scl_pin;

    // TB6612FNG Motor Driver
    gpio_num_t motor_pwma_pin;
    gpio_num_t motor_ain1_pin;
    gpio_num_t motor_ain2_pin;
    gpio_num_t motor_pwmb_pin;
    gpio_num_t motor_bin1_pin;
    gpio_num_t motor_bin2_pin;
    gpio_num_t motor_stby_pin;
};

// Default pins for Deskbot Mimo (ESP32-S3)
constexpr HardwareConfig DESKBOT_MIMO_CONFIG = {
    .audio_i2s_gpio_ws = GPIO_NUM_41,
    .audio_i2s_gpio_bclk = GPIO_NUM_40,
    .audio_i2s_gpio_din = GPIO_NUM_38,
    .audio_i2s_gpio_dout = GPIO_NUM_18,

    .display_mosi_pin = GPIO_NUM_16,
    .display_clk_pin = GPIO_NUM_15,
    .display_dc_pin = GPIO_NUM_6,
    .display_rst_pin = GPIO_NUM_7,
    .display_cs_pin = GPIO_NUM_5,

    .i2c_sda_pin = GPIO_NUM_17,
    .i2c_scl_pin = GPIO_NUM_14,

    .motor_pwma_pin = GPIO_NUM_1,
    .motor_ain1_pin = GPIO_NUM_3,
    .motor_ain2_pin = GPIO_NUM_4,
    .motor_pwmb_pin = GPIO_NUM_8,
    .motor_bin1_pin = GPIO_NUM_9,
    .motor_bin2_pin = GPIO_NUM_10,
    .motor_stby_pin = GPIO_NUM_11,
};

#define LCD_TYPE_SSD1306_SPI
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_INVERT_COLOR false
#define DISPLAY_RGB_ORDER LCD_RGB_ELEMENT_ORDER_RGB
#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 3

#define BOOT_BUTTON_GPIO GPIO_NUM_0

#define BUILTIN_LED_GPIO GPIO_NUM_48
#define BUILTIN_LED_COUNT 1

#endif
