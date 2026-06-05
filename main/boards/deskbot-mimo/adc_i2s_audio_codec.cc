#include "adc_i2s_audio_codec.h"
#include <esp_log.h>
#include <esp_check.h>
#include <cstring>
#include <cstdlib>
#include <driver/adc.h>
#include <esp_adc/adc_continuous.h>
#include <audio_codec_data_if.h>
#include <esp_codec_adc.h>

static const char* TAG = "AdcI2sAudioCodec";

AdcI2sAudioCodec::AdcI2sAudioCodec(int input_sample_rate, int output_sample_rate,
    uint32_t adc_mic_channel, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout) {
    
    input_sample_rate_ = input_sample_rate;
    output_sample_rate_ = output_sample_rate;
    duplex_ = true; // Prevent power manager from independently toggling ADC

    // 1. Initialize ADC Input
    audio_codec_adc_cfg_t adc_cfg = {};
    adc_cfg.continuous_cfg.max_store_buf_size = 1024 * 4;
    adc_cfg.continuous_cfg.conv_frame_size = 1024;
    adc_cfg.continuous_cfg.sample_freq_hz = (uint32_t)input_sample_rate;
    adc_cfg.continuous_cfg.conv_mode = ADC_CONV_SINGLE_UNIT_1;
    adc_cfg.continuous_cfg.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;
    adc_cfg.continuous_cfg.pattern_num = 1;
    adc_cfg.continuous_cfg.cfg_mode = AUDIO_CODEC_ADC_CFG_MODE_SINGLE_UNIT;
    adc_cfg.continuous_cfg.cfg.single_unit.unit_id = ADC_UNIT_1;
    adc_cfg.continuous_cfg.cfg.single_unit.atten = ADC_ATTEN_DB_12;
    adc_cfg.continuous_cfg.cfg.single_unit.bit_width = ADC_BITWIDTH_12;
    adc_cfg.continuous_cfg.cfg.single_unit.channel_id[0] = (uint8_t)adc_mic_channel;
    
    const audio_codec_data_if_t* adc_data_if = audio_codec_new_adc_data(&adc_cfg);
    
    esp_codec_dev_cfg_t input_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .data_if = adc_data_if,
    };
    input_dev_ = esp_codec_dev_new(&input_dev_cfg);

    // 2. Initialize I2S Output
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle_, nullptr));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG((uint32_t)output_sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = bclk,
            .ws = ws,
            .dout = dout,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    std_cfg.slot_cfg.ws_width = I2S_DATA_BIT_WIDTH_32BIT;
    std_cfg.slot_cfg.bit_shift = true;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle_, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle_));

    audio_codec_i2s_cfg_t i2s_data_cfg = {
        .port = I2S_NUM_0,
        .rx_handle = nullptr,
        .tx_handle = tx_handle_,
    };
    const audio_codec_data_if_t* i2s_data_if = audio_codec_new_i2s_data(&i2s_data_cfg);

    esp_codec_dev_cfg_t output_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .data_if = i2s_data_if,
    };
    output_dev_ = esp_codec_dev_new(&output_dev_cfg);

    ESP_LOGI(TAG, "AdcI2sAudioCodec initialized (ADC Mic Channel: %d)", (int)adc_mic_channel);
}

AdcI2sAudioCodec::~AdcI2sAudioCodec() {
    if (input_dev_) {
        esp_codec_dev_close(input_dev_);
        esp_codec_dev_delete(input_dev_);
    }
    if (output_dev_) {
        esp_codec_dev_close(output_dev_);
        esp_codec_dev_delete(output_dev_);
    }
    if (tx_handle_) {
        i2s_channel_disable(tx_handle_);
        i2s_del_channel(tx_handle_);
    }
}

void AdcI2sAudioCodec::SetOutputVolume(int volume) {
    if (output_dev_) {
        esp_codec_dev_set_out_vol(output_dev_, volume);
    }
    AudioCodec::SetOutputVolume(volume);
}

void AdcI2sAudioCodec::EnableInput(bool enable) {
    if (enable == input_enabled_) return;
    if (enable && !input_dev_opened_) {
        // Open the ADC device only once — never close it to avoid
        // race condition with the audio processor task.
        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 1,
            .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0),
            .sample_rate = (uint32_t)input_sample_rate_,
        };
        if (esp_codec_dev_open(input_dev_, &fs) == ESP_OK) {
            input_dev_opened_ = true;
        } else {
            ESP_LOGE(TAG, "Failed to open input codec device");
            return;
        }
    }
    // Just update the flag — ADC keeps running in background
    AudioCodec::EnableInput(enable);
}

void AdcI2sAudioCodec::EnableOutput(bool enable) {
    if (enable == output_enabled_) return;
    if (enable) {
        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 1,
            .channel_mask = 0,
            .sample_rate = (uint32_t)output_sample_rate_,
        };
        ESP_ERROR_CHECK(esp_codec_dev_open(output_dev_, &fs));
        ESP_ERROR_CHECK(esp_codec_dev_set_out_vol(output_dev_, output_volume_));
    } else {
        ESP_ERROR_CHECK(esp_codec_dev_close(output_dev_));
    }
    AudioCodec::EnableOutput(enable);
}

int AdcI2sAudioCodec::Read(int16_t* dest, int samples) {
    if (!input_dev_opened_ || !input_dev_) {
        vTaskDelay(pdMS_TO_TICKS(10));
        std::memset(dest, 0, samples * sizeof(int16_t));
        return samples;
    }

    // Always read from ADC to maintain timing (blocking call)
    esp_err_t err = esp_codec_dev_read(input_dev_, (void*)dest, samples * sizeof(int16_t));
    if (err != ESP_OK) {
        // If read fails (timeout or error), delay slightly to avoid high-CPU spin
        vTaskDelay(1);
        std::memset(dest, 0, samples * sizeof(int16_t));
        return samples;
    }

    if (!input_enabled_) {
        std::memset(dest, 0, samples * sizeof(int16_t));
        return samples;
    }

    // Single pass for: Filter, Gain, DC Removal, AND Peak Detection
    int16_t peak = 0;
    for (int i = 0; i < samples; i++) {
        // 1. Raw extraction (assuming TYPE2 12-bit)
        uint32_t raw_val = (uint32_t)dest[i] & 0x0FFF;
        int32_t voltage = (raw_val * 3300) / 4095;
        int32_t sample = voltage - 1650;

        // 2. Moving Average
        running_sum_ = running_sum_ - filter_buf_[filter_idx_] + sample;
        filter_buf_[filter_idx_] = sample;
        filter_idx_ = (filter_idx_ + 1) % FILTER_SIZE;
        sample = running_sum_ / FILTER_SIZE;
        
        // 3. Gain & Fixed-Point Alpha-DC Removal
        int32_t current_sample = (int32_t)(sample * input_gain_);
        
        // dc_offset_ += (current_sample - dc_offset_) * alpha
        // Use 16.16 fixed point for dc_offset_
        int32_t diff = (current_sample << 16) - dc_offset_;
        dc_offset_ += (int32_t)(((int64_t)diff * alpha_fp_) >> 16);
        sample = current_sample - (dc_offset_ >> 16);
        
        // 4. Clamp
        if (sample > 32767) sample = 32767;
        else if (sample < -32768) sample = -32768;
        
        dest[i] = (int16_t)sample;

        // 5. Build peak for gate logic
        int16_t abs_val = (dest[i] < 0) ? -dest[i] : dest[i];
        if (abs_val > peak) peak = abs_val;
    }

    // 6. Update Hysteresis Noise Gate
    if (!gate_open_ && peak > open_threshold_) {
        gate_open_ = true;
    } else if (gate_open_ && peak < close_threshold_) {
        gate_open_ = false;
    }

    // 7. Apply Gain Ramping (Fade-in/out)
    float target_gain = gate_open_ ? 1.0f : 0.0f;
    for (int i = 0; i < samples; i++) {
        if (gate_gain_ < target_gain) {
            gate_gain_ += attack_step_;
            if (gate_gain_ > target_gain) gate_gain_ = target_gain;
        } else if (gate_gain_ > target_gain) {
            gate_gain_ -= release_step_;
            if (gate_gain_ < target_gain) gate_gain_ = target_gain;
        }
        dest[i] = (int16_t)(dest[i] * gate_gain_);
    }

    return samples;
}

int AdcI2sAudioCodec::Write(const int16_t* data, int samples) {
    if (output_enabled_) {
        esp_codec_dev_write(output_dev_, (void*)data, samples * sizeof(int16_t));
    }
    return samples;
}
