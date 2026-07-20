#include "mcp_mimo.h"
#include "tb6612fng.h"
#include "drv8833.h"
#include <esp_log.h>
#include <driver/i2c_master.h>
#include "application.h"

#define TAG "McpMimo"

#define STEP_DURATION_MS  500
#define STEP_GAP_MS       50

#define SPEED_SLOW   20
#define SPEED_NORMAL 40
#define SPEED_FAST   70

McpMimo::McpMimo(const HardwareConfig& config) {
#if DESKBOT_MOTOR_TYPE == MOTOR_TYPE_TB6612FNG
    ESP_LOGI(TAG, "Initializing TB6612FNG Motor Driver");
    driver_ = new Tb6612fngDriver(config.motor_pwma_pin, config.motor_ain1_pin, config.motor_ain2_pin,
                                  config.motor_pwmb_pin, config.motor_bin1_pin, config.motor_bin2_pin,
                                  config.motor_stby_pin);
#elif DESKBOT_MOTOR_TYPE == MOTOR_TYPE_DRV8833
    ESP_LOGI(TAG, "Initializing DRV8833 Motor Driver");
    // DRV8833 doesn't use STBY, and we map AIN1, AIN2, BIN1, BIN2
    // We repurpose the pins from config: pwma->AIN1, ain1->AIN2, pwmb->BIN1, bin1->BIN2
    driver_ = new Drv8833Driver(config.motor_pwma_pin, config.motor_ain1_pin, 
                                config.motor_pwmb_pin, config.motor_bin1_pin,
                                config.motor_stby_pin);
#endif
    action_queue_ = xQueueCreate(10, sizeof(MimoActionParams));

    // Initialize I2C for VL53L0X
    if (config.i2c_sda_pin != GPIO_NUM_NC && config.i2c_scl_pin != GPIO_NUM_NC) {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = config.i2c_sda_pin,
            .scl_io_num = config.i2c_scl_pin,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_LOGI(TAG, "Initializing I2C SDA:%d SCL:%d", config.i2c_sda_pin, config.i2c_scl_pin);
        esp_err_t ret = i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_);
        if (ret == ESP_OK) {
            // Scanner
            for (uint8_t i = 1; i < 127; i++) {
                if (i2c_master_probe(i2c_bus_, i, 100) == ESP_OK) {
                    ESP_LOGI(TAG, "I2C device found at address 0x%02X", i);
                }
            }
            ESP_LOGI(TAG, "I2C bus initialized on SDA:%d SCL:%d", config.i2c_sda_pin, config.i2c_scl_pin);
            sensor_ = new Vl53l0x(i2c_bus_);
            if (!sensor_->Initialize()) {
                ESP_LOGE(TAG, "Failed to initialize VL53L0X sensor");
            }
        } else {
            ESP_LOGE(TAG, "Failed to initialize I2C bus: %d", ret);
        }
    }
}

McpMimo::~McpMimo() {
    if (action_task_handle_ != nullptr) {
        vTaskDelete(action_task_handle_);
        action_task_handle_ = nullptr;
    }
    if (action_queue_ != nullptr) {
        vQueueDelete(action_queue_);
    }
    if (driver_ != nullptr) {
        delete driver_;
    }
    if (sensor_ != nullptr) {
        delete sensor_;
    }
    if (i2c_bus_ != nullptr) {
        i2c_del_master_bus(i2c_bus_);
    }
}

void McpMimo::RegisterTools() {
    auto& server = McpServer::GetInstance();

    // Unified Action tool
    PropertyList action_props;
    action_props.AddProperty(Property("action", kPropertyTypeString, "forward"));
    action_props.AddProperty(Property("steps", kPropertyTypeInteger, 3, 3, 20));
    action_props.AddProperty(Property("speed", kPropertyTypeInteger, 2, 1, 3));

    server.AddTool("self.mimo.action", "Gerakkan robot. action: arah gerakan (forward, backward, turn_left, turn_right), steps: jumlah langkah (1-20), speed: kecepatan (1=pelan, 2=normal, 3=cepat)", action_props,
        std::bind(&McpMimo::RobotAction, this, std::placeholders::_1));

    // Emergency Stop tool
    // PropertyList stop_props;
    // server.AddTool("self.mimo.stop", "Hentikan semua gerakan dan kosongkan antrian seketika.", stop_props,
    //     std::bind(&McpMimo::StopRobot, this, std::placeholders::_1));
        
    // Status tool
    // PropertyList status_props;
    // server.AddTool("self.mimo.get_status", "Dapatkan status robot saat ini (moving atau idle).", status_props,
    //     std::bind(&McpMimo::GetStatus, this, std::placeholders::_1));

    // Distance sensor tool
    // PropertyList distance_props;
    // server.AddTool("self.mimo.get_distance", "Raw value sensor jarak (mm) di bawah robot.", distance_props,
    //     std::bind(&McpMimo::GetDistance, this, std::placeholders::_1));

    // Ground stability check tool
    PropertyList ground_props;
    server.AddTool("self.mimo.is_ground_stable", "Cek apakah ada permukaan meja yang stabil di bawah depan robot (untuk pencegahan jatuh).", ground_props,
        std::bind(&McpMimo::IsGroundStable, this, std::placeholders::_1));

    ESP_LOGI(TAG, "Motor control tools registered (Redesign)");
}

void McpMimo::ActionTask(void* arg) {
    McpMimo* controller = static_cast<McpMimo*>(arg);
    MimoActionParams params;

    while (true) {
        if (xQueueReceive(controller->action_queue_, &params, pdMS_TO_TICKS(1000)) == pdTRUE) {
            ESP_LOGI(TAG, "Executing Action: type=%d, steps=%d, speed=%d", params.action_type, params.steps, params.speed);
            controller->is_action_in_progress_ = true;

            int duty = SPEED_NORMAL;
            if (params.speed == 1) duty = SPEED_SLOW;
            else if (params.speed == 3) duty = SPEED_FAST;

            bool is_turning = (params.action_type == ACTION_TURN_LEFT || params.action_type == ACTION_TURN_RIGHT);
            if (is_turning) {
                duty = 100;
            }

            int left_speed = 0;
            int right_speed = 0;

            switch (params.action_type) {
                case ACTION_FORWARD:
                    left_speed = duty; right_speed = duty;
                    break;
                case ACTION_BACKWARD:
                    left_speed = -duty; right_speed = -duty;
                    break;
                case ACTION_TURN_LEFT:
                    left_speed = -duty; right_speed = duty;
                    break;
                case ACTION_TURN_RIGHT:
                    left_speed = duty; right_speed = -duty;
                    break;
            }

            int total_time_ms = params.steps * STEP_DURATION_MS;
            if (is_turning) {
                total_time_ms = (int)(total_time_ms * 2.0f);
            }

#define RAMP_TIME_MS      600
#define UPDATE_INTERVAL   10

            int current_ramp_time = RAMP_TIME_MS;
            if (total_time_ms < current_ramp_time * 2) {
                current_ramp_time = total_time_ms / 2;
            }

            // ESP_LOGI(TAG, "Smooth Move: %dms, ramping: %dms, base duty: %d", total_time_ms, current_ramp_time, duty);
            // ESP_LOGI(TAG, "Targets: L=%d, R=%d", left_speed, right_speed);

            for (int elapsed = 0; elapsed < total_time_ms; elapsed += UPDATE_INTERVAL) {
                float factor = 1.0f;
                bool is_turning = (params.action_type == ACTION_TURN_LEFT || params.action_type == ACTION_TURN_RIGHT);

                if (is_turning) {
                    // HAPUS SOFT-START: Berikan tendangan 100% instan untuk skid steering 
                    // agar tidak nyangkut (stall) yang menyebabkan DRV8833 mati karena Over-Current.
                    factor = 1.0f;
                    
                    // Opsional: kita tetap beri ramp-down di akhir agar berhentinya mulus
                    if (elapsed > total_time_ms - current_ramp_time) {
                        float p = (float)(total_time_ms - elapsed) / current_ramp_time;
                        factor = 0.6f + (p * p * 0.4f);
                    }
                } else {
                    // Quadratic ramping from 0 for forward/backward
                    if (elapsed < current_ramp_time) {
                        float p = (float)elapsed / current_ramp_time;
                        factor = p * p;
                    } else if (elapsed > total_time_ms - current_ramp_time) {
                        float p = (float)(total_time_ms - elapsed) / current_ramp_time;
                        factor = p * p;
                    }
                }

                int cur_left = (int)(left_speed * factor);
                int cur_right = (int)(right_speed * factor);
                
                // Cap values
                auto cap = [](int v) { return v > 100 ? 100 : (v < -100 ? -100 : v); };
                cur_left = cap(cur_left);
                cur_right = cap(cur_right);

                controller->driver_->Move(cur_left, cur_right);

                // Cliff detection (Edge detection)
                // Check every 50ms, and skip the very first read to avoid motor startup noise
                if (controller->sensor_ != nullptr && params.action_type == ACTION_FORWARD && (elapsed >= 50)) {
                    uint16_t dist;
                    if (!controller->CheckGroundStability(&dist)) {
                        ESP_LOGW(TAG, "CLIFF DETECTED! Distance: %dmm. Emergency Escape!", dist);
                        
                        // Emergency Escape: Move backward at 100% speed for 1 step
                        controller->driver_->Move(-35, -35);
                        vTaskDelay(pdMS_TO_TICKS(STEP_DURATION_MS));
                        controller->driver_->Stop();
                        
                        controller->is_action_in_progress_ = false;
                        Application::GetInstance().Alert("Error", "Ada jurang! Mundur menyelamatkan diri.", "fear");
                        goto action_complete; 
                    }
                }

                vTaskDelay(pdMS_TO_TICKS(UPDATE_INTERVAL));
            }
            controller->driver_->Stop();
            controller->is_action_in_progress_ = false;
        }
action_complete:
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#define CLIFF_THRESHOLD_MM 200

bool McpMimo::CheckGroundStability(uint16_t* distance_out) {
    if (sensor_ == nullptr) return true; 
    uint16_t distance = sensor_->ReadDistance();
    if (distance_out) *distance_out = distance;
    ESP_LOGI(TAG, "Distance: %dmm", distance);
    return (distance < CLIFF_THRESHOLD_MM);
}

void McpMimo::StartActionTaskIfNeeded() {
    if (action_task_handle_ == nullptr) {
        xTaskCreate(ActionTask, "mimo_action", 1024 * 3, this, configMAX_PRIORITIES - 1,
                    &action_task_handle_);
    }
}

void McpMimo::QueueAction(int action_type, int steps, int speed) {
    MimoActionParams params = {action_type, steps, speed};
    xQueueSend(action_queue_, &params, portMAX_DELAY);
    StartActionTaskIfNeeded();
}

ReturnValue McpMimo::RobotAction(const PropertyList& properties) {
    std::string action = properties["action"].value<std::string>();
    int steps = properties["steps"].value<int>();
    int speed = properties["speed"].value<int>();

    int action_type = ACTION_FORWARD;
    if (action == "forward") action_type = ACTION_FORWARD;
    else if (action == "backward") action_type = ACTION_BACKWARD;
    else if (action == "turn_left") action_type = ACTION_TURN_LEFT;
    else if (action == "turn_right") action_type = ACTION_TURN_RIGHT;
    else return std::string("Error: Invalid action");

    // Pre-action ground stability check for all actions except backward
    if (action != "backward" && sensor_ != nullptr) {
        uint16_t distance;
        if (!CheckGroundStability(&distance)) {
            ESP_LOGW(TAG, "Action denied: CLIFF DETECTED! Distance: %dmm", distance);
            return std::string("Error: Ada jurang di depan (" + std::to_string(distance) + "mm)! Tidak bisa bergerak.");
        }
    }

    QueueAction(action_type, steps, speed);
    ESP_LOGI(TAG, "Tool Called: action=%s, type=%d, steps=%d, speed=%d", action.c_str(), action_type, steps, speed);
    return std::string("Action " + action + " queued");
}

ReturnValue McpMimo::StopRobot(const PropertyList& properties) {
    ESP_LOGI(TAG, "Stopping robot immediately");
    if (action_task_handle_ != nullptr) {
        vTaskDelete(action_task_handle_);
        action_task_handle_ = nullptr;
    }
    is_action_in_progress_ = false;
    xQueueReset(action_queue_);
    driver_->Stop();

    return std::string("Robot stopped");
}

ReturnValue McpMimo::GetStatus(const PropertyList& properties) {
    return is_action_in_progress_ ? std::string("moving") : std::string("idle");
}

ReturnValue McpMimo::GetDistance(const PropertyList& properties) {
    if (sensor_ == nullptr) {
        return std::string("Error: Jarak tidak aktif");
    }
    uint16_t distance = sensor_->ReadDistance();
    return std::to_string(distance) + " mm";
}

ReturnValue McpMimo::IsGroundStable(const PropertyList& properties) {
    if (sensor_ == nullptr) {
        return std::string("Error: Sensor tidak aktif");
    }
    uint16_t distance;
    if (CheckGroundStability(&distance)) {
        return std::string("Stable ground detected (distance: " + std::to_string(distance) + " mm)");
    } else {
        return std::string("CLIFF DETECTED! Ground is not stable (distance: " + std::to_string(distance) + " mm)");
    }
}
