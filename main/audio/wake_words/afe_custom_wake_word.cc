#include "afe_custom_wake_word.h"
#include "audio_service.h"
#include "assets.h"

#include <esp_log.h>
#include <esp_mn_speech_commands.h>
#include <cJSON.h>

#define DETECTION_RUNNING_EVENT 1
#define TAG "AfeCustomWakeWord"

AfeCustomWakeWord::AfeCustomWakeWord()
    : afe_data_(nullptr), multinet_model_data_(nullptr) {
    event_group_ = xEventGroupCreate();
}

AfeCustomWakeWord::~AfeCustomWakeWord() {
    Stop();
    if (afe_data_ != nullptr) {
        afe_iface_->destroy(afe_data_);
    }
    if (multinet_model_data_ != nullptr && multinet_ != nullptr) {
        multinet_->destroy(multinet_model_data_);
    }
    if (wake_word_encode_task_stack_ != nullptr) {
        heap_caps_free(wake_word_encode_task_stack_);
    }
    if (wake_word_encode_task_buffer_ != nullptr) {
        heap_caps_free(wake_word_encode_task_buffer_);
    }
    if (models_ != nullptr) {
        esp_srmodel_deinit(models_);
    }
    vEventGroupDelete(event_group_);
}

void AfeCustomWakeWord::ParseWakenetModelConfig() {
    auto& assets = Assets::GetInstance();
    void* ptr = nullptr;
    size_t size = 0;
    if (!assets.GetAssetData("index.json", ptr, size)) {
        ESP_LOGE(TAG, "Failed to read index.json");
        return;
    }
    cJSON* root = cJSON_ParseWithLength(static_cast<char*>(ptr), size);
    if (root == nullptr) {
        ESP_LOGE(TAG, "Failed to parse index.json");
        return;
    }
    cJSON* multinet_model = cJSON_GetObjectItem(root, "multinet_model");
    if (cJSON_IsObject(multinet_model)) {
        cJSON* language = cJSON_GetObjectItem(multinet_model, "language");
        cJSON* duration = cJSON_GetObjectItem(multinet_model, "duration");
        cJSON* threshold = cJSON_GetObjectItem(multinet_model, "threshold");
        cJSON* commands = cJSON_GetObjectItem(multinet_model, "commands");
        if (cJSON_IsString(language)) language_ = language->valuestring;
        if (cJSON_IsNumber(duration)) duration_ = duration->valueint;
        if (cJSON_IsNumber(threshold)) threshold_ = threshold->valuedouble;
        if (cJSON_IsArray(commands)) {
            for (int i = 0; i < cJSON_GetArraySize(commands); i++) {
                cJSON* command = cJSON_GetArrayItem(commands, i);
                if (cJSON_IsObject(command)) {
                    cJSON* command_name = cJSON_GetObjectItem(command, "command");
                    cJSON* text = cJSON_GetObjectItem(command, "text");
                    cJSON* action = cJSON_GetObjectItem(command, "action");
                    if (cJSON_IsString(command_name) && cJSON_IsString(text) && cJSON_IsString(action)) {
                        commands_.push_back({command_name->valuestring, text->valuestring, action->valuestring});
                        ESP_LOGI(TAG, "Command: %s, Text: %s, Action: %s", command_name->valuestring, text->valuestring, action->valuestring);
                    }
                }
            }
        }
    }
    cJSON_Delete(root);
}

bool AfeCustomWakeWord::Initialize(AudioCodec* codec, srmodel_list_t* models_list) {
    codec_ = codec;
    commands_.clear();

    if (models_list == nullptr) {
        language_ = "cn";
        models_ = esp_srmodel_init("model");
#ifdef CONFIG_CUSTOM_WAKE_WORD
        threshold_ = CONFIG_CUSTOM_WAKE_WORD_THRESHOLD / 100.0f;
        commands_.push_back({CONFIG_CUSTOM_WAKE_WORD, CONFIG_CUSTOM_WAKE_WORD_DISPLAY, "wake"});
#endif
    } else {
        models_ = models_list;
        ParseWakenetModelConfig();
    }

    if (models_ == nullptr || models_->num == -1) {
        ESP_LOGE(TAG, "Failed to initialize models");
        return false;
    }

    // Initialize MultiNet
    mn_name_ = esp_srmodel_filter(models_, ESP_MN_PREFIX, language_.c_str());
    if (mn_name_ == nullptr) {
        mn_name_ = esp_srmodel_filter(models_, ESP_MN_PREFIX, NULL);
    }
    if (mn_name_ == nullptr) {
        ESP_LOGE(TAG, "MultiNet model not found");
        return false;
    }

    multinet_ = esp_mn_handle_from_name(mn_name_);
    multinet_model_data_ = (model_iface_data_t*)multinet_->create(mn_name_, duration_);
    multinet_->set_det_threshold(multinet_model_data_, threshold_);
    esp_mn_commands_clear();
    for (int i = 0; i < commands_.size(); i++) {
        esp_mn_commands_add(i+1, commands_[i].command.c_str());
    }
    esp_mn_commands_update();

    // Initialize AFE with AEC for Barge-in
    int ref_num = codec_->input_reference() ? 1 : 0;
    std::string input_format;
    for (int i = 0; i < codec_->input_channels() - ref_num; i++) input_format.push_back('M');
    for (int i = 0; i < ref_num; i++) input_format.push_back('R');

    afe_config_t* afe_config = afe_config_init(input_format.c_str(), models_, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);
    afe_config->aec_init = codec_->input_reference();
    afe_config->aec_mode = AEC_MODE_SR_HIGH_PERF;
    
    // Only use NSNet if the model is explicitly found, otherwise use basic NS to save resources
    char* ns_model_name = esp_srmodel_filter(models_, ESP_NSNET_PREFIX, NULL);
    if (ns_model_name != nullptr) {
        afe_config->ns_init = true;
        afe_config->ns_model_name = ns_model_name;
        afe_config->afe_ns_mode = AFE_NS_MODE_NET;
    } else {
        afe_config->ns_init = true; // Use basic NS if model not found
    }
    
    afe_config->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;
    afe_config->afe_perferred_core = 1;
    afe_config->afe_perferred_priority = 5;
    
    afe_iface_ = esp_afe_handle_from_config(afe_config);
    afe_data_ = (esp_afe_sr_data_t*)afe_iface_->create_from_config(afe_config);

    xTaskCreatePinnedToCore([](void* arg) {
        auto this_ = (AfeCustomWakeWord*)arg;
        this_->AudioDetectionTask();
        vTaskDelete(NULL);
    }, "afe_custom_wn", 8192, this, 10, nullptr, 0); // Core 0, higher priority

    return true;
}

void AfeCustomWakeWord::OnWakeWordDetected(std::function<void(const std::string& wake_word)> callback) {
    wake_word_detected_callback_ = callback;
}

void AfeCustomWakeWord::Start() {
    running_ = true;
    xEventGroupSetBits(event_group_, DETECTION_RUNNING_EVENT);
}

void AfeCustomWakeWord::Stop() {
    running_ = false;
    xEventGroupClearBits(event_group_, DETECTION_RUNNING_EVENT);
    std::lock_guard<std::mutex> lock(input_buffer_mutex_);
    if (afe_data_ != nullptr) afe_iface_->reset_buffer(afe_data_);
    input_buffer_.clear();
}

void AfeCustomWakeWord::Feed(const std::vector<int16_t>& data) {
    if (afe_data_ == nullptr || !running_) return;

    std::lock_guard<std::mutex> lock(input_buffer_mutex_);
    input_buffer_.insert(input_buffer_.end(), data.begin(), data.end());
    size_t chunk_size = afe_iface_->get_feed_chunksize(afe_data_) * codec_->input_channels();
    while (input_buffer_.size() >= chunk_size) {
        afe_iface_->feed(afe_data_, input_buffer_.data());
        input_buffer_.erase(input_buffer_.begin(), input_buffer_.begin() + chunk_size);
    }
}

size_t AfeCustomWakeWord::GetFeedSize() {
    return afe_data_ ? afe_iface_->get_feed_chunksize(afe_data_) : 0;
}

const std::string& AfeCustomWakeWord::GetLastDetectedWakeWord() const {
    return last_detected_wake_word_;
}

void AfeCustomWakeWord::AudioDetectionTask() {
    auto fetch_size = afe_iface_->get_fetch_chunksize(afe_data_);
    ESP_LOGI(TAG, "AFE+MultiNet task started, fetch size: %d", fetch_size);

    while (true) {
        xEventGroupWaitBits(event_group_, DETECTION_RUNNING_EVENT, pdFALSE, pdTRUE, portMAX_DELAY);

        auto res = afe_iface_->fetch_with_delay(afe_data_, portMAX_DELAY);
        if (res == nullptr || res->ret_value == ESP_FAIL || !running_) continue;

        StoreWakeWordData(res->data, res->data_size / sizeof(int16_t));

        // Feed cleaned audio to MultiNet
        esp_mn_state_t mn_state = multinet_->detect(multinet_model_data_, res->data);
        if (mn_state == ESP_MN_STATE_DETECTED) {
            esp_mn_results_t *mn_result = multinet_->get_results(multinet_model_data_);
            for (int i = 0; i < mn_result->num && running_; i++) {
                ESP_LOGI(TAG, "MultiNet Detected: id=%d, prob=%f", mn_result->command_id[i], mn_result->prob[i]);
                auto& command = commands_[mn_result->command_id[i] - 1];
                if (command.action == "wake") {
                    last_detected_wake_word_ = command.text;
                    Stop();
                    if (wake_word_detected_callback_) wake_word_detected_callback_(last_detected_wake_word_);
                }
            }
            multinet_->clean(multinet_model_data_);
        } else if (mn_state == ESP_MN_STATE_TIMEOUT) {
            multinet_->clean(multinet_model_data_);
        }
    }
}

void AfeCustomWakeWord::StoreWakeWordData(const int16_t* data, size_t samples) {
    std::lock_guard<std::mutex> lock(wake_word_mutex_);
    wake_word_pcm_.emplace_back(std::vector<int16_t>(data, data + samples));
    while (wake_word_pcm_.size() > 2000 / 32) wake_word_pcm_.pop_front();
}

void AfeCustomWakeWord::EncodeWakeWordData() {
    const size_t stack_size = 4096 * 6;
    wake_word_opus_.clear();
    if (wake_word_encode_task_stack_ == nullptr) {
        wake_word_encode_task_stack_ = (StackType_t*)heap_caps_malloc(stack_size, MALLOC_CAP_SPIRAM);
    }
    if (wake_word_encode_task_buffer_ == nullptr) {
        wake_word_encode_task_buffer_ = (StaticTask_t*)heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);
    }

    wake_word_encode_task_ = xTaskCreateStaticPinnedToCore([](void* arg) {
        auto this_ = (AfeCustomWakeWord*)arg;
        {
            esp_opus_enc_config_t opus_enc_cfg = AS_OPUS_ENC_CONFIG();
            void* encoder_handle = nullptr;
            esp_opus_enc_open(&opus_enc_cfg, sizeof(esp_opus_enc_config_t), &encoder_handle);
            if (!encoder_handle) return;
            
            int frame_size, outbuf_size;
            esp_opus_enc_get_frame_size(encoder_handle, &frame_size, &outbuf_size);
            frame_size /= sizeof(int16_t);
            
            std::vector<int16_t> in_buffer;
            for (auto& pcm : this_->wake_word_pcm_) {
                in_buffer.insert(in_buffer.end(), pcm.begin(), pcm.end());
                while (in_buffer.size() >= frame_size) {
                    std::vector<uint8_t> opus_buf(outbuf_size);
                    esp_audio_enc_in_frame_t in = {(uint8_t*)in_buffer.data(), (uint32_t)(frame_size * sizeof(int16_t))};
                    esp_audio_enc_out_frame_t out = {opus_buf.data(), (uint32_t)outbuf_size, 0};
                    if (esp_opus_enc_process(encoder_handle, &in, &out) == ESP_AUDIO_ERR_OK) {
                        std::lock_guard<std::mutex> lock(this_->wake_word_mutex_);
                        this_->wake_word_opus_.emplace_back(opus_buf.data(), opus_buf.data() + out.encoded_bytes);
                        this_->wake_word_cv_.notify_all();
                    }
                    in_buffer.erase(in_buffer.begin(), in_buffer.begin() + frame_size);
                }
            }
            this_->wake_word_pcm_.clear();
            esp_opus_enc_close(encoder_handle);
            std::lock_guard<std::mutex> lock(this_->wake_word_mutex_);
            this_->wake_word_opus_.push_back({});
            this_->wake_word_cv_.notify_all();
        }
        vTaskDelete(NULL);
    }, "enc_wn", stack_size, this, 2, wake_word_encode_task_stack_, wake_word_encode_task_buffer_, 1);
}

bool AfeCustomWakeWord::GetWakeWordOpus(std::vector<uint8_t>& opus) {
    std::unique_lock<std::mutex> lock(wake_word_mutex_);
    wake_word_cv_.wait(lock, [this]() { return !wake_word_opus_.empty(); });
    opus.swap(wake_word_opus_.front());
    wake_word_opus_.pop_front();
    return !opus.empty();
}
