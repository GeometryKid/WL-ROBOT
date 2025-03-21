#ifndef _CHASSISTASK_H
#define _CHASSISTASK_H

// 机器人控制头文件
#include <MPU6050_tockn.h>
#include "Servo_STS3032.h"
#include <SimpleFOC.h>
#include <Arduino.h>
#include "robot.h"

extern SMS_STS sms_sts;
extern TwoWire I2Cone;
extern TwoWire I2Ctwo;
extern MagneticSensorI2C sensor1;
extern MagneticSensorI2C sensor2;
extern MPU6050 mpu6050;
extern BLDCMotor motor1;
extern BLDCMotor motor2;
extern BLDCDriver3PWM driver1;
extern BLDCDriver3PWM driver2;
extern Commander command;
extern PIDController pid_angle;
extern PIDController pid_gyro;
extern PIDController pid_distance;
extern PIDController pid_speed;
extern PIDController pid_yaw_angle;
extern PIDController pid_yaw_gyro;
extern PIDController pid_lqr_u;
extern PIDController pid_zeropoint;
extern PIDController pid_roll_angle;
extern LowPassFilter lpf_joyy;
extern LowPassFilter lpf_zeropoint;
extern LowPassFilter lpf_roll;

// 腿部舵机控制数据
extern byte ID[2];
extern s16 Position[2];
extern u16 Speed[2];
extern byte ACC[2];

class ChassisTask
{
  public:
    void startTask();
    void Chassis_loop();
};

extern ChassisTask WLRobot;

void lqr_balance_loop();
void yaw_loop();
void leg_loop();
void jump_loop();
void yaw_angle_addup();
void StabAngle(char *cmd);
void StabGyro(char *cmd);
void StabDistance(char *cmd);
void StabSpeed(char *cmd);
void StabYawAngle(char *cmd);
void StabYawGyro(char *cmd);
void lpfJoyy(char *cmd);
void StabLqrU(char *cmd);
void StabZeropoint(char *cmd);
void lpfZeropoint(char *cmd);
void StabRollAngle(char *cmd);
void lpfRoll(char *cmd);

// void Stabtest_zeropoint(char* cmd) { command.pid(&test_zeropoint, cmd); }


#endif