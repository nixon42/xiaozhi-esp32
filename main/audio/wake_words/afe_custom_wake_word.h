#ifndef _AFE_CUSTOM_WAKE_WORD_H_
#define _AFE_CUSTOM_WAKE_WORD_H_

#include "wake_word.h"
#include "audio_codec.h"

#include <esp_afe_sr_iface.h>
#include <esp_afe_sr_models.h>
#include <esp_mn_iface.h>
#include <esp_mn_models.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <deque>
#include <condition_variable>

class AfeCustomWakeWord : public WakeWord {
public:
    AfeCustomWakeWord();
    virtual ~AfeCustomWakeWord();

    bool Initialize(AudioCodec* codec, srmodel_list_t* models_list) override;
    void Start() override;
    void Stop() override;
    void Feed(const std::vector<int16_t>& data) override;
    void OnWakeWordDetected(std::function<void(const std::string& wake_word)> callback) override;
    size_t GetFeedSize() override;
    const std::string& GetLastDetectedWakeWord() const override;

    void EncodeWakeWordData() override;
    bool GetWakeWordOpus(std::vector<uint8_t>& opus) override;

private:
    struct Command {
        std::string command;
        std::string text;
        std::string action;
    };

    AudioCodec* codec_;
    srmodel_list_t* models_ = nullptr;
    std::string language_ = "en";
    float threshold_ = 0.4f;
    int duration_ = 3000;
    std::vector<Command> commands_;
    std::string last_detected_wake_word_;
    std::function<void(const std::string& wake_word)> wake_word_detected_callback_;

    const esp_afe_sr_iface_t* afe_iface_ = nullptr;
    esp_afe_sr_data_t* afe_data_ = nullptr;
    const esp_mn_iface_t* multinet_ = nullptr;
    model_iface_data_t* multinet_model_data_ = nullptr;
    char* mn_name_ = nullptr;

    bool running_ = false;
    EventGroupHandle_t event_group_;
    std::vector<int16_t> input_buffer_;
    std::mutex input_buffer_mutex_;

    std::deque<std::vector<int16_t>> wake_word_pcm_;
    std::deque<std::vector<uint8_t>> wake_word_opus_;
    std::mutex wake_word_mutex_;
    std::condition_variable wake_word_cv_;
    TaskHandle_t wake_word_encode_task_ = nullptr;
    StackType_t* wake_word_encode_task_stack_ = nullptr;
    StaticTask_t* wake_word_encode_task_buffer_ = nullptr;

    void ParseWakenetModelConfig();
    void AudioDetectionTask();
    void StoreWakeWordData(const int16_t* data, size_t samples);
};

#endif
