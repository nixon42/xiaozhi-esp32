#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stddef.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    spi_device_handle_t spi;
    gpio_num_t dc_pin;
    gpio_num_t reset_pin;
    gpio_num_t cs_pin;
    uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
} ssd1306_t;

esp_err_t ssd1306_init(ssd1306_t *dev, spi_host_device_t host, int mosi_pin, int clk_pin, int cs_pin, int dc_pin, int reset_pin);
void ssd1306_draw_pixel(ssd1306_t *dev, int x, int y, int color);
void ssd1306_clear(ssd1306_t *dev);
void ssd1306_refresh(ssd1306_t *dev);
void ssd1306_display_on(ssd1306_t *dev, bool on);
void ssd1306_draw_bitmap(ssd1306_t *dev, int x, int y, const uint8_t *bitmap, int w, int h);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_H
