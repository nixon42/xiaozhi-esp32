#ifndef _ADC_I2S_AUDIO_CODEC_H
#define _ADC_I2S_AUDIO_CODEC_H

#include "audio/audio_codec.h"
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>

class AdcI2sAudioCodec : public AudioCodec {
private:
    esp_codec_dev_handle_t output_dev_ = nullptr;
    esp_codec_dev_handle_t input_dev_ = nullptr;
    i2s_chan_handle_t tx_handle_ = nullptr;
    bool input_dev_opened_ = false; // ADC is opened once and never closed

    static constexpr int FILTER_SIZE = 8;
    int32_t filter_buf_[FILTER_SIZE] = {0};
    int32_t running_sum_ = 0;
    int filter_idx_ = 0;

    int32_t dc_offset_ = 0; // Fixed-point 16.16
    int32_t alpha_fp_ = 3276; // Approx 0.05 in 16-bit fixed point (0.05 * 65536)

    // Hysteresis Noise Gate & Ramping
    bool gate_open_ = false;
    float gate_gain_ = 0.0f;
    int16_t open_threshold_ = 1000;  // Open threshold (adjust as needed)
    int16_t close_threshold_ = 600;  // Close threshold (hysteresis)
    float attack_step_ = 0.02f;      // Speed of fade-in
    float release_step_ = 0.01f;     // Speed of fade-out

    virtual int Read(int16_t* dest, int samples) override;
    virtual int Write(const int16_t* data, int samples) override;

public:
    AdcI2sAudioCodec(int input_sample_rate, int output_sample_rate,
        uint32_t adc_mic_channel, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout);
    virtual ~AdcI2sAudioCodec();

    virtual void SetOutputVolume(int volume) override;
    virtual void EnableInput(bool enable) override;
    virtual void EnableOutput(bool enable) override;
};

#endif // _ADC_I2S_AUDIO_CODEC_H
