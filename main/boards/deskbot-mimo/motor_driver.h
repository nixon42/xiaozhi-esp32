#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

class MotorDriver {
public:
    virtual ~MotorDriver() = default;
    virtual void Move(int speed_left, int speed_right) = 0;
    virtual void Stop() = 0;
};

#endif // MOTOR_DRIVER_H
