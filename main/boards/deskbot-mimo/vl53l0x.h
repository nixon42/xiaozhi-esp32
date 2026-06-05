#ifndef VL53L0X_H
#define VL53L0X_H

#include "i2c_device.h"

class Vl53l0x : public I2cDevice {
public:
    Vl53l0x(i2c_master_bus_handle_t i2c_bus, uint8_t addr = 0x29);
    bool Initialize();
    uint16_t ReadDistance();

private:
    bool WaitForRevision();
    void DataInit();
    void StaticInit();
    void SetSequenceStepEnable(uint8_t steps);
    void PerformRefCalibration();
    uint16_t ReadDistanceMillimeters();
    uint8_t stop_variable_;
};

#endif // VL53L0X_H
