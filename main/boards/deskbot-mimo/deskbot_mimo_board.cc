#include "wifi_board.h"
#include "deskbot_mimo_display.h"
#include "audio/audio_codec.h"
#include "audio/codecs/no_audio_codec.h"
#include "config.h"
#include "application.h"
#include "button.h"
#include "led/circular_strip.h"
#include <esp_log.h>
#include <driver/spi_common.h>
#include "ssd1306.h"
#include "mcp_mimo.h"
#include "adc_battery_monitor.h"

#define TAG "DeskbotMimoBoard"

class DeskbotMimoBoard : public WifiBoard {
private:
    ssd1306_t oled_dev_;
    Display* display_ = nullptr;
    Button boot_button_;
    AudioCodec* audio_codec_ = nullptr;
    Led* led_ = nullptr;
    McpMimo* mcp_ = nullptr;
    AdcBatteryMonitor* adc_battery_monitor_ = nullptr;

    void InitializeBatteryMonitor() {
        // Gunakan resistor 100k dan 100k, ADC_UNIT_1_CH9 ada di GPIO 10
        adc_battery_monitor_ = new AdcBatteryMonitor(ADC_UNIT_1, ADC_CHANNEL_9, 100000, 100000, GPIO_NUM_NC);
    }

    void InitializeSsd1306Display() {
        ESP_LOGI(TAG, "Initializing custom SSD1306 driver...");
        esp_err_t ret = ssd1306_init(&oled_dev_, SPI2_HOST, 
                                     DESKBOT_MIMO_CONFIG.display_mosi_pin, 
                                     DESKBOT_MIMO_CONFIG.display_clk_pin, 
                                     DESKBOT_MIMO_CONFIG.display_cs_pin, 
                                     DESKBOT_MIMO_CONFIG.display_dc_pin, 
                                     DESKBOT_MIMO_CONFIG.display_rst_pin);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize custom SSD1306 driver: %d", ret);
            return;
        }

        display_ = new MimoEmojiDisplay(&oled_dev_, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            auto state = app.GetDeviceState();
            if (state == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            
            if (state == kDeviceStateSpeaking) {
                app.AbortSpeaking(kAbortReasonNone);
            }
            app.StartListening();
        });
    }

    void InitializeAudioCodec() {
        audio_codec_ = new NoAudioCodecDuplex(16000, 16000,
            DESKBOT_MIMO_CONFIG.audio_i2s_gpio_bclk, DESKBOT_MIMO_CONFIG.audio_i2s_gpio_ws,
            DESKBOT_MIMO_CONFIG.audio_i2s_gpio_dout, DESKBOT_MIMO_CONFIG.audio_i2s_gpio_din);
        audio_codec_->SetInputGain(4.0f);
    }

    void InitializeLed() {
        led_ = new CircularStrip(BUILTIN_LED_GPIO, BUILTIN_LED_COUNT);
    }

public:
    DeskbotMimoBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        mcp_ = new McpMimo(DESKBOT_MIMO_CONFIG); // DO THIS FIRST!
        mcp_->RegisterTools();
        
        InitializeSsd1306Display();
        InitializeAudioCodec();
        InitializeLed();
        InitializeButtons();
        InitializeBatteryMonitor();
    }

    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
        charging = adc_battery_monitor_->IsCharging();
        discharging = adc_battery_monitor_->IsDischarging();
        level = adc_battery_monitor_->GetBatteryLevel();
        return true;
    }

    virtual AudioCodec* GetAudioCodec() override {
        return audio_codec_;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Led* GetLed() override {
        return led_;
    }
};

DECLARE_BOARD(DeskbotMimoBoard);
