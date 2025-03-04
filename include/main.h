#ifndef _MAIN_H
#define _MAIN_H

// 机器人控制头文件
#include <MPU6050_tockn.h>
#include "Servo_STS3032.h"
#include <SimpleFOC.h>
#include <Arduino.h>
#include "freertos_task.h"

void adc_calibration_init();
void bat_check();
void web_loop();
void lqr_balance_loop();
void yaw_loop();
void leg_loop();
void jump_loop();
void yaw_angle_addup();

#endif
