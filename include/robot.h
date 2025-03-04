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

#pragma once

#include <WiFi.h>
#include <ArduinoJson.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

typedef struct
{
  int height = 38;
  int roll;
  int linear;
  int angular;
  int dir;
  int dir_last;
  int joyy;
  int joyy_last;
  int joyx;
  int joyx_last;
  bool go;
} Wrobot;

extern Wrobot wrobot;

// 机器人运动状态枚举
typedef enum
{
  FORWARD = 0,
  BACK,
  RIGHT,
  LEFT,
  STOP,
  JUMP,
} QR_State_t;

// 机器人模式枚举类型
typedef enum
{
  BASIC = 0,
} Robot_Mode_t;

// 机器人协议
class RobotProtocol
{
public:
  // 构造函数，初始化长度
  RobotProtocol(uint8_t len);
  // 析构函数
  ~RobotProtocol();
  // 执行一次操作
  void spinOnce(void);
  // 解析基本数据
  void parseBasic(StaticJsonDocument<300> &doc);
   // 新增 Xbox 输入解析方法
  void parseXboxInput() {
      _now_buf[2] = BASIC;

      // 方向
      _now_buf[3] = wrobot.dir;

      // 高度
      _now_buf[4] = wrobot.height;

      // 横滚（符号 + 绝对值）
      _now_buf[5] = (wrobot.roll >= 0) ? 0 : 1;
      _now_buf[6] = abs(wrobot.roll);

      // 线速度
      _now_buf[7] = (wrobot.linear >= 0) ? 0 : 1;
      _now_buf[8] = abs(wrobot.linear);

      // 角速度
      _now_buf[9] = (wrobot.angular >= 0) ? 0 : 1;
      _now_buf[10] = abs(wrobot.angular);

      // 稳定模式
      _now_buf[11] = wrobot.go ? 1 : 0;

      // 摇杆值（已通过 mapJoysticks 处理）
      _now_buf[12] = (wrobot.joyx >= 0) ? 0 : 1;
      _now_buf[13] = abs(wrobot.joyx);
      _now_buf[14] = (wrobot.joyy >= 0) ? 0 : 1;
      _now_buf[15] = abs(wrobot.joyy);
  }

private:
  // 当前缓冲区
  uint8_t *_now_buf;
  // 旧缓冲区
  uint8_t *_old_buf;
  // 长度
  uint8_t _len;
  // 写入缓冲区
  void UART_WriteBuf(void);
  // 检查缓冲区是否刷新
  int checkBufRefresh(void);
};

class XboxControllerHandler {
public:
    void setup() {
        // 初始化 Xbox 控制器
        Xbox.begin();
    }

    void update(RobotProtocol &protocol) {
        // 轮询手柄状态
        Xbox.refresh();

        // 若手柄未连接则退出
        if (!Xbox.isConnected()) return;

        // 将 Xbox 输入映射到 wrobot 变量
        mapDirection(Xbox);
        mapJoysticks(Xbox);
        mapTriggers(Xbox);
        mapButtons(Xbox);

        // 更新协议缓冲区
        protocol.parseXboxInput(); // 需新增方法
    }

private:
    // 将 Xbox 控制器的方向映射到 wrobot 变量
    void mapDirection(XboxController &xbox);
    // 将 Xbox 控制器的摇杆映射到 wrobot 变量
    void mapJoysticks(XboxController &xbox);
    // 将 Xbox 控制器的触发器映射到 wrobot 变量
    void mapTriggers(XboxController &xbox);
    // 将 Xbox 控制器的按钮映射到 wrobot 变量
    void mapButtons(XboxController &xbox);
};