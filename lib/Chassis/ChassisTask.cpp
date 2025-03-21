#include "ChassisTask.h"

/************实例定义*************/

// 电机实例
BLDCMotor motor1 = BLDCMotor(7);
BLDCMotor motor2 = BLDCMotor(7);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(32, 33, 25, 22);
BLDCDriver3PWM driver2 = BLDCDriver3PWM(26, 27, 14, 12);

// 编码器实例
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);
MagneticSensorI2C sensor1 = MagneticSensorI2C(AS5600_I2C);
MagneticSensorI2C sensor2 = MagneticSensorI2C(AS5600_I2C);

// PID控制器实例
PIDController pid_angle(1, 0, 0, 100000, 8);
PIDController pid_gyro(0.06, 0, 0, 100000, 8);
PIDController pid_distance(0.5, 0, 0, 100000, 8);
PIDController pid_speed(0.7, 0, 0, 100000, 8);
PIDController pid_yaw_angle(1.0, 0, 0, 100000, 8);
PIDController pid_yaw_gyro(0.04, 0, 0, 100000, 8);
PIDController pid_lqr_u(1, 15, 0, 100000, 8);
PIDController pid_zeropoint(0.002, 0, 0, 100000, 4);
PIDController pid_roll_angle(8, 0, 0, 100000, 450);

// 低通滤波器实例
LowPassFilter lpf_joyy(0.2);
LowPassFilter lpf_zeropoint(0.1);
LowPassFilter lpf_roll(0.3);

// commander通信实例
Commander command = Commander(Serial);

int joystick_value[2];

// STS舵机实例
SMS_STS sms_sts;

// MPU6050实例
MPU6050 mpu6050(I2Ctwo);
/************参数定义*************/
#define pi 3.1415927

// LQR自平衡控制器参数
float LQR_angle = 0;
float LQR_gyro = 0;
float LQR_speed = 0;
float LQR_distance = 0;
float angle_control = 0;
float gyro_control = 0;
float speed_control = 0;
float distance_control = 0;
float LQR_u = 0;
float angle_zeropoint = 0.9;  // 1 | -1.3 | 3.4
float distance_zeropoint = -256.0; // 轮部位移零点偏置（-256为一个不可能的位移值，将其作为未刷新的标志）

// YAW轴控制数据
float YAW_gyro = 0;
float YAW_angle = 0;
float YAW_angle_last = 0;
float YAW_angle_total = 0;
float YAW_angle_zero_point = -10;
float YAW_output = 0;

// 逻辑处理标志位
float robot_speed = 0;         // 记录当前轮部转速
float robot_speed_last = 0;    // 记录上一时刻的轮部转速
int wrobot_move_stop_flag = 0; // 记录摇杆停止的标识
int jump_flag = 0;             // 跳跃时段标识
float leg_position_add = 0;    // roll轴平衡控制量
int uncontrolable = 0;         // 机身倾角过大导致失控

// 腿部舵机控制数据
byte ID[2];
s16 Position[2];
u16 Speed[2];
byte ACC[2];

void ChassisTask::startTask()
{
  Serial.begin(115200);   // 通讯串口
  Serial2.begin(1000000); // 腿部sts舵机
  
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
void ChassisTask::Chassis_loop()
{
  mpu6050.update();   // IMU数据更新
  lqr_balance_loop(); // lqr自平衡控制
  yaw_loop();         // yaw轴转向控制
  leg_loop();         // 腿部动作控制

  // 将自平衡计算输出转矩赋给电机
  motor1.target = (-0.5) * (LQR_u + YAW_output);
  motor2.target = (-0.5) * (LQR_u - YAW_output);

  // 倒地失控后关闭输出
  if (abs(LQR_angle) > 25.0f)
  {
    uncontrolable = 1;
  }
  if (uncontrolable != 0) // 扶起后延时恢复
  {
    if (abs(LQR_angle) < 10.0f)
    {
      uncontrolable++;
    }
    if (uncontrolable > 200) // 200次程序循环的延时时间
    {
      uncontrolable = 0;
    }
  }

  // 关停输出（遥控停止或角度过大失控）
  if (wrobot.go == 0 || uncontrolable != 0)
  {
    motor1.target = 0;
    motor2.target = 0;
    leg_position_add = 0;
  }

  // 记录上一次的遥控数据数据
  wrobot.dir_last = wrobot.dir;
  wrobot.joyx_last = wrobot.joyx;
  wrobot.joyy_last = wrobot.joyy;

  // 迭代计算FOC相电压
  motor1.loopFOC();
  motor2.loopFOC();

  // 设置轮部电机输出
  motor1.move();
  motor2.move();

  command.run();
}
ChassisTask WLRobot;

// lqr自平衡控制
void lqr_balance_loop()
{
  // LQR平衡算式，实际使用中为便于调参，讲算式分解为4个P控制，采用PIDController方法在commander中实时调试
  // QR_u = LQR_k1*(LQR_angle - angle_zeropoint) + LQR_k2*LQR_gyro + LQR_k3*(LQR_distance - distance_zeropoint) + LQR_k4*LQR_speed;

  // 给负值是因为按照当前的电机接线，正转矩会向后转
  LQR_distance = (-0.5) * (motor1.shaft_angle + motor2.shaft_angle);
  LQR_speed = (-0.5) * (motor1.shaft_velocity + motor2.shaft_velocity);
  LQR_angle = (float)mpu6050.getAngleY();
  LQR_gyro = (float)mpu6050.getGyroY();
  // Serial.println(LQR_distance);

  // 计算自平衡输出
  angle_control = pid_angle(LQR_angle - angle_zeropoint);
  gyro_control = pid_gyro(LQR_gyro);

  // 零点找平时需要的日志输出
  {
    static int time_temp = 0;
    if (time_temp > 35)
    {
      Serial.println(LQR_angle);
      time_temp = 0;
    }
    else
      time_temp++;
  }

  // 运动细节优化处理
  if (wrobot.joyy != 0) // 有前后方向运动指令时的处理
  {
    distance_zeropoint = LQR_distance; // 位移零点重置
    pid_lqr_u.error_prev = 0;          // 输出积分清零
  }

  if ((wrobot.joyx_last != 0 && wrobot.joyx == 0) || (wrobot.joyy_last != 0 && wrobot.joyy == 0)) // 运动指令复零时的原地停车处理
  {
    wrobot_move_stop_flag = 1;
  }
  if ((wrobot_move_stop_flag == 1) && (abs(LQR_speed) < 0.5))
  {
    distance_zeropoint = LQR_distance; // 位移零点重置
    wrobot_move_stop_flag = 0;
  }

  if (abs(LQR_speed) > 15) // 被快速推动时的原地停车处理
  {
    distance_zeropoint = LQR_distance; // 位移零点重置
  }

  // 计算位移控制输出
  distance_control = pid_distance(LQR_distance - distance_zeropoint);
  speed_control = pid_speed(LQR_speed - 0.1 * lpf_joyy(wrobot.joyy));

  // 轮部离地检测
  robot_speed_last = robot_speed; // 记录连续两次的轮部转速
  robot_speed = LQR_speed;
  if (abs(robot_speed - robot_speed_last) > 10 || abs(robot_speed) > 50 || (jump_flag != 0)) // 若轮部角速度、角加速度过大或处于跳跃后的恢复时期，认为出现轮部离地现象，需要特殊处理
  {
    distance_zeropoint = LQR_distance;    // 位移零点重置
    LQR_u = angle_control + gyro_control; // 轮部离地情况下，对轮部分量不输出；反之，正常状态下完整输出平衡转矩
    pid_lqr_u.error_prev = 0;             // 输出积分清零
  }
  else
  {
    LQR_u = angle_control + gyro_control + distance_control + speed_control;
  }

  // 触发条件：遥控器无信号输入、轮部位移控制正常介入、不处于跳跃后的恢复时期
  if (abs(LQR_u) < 5 && wrobot.joyy == 0 && abs(distance_control) < 4 && (jump_flag == 0))
  {

    LQR_u = pid_lqr_u(LQR_u); // 补偿小转矩非线性
    // Serial.println(LQR_u);
    angle_zeropoint -= pid_zeropoint(lpf_zeropoint(distance_control)); // 重心自适应
  }
  else
  {
    pid_lqr_u.error_prev = 0; // 输出积分清零
  }

  // 平衡控制参数自适应
  if (wrobot.height < 50)
  {
    pid_speed.P = 0.7;
  }
  else if (wrobot.height < 64)
  {
    pid_speed.P = 0.6;
  }
  else
  {
    pid_speed.P = 0.5;
  }
}

// 腿部动作控制
void leg_loop()
{
  jump_loop();
  if (jump_flag == 0) // 不处于跳跃状态
  {
    // 机身高度自适应控制
    ACC[0] = 8;
    ACC[1] = 8;
    Speed[0] = 200;
    Speed[1] = 200;
    float roll_angle = (float)mpu6050.getAngleX() + 2.0;
    // leg_position_add += pid_roll_angle(roll_angle);
    leg_position_add = pid_roll_angle(lpf_roll(roll_angle)); // test
    Position[0] = 2048 + 12 + 8.4 * (wrobot.height - 32) - leg_position_add;
    Position[1] = 2048 - 12 - 8.4 * (wrobot.height - 32) - leg_position_add;
    if (Position[0] < 2110)
      Position[0] = 2110;
    else if (Position[0] > 2510)
      Position[0] = 2510;
    if (Position[1] < 1586)
      Position[1] = 1586;
    else if (Position[1] > 1986)
      Position[1] = 1986;
    sms_sts.SyncWritePosEx(ID, 2, Position, Speed, ACC);
  }
}

// 跳跃控制
void jump_loop()
{
  if ((wrobot.dir_last == 5) && (wrobot.dir == 4) && (jump_flag == 0))
  {
    ACC[0] = 0;
    ACC[1] = 0;
    Speed[0] = 0;
    Speed[1] = 0;
    Position[0] = 2048 + 12 + 8.4 * (80 - 32);
    Position[1] = 2048 - 12 - 8.4 * (80 - 32);
    sms_sts.SyncWritePosEx(ID, 2, Position, Speed, ACC);

    jump_flag = 1;
  }
  if (jump_flag > 0)
  {
    jump_flag++;
    if ((jump_flag > 30) && (jump_flag < 35))
    {
      ACC[0] = 0;
      ACC[1] = 0;
      Speed[0] = 0;
      Speed[1] = 0;
      Position[0] = 2048 + 12 + 8.4 * (40 - 32);
      Position[1] = 2048 - 12 - 8.4 * (40 - 32);
      sms_sts.SyncWritePosEx(ID, 2, Position, Speed, ACC);

      jump_flag = 40;
    }
    if (jump_flag > 200)
    {
      jump_flag = 0; // 已准备好再次跳跃
    }
  }
}

// yaw轴转向控制
void yaw_loop()
{
  // YAW_output = 0.03*(YAW_Kp*YAW_angle_total + YAW_Kd*YAW_gyro);
  yaw_angle_addup();

  YAW_angle_total += wrobot.joyx * 0.002;
  float yaw_angle_control = pid_yaw_angle(YAW_angle_total);
  float yaw_gyro_control = pid_yaw_gyro(YAW_gyro);
  YAW_output = yaw_angle_control + yaw_gyro_control;
}

// yaw轴角度累加函数
void yaw_angle_addup()
{
  YAW_angle = (float)mpu6050.getAngleZ();
  YAW_gyro = (float)mpu6050.getGyroZ();

  if (YAW_angle_zero_point == (-10))
  {
    YAW_angle_zero_point = YAW_angle;
  }

  float yaw_angle_1, yaw_angle_2, yaw_addup_angle;
  if (YAW_angle > YAW_angle_last)
  {
    yaw_angle_1 = YAW_angle - YAW_angle_last;
    yaw_angle_2 = YAW_angle - YAW_angle_last - 2 * PI;
  }
  else
  {
    yaw_angle_1 = YAW_angle - YAW_angle_last;
    yaw_angle_2 = YAW_angle - YAW_angle_last + 2 * PI;
  }

  if (abs(yaw_angle_1) > abs(yaw_angle_2))
  {
    yaw_addup_angle = yaw_angle_2;
  }
  else
  {
    yaw_addup_angle = yaw_angle_1;
  }

  YAW_angle_total = YAW_angle_total + yaw_addup_angle;
  YAW_angle_last = YAW_angle;
}





void StabAngle(char *cmd) { command.pid(&pid_angle, cmd); }
void StabGyro(char *cmd) { command.pid(&pid_gyro, cmd); }
void StabDistance(char *cmd) { command.pid(&pid_distance, cmd); }
void StabSpeed(char *cmd) { command.pid(&pid_speed, cmd); }
void StabYawAngle(char *cmd) { command.pid(&pid_yaw_angle, cmd); }
void StabYawGyro(char *cmd) { command.pid(&pid_yaw_gyro, cmd); }
void lpfJoyy(char *cmd) { command.lpf(&lpf_joyy, cmd); }
void StabLqrU(char *cmd) { command.pid(&pid_lqr_u, cmd); }
void StabZeropoint(char *cmd) { command.pid(&pid_zeropoint, cmd); }
void lpfZeropoint(char *cmd) { command.lpf(&lpf_zeropoint, cmd); }
void StabRollAngle(char *cmd) { command.pid(&pid_roll_angle, cmd); }
void lpfRoll(char *cmd) { command.lpf(&lpf_roll, cmd); }

// void Stabtest_zeropoint(char* cmd) { command.pid(&test_zeropoint, cmd); }