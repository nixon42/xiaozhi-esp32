#ifndef MCP_MIMO_H
#define MCP_MIMO_H

#include "mcp_server.h"
#include "motor_driver.h"
#include "config.h"
#include "vl53l0x.h"
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class McpMimo {
public:
    McpMimo(const HardwareConfig& config);
    ~McpMimo();
    void RegisterTools();

private:
    MotorDriver* driver_ = nullptr;
    i2c_master_bus_handle_t i2c_bus_ = nullptr;
    Vl53l0x* sensor_ = nullptr;
    
    TaskHandle_t action_task_handle_ = nullptr;
    QueueHandle_t action_queue_;
    bool is_action_in_progress_ = false;

    enum ActionType {
        ACTION_FORWARD = 1,
        ACTION_BACKWARD = 2,
        ACTION_TURN_LEFT = 3,
        ACTION_TURN_RIGHT = 4
    };

    struct MimoActionParams {
        int action_type;
        int steps;
        int speed;
    };

    static void ActionTask(void* arg);
    void StartActionTaskIfNeeded();
    void QueueAction(int action_type, int steps, int speed);

    ReturnValue RobotAction(const PropertyList& properties);
    ReturnValue StopRobot(const PropertyList& properties);
    ReturnValue GetStatus(const PropertyList& properties);
    ReturnValue GetDistance(const PropertyList& properties);
    ReturnValue IsGroundStable(const PropertyList& properties);
    bool CheckGroundStability(uint16_t* distance_out = nullptr);
};

#endif // MCP_MIMO_H
