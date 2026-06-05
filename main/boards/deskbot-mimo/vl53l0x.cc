#include "vl53l0x.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "VL53L0X"

Vl53l0x::Vl53l0x(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
}

bool Vl53l0x::Initialize() {
    uint8_t ids[3] = {0, 0, 0};
    ReadRegNoAbort(0xC0, &ids[0]);
    ReadRegNoAbort(0xC1, &ids[1]);
    ReadRegNoAbort(0xC2, &ids[2]);
    
    ESP_LOGI(TAG, "Sensor IDs: 0xC0:0x%02X, 0xC1:0x%02X, 0xC2:0x%02X", ids[0], ids[1], ids[2]);

    if (ids[0] == 0 && ids[1] == 0 && ids[2] == 0) {
        ESP_LOGE(TAG, "Failed to read sensor ID (all zero)");
        // Try one more thing: maybe the sensor is in a state where it needs a reset?
        // For now, we'll return false.
        return false;
    }
    
    // DataInit sequence (from Pololu/ST API)
    WriteRegNoAbort(0x88, 0x00);
    WriteRegNoAbort(0x80, 0x01);
    WriteRegNoAbort(0xFF, 0x01);
    WriteRegNoAbort(0x00, 0x00);
    ReadRegNoAbort(0x91, &stop_variable_);
    WriteRegNoAbort(0x00, 0x01);
    WriteRegNoAbort(0xFF, 0x00);
    WriteRegNoAbort(0x80, 0x00);

    // Disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks
    WriteRegNoAbort(0x60, 0x00);

    // Set signal rate limit to 0.1 * (1 << 7) = 0x00C8
    WriteRegNoAbort(0x44, 0x00);
    WriteRegNoAbort(0x45, 0xC8);

    // Sequence config: enable all steps
    WriteRegNoAbort(0x01, 0xFF);

    // Clear interrupt & set interrupt config
    WriteRegNoAbort(0x0A, 0x04);
    WriteRegNoAbort(0x0B, 0x01);

    return true;
}

uint16_t Vl53l0x::ReadDistance() {
    // Start single-shot measurement (Pololu style)
    WriteRegNoAbort(0x80, 0x01);
    WriteRegNoAbort(0xFF, 0x01);
    WriteRegNoAbort(0x00, 0x00);
    WriteRegNoAbort(0x91, stop_variable_);
    WriteRegNoAbort(0x00, 0x01);
    WriteRegNoAbort(0xFF, 0x00);
    WriteRegNoAbort(0x80, 0x00);

    WriteRegNoAbort(0x0B, 0x01); // Clear any pending interrupt
    WriteRegNoAbort(0x00, 0x01); // SYSRANGE_START

    // Wait for SYSRANGE_START to clear (busy bit)
    int timeout = 100;
    uint8_t sysrange = 0;
    while (timeout > 0) {
        if (ReadRegNoAbort(0x00, &sysrange) == ESP_OK && !(sysrange & 0x01)) break;
        vTaskDelay(pdMS_TO_TICKS(5));
        timeout--;
    }
    if (timeout == 0) {
        ESP_LOGE(TAG, "Timeout: SYSRANGE_START did not clear");
        return 8190;
    }

    // Wait for result interrupt (bits [2:0] of RESULT_INTERRUPT_STATUS)
    timeout = 100;
    uint8_t status = 0;
    while (timeout > 0) {
        if (ReadRegNoAbort(0x13, &status) == ESP_OK && (status & 0x07)) break;
        vTaskDelay(pdMS_TO_TICKS(5));
        timeout--;
    }

    if (timeout == 0) {
        ESP_LOGE(TAG, "Timeout: RESULT_INTERRUPT_STATUS=0x%02X", status);
        return 8190;
    }

    // Read distance from 0x1E (2 bytes big-endian)
    uint8_t buffer[2];
    if (ReadRegsNoAbort(0x1E, buffer, 2) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read distance registers");
        return 8190;
    }
    uint16_t distance = (uint16_t(buffer[0]) << 8) | buffer[1];

    // Clear interrupt
    WriteRegNoAbort(0x0B, 0x01);

    return distance;
}
