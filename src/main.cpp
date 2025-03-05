// -----------------------------------------------------------------------------
// Copyright (c) 2024 Mu Shibo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// -----------------------------------------------------------------------------

// 转成PlatformIO工程创建
#include "main.h"

void setup()
{
  Serial.begin(115200);   // 通讯串口
  Serial2.begin(1000000); // 腿部sts舵机

  // freertos任务创建
  create_freertos_tasks();

  // 舵机初始化
  // 舵机有效行程450
  // 左侧舵机[2048+12+50,2048+12+450]
  // 左侧舵机[2048-12-50,2048-12-450]
  sms_sts.pSerial = &Serial2;
  ID[0] = 1;
  ID[1] = 2;
  ACC[0] = 30;
  ACC[1] = 30;
  Speed[0] = 300;
  Speed[1] = 300;
  Position[0] = 2148;
  Position[1] = 1948;
  // 舵机(ID1/ID2)以最高速度V=2400步/秒，加速度A=50(50*100步/秒^2)，运行至各自的Position位置
  sms_sts.SyncWritePosEx(ID, 2, Position, Speed, ACC);

  // 电压检测
  adc_calibration_init();
  adc1_config_width(width);
  adc1_config_channel_atten(channel, atten);
  esp_adc_cal_characterize(unit, atten, width, 0, &adc_chars);

  // 电量显示LED
  pinMode(LED_BAT, OUTPUT);

  // 编码器设置
  I2Cone.begin(19, 18, 400000UL);
  I2Ctwo.begin(23, 5, 400000UL);
  sensor1.init(&I2Cone);
  sensor2.init(&I2Ctwo);

  // mpu6050设置
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  // 连接motor对象与编码器对象
  motor1.linkSensor(&sensor1);
  motor2.linkSensor(&sensor2);

  // 速度环PID参数
  motor1.PID_velocity.P = 0.05;
  motor1.PID_velocity.I = 1;
  motor1.PID_velocity.D = 0;

  motor2.PID_velocity.P = 0.05;
  motor2.PID_velocity.I = 1;
  motor2.PID_velocity.D = 0;

  // 驱动器设置
  motor1.voltage_sensor_align = 6;
  motor2.voltage_sensor_align = 6;
  driver1.voltage_power_supply = 8;
  driver2.voltage_power_supply = 8;
  driver1.init();
  driver2.init();

  // 连接motor对象与驱动器对象
  motor1.linkDriver(&driver1);
  motor2.linkDriver(&driver2);

  motor1.torque_controller = TorqueControlType::voltage;
  motor2.torque_controller = TorqueControlType::voltage;
  motor1.controller = MotionControlType::torque;
  motor2.controller = MotionControlType::torque;

  // monitor相关设置
  motor1.useMonitoring(Serial);
  motor2.useMonitoring(Serial);
  // 电机初始化
  motor1.init();
  motor1.initFOC();
  motor2.init();
  motor2.initFOC();

  // 映射电机到commander
  command.add('A', StabAngle, "pid angle");
  command.add('B', StabGyro, "pid gyro");
  command.add('C', StabDistance, "pid distance");
  command.add('D', StabSpeed, "pid speed");
  command.add('E', StabYawAngle, "pid yaw angle");
  command.add('F', StabYawGyro, "pid yaw gyro");
  command.add('G', lpfJoyy, "lpf joyy");
  command.add('H', StabLqrU, "pid lqr u");
  command.add('I', StabZeropoint, "pid zeropoint");
  command.add('J', lpfZeropoint, "lpf zeropoint");
  command.add('K', StabRollAngle, "pid roll angle");
  command.add('L', lpfRoll, "lpf roll");

  // command.add('M', Stabtest_zeropoint, "test_zeropoint");

  delay(500);
}

void loop()
{

}

// 电压检测初始化
void adc_calibration_init()
{
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
  {
    printf("eFuse Two Point: Supported\n");
  }
  else
  {
    printf("eFuse Two Point: NOT supported\n");
  }
  // Check Vref is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
  {
    printf("eFuse Vref: Supported\n");
  }
  else
  {
    printf("eFuse Vref: NOT supported\n");
  }
}