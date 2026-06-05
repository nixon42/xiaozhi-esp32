#include "ssd1306.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SSD1306";

static void ssd1306_write_cmd(ssd1306_t *dev, uint8_t cmd) {
    gpio_set_level(dev->dc_pin, 0);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    spi_device_polling_transmit(dev->spi, &t);
}

static void ssd1306_write_data(ssd1306_t *dev, uint8_t *data, size_t len) {
    gpio_set_level(dev->dc_pin, 1);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = data;
    spi_device_polling_transmit(dev->spi, &t);
}

esp_err_t ssd1306_init(ssd1306_t *dev, spi_host_device_t host, int mosi_pin, int clk_pin, int cs_pin, int dc_pin, int reset_pin) {
    dev->dc_pin = dc_pin;
    dev->reset_pin = reset_pin;
    dev->cs_pin = cs_pin;

    // Configure DC, RST, CS pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << dc_pin) | (1ULL << reset_pin) | (1ULL << cs_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // SPI Bus Config
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = mosi_pin,
        .sclk_io_num = clk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SSD1306_WIDTH * SSD1306_HEIGHT / 8,
    };
    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    // SPI Device Config
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10MHz
        .mode = 0,
        .spics_io_num = cs_pin,
        .queue_size = 7,
    };
    ret = spi_bus_add_device(host, &devcfg, &dev->spi);
    if (ret != ESP_OK) return ret;

    // Reset Display
    gpio_set_level(reset_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(reset_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // SSD1306 Initialization Sequence
    ssd1306_write_cmd(dev, 0xAE); // Display OFF
    ssd1306_write_cmd(dev, 0xD5); // Set Display Clock Divide Ratio
    ssd1306_write_cmd(dev, 0x80);
    ssd1306_write_cmd(dev, 0xA8); // Set Multiplex Ratio
    ssd1306_write_cmd(dev, 0x3F);
    ssd1306_write_cmd(dev, 0xD3); // Set Display Offset
    ssd1306_write_cmd(dev, 0x00);
    ssd1306_write_cmd(dev, 0x40); // Set Display Start Line
    ssd1306_write_cmd(dev, 0x8D); // Charge Pump
    ssd1306_write_cmd(dev, 0x14);
    ssd1306_write_cmd(dev, 0x20); // Addressing Mode
    ssd1306_write_cmd(dev, 0x00); // Horizontal
    ssd1306_write_cmd(dev, 0xA1); // Segment Re-map
    ssd1306_write_cmd(dev, 0xC8); // COM Output Scan Direction
    ssd1306_write_cmd(dev, 0xDA); // COM Pins hardware configuration
    ssd1306_write_cmd(dev, 0x12);
    ssd1306_write_cmd(dev, 0x81); // Contrast Control
    ssd1306_write_cmd(dev, 0x7F);
    ssd1306_write_cmd(dev, 0xD9); // Pre-charge Period
    ssd1306_write_cmd(dev, 0xF1);
    ssd1306_write_cmd(dev, 0xDB); // VCOMH Deselect Level
    ssd1306_write_cmd(dev, 0x40);
    ssd1306_write_cmd(dev, 0xA4); // Entire Display ON
    ssd1306_write_cmd(dev, 0xA6); // Normal Display
    ssd1306_write_cmd(dev, 0xAF); // Display ON

    ssd1306_clear(dev);
    ssd1306_refresh(dev);

    return ESP_OK;
}

void ssd1306_draw_pixel(ssd1306_t *dev, int x, int y, int color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if (color) {
        dev->buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        dev->buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void ssd1306_clear(ssd1306_t *dev) {
    memset(dev->buffer, 0, sizeof(dev->buffer));
}

void ssd1306_refresh(ssd1306_t *dev) {
    ssd1306_write_cmd(dev, 0x21); // Column Address
    ssd1306_write_cmd(dev, 0);
    ssd1306_write_cmd(dev, SSD1306_WIDTH - 1);
    ssd1306_write_cmd(dev, 0x22); // Page Address
    ssd1306_write_cmd(dev, 0);
    ssd1306_write_cmd(dev, (SSD1306_HEIGHT / 8) - 1);
    ssd1306_write_data(dev, dev->buffer, sizeof(dev->buffer));
}

void ssd1306_draw_bitmap(ssd1306_t *dev, int x, int y, const uint8_t *bitmap, int w, int h) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int byte_idx = (i + j * w) / 8;
            int bit_idx = (i + j * w) % 8;
            if (bitmap[byte_idx] & (0x80 >> bit_idx)) {
                ssd1306_draw_pixel(dev, x + i, y + j, 1);
            }
        }
    }
}

void ssd1306_display_on(ssd1306_t *dev, bool on) {
    if (on) {
        ssd1306_write_cmd(dev, 0xAF); // Display ON
    } else {
        ssd1306_write_cmd(dev, 0xAE); // Display OFF
    }
}
