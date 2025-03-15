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

#include "robot.h"

Wrobot wrobot;
// WebServer实例
WebServer webserver;                               // server服务器
WebSocketsServer websocket = WebSocketsServer(81);
RobotProtocol rp(20);

// 构造函数，初始化RobotProtocol对象
RobotProtocol::RobotProtocol(uint8_t len)
{
    // 初始化_len变量
    _len = len;
    // 动态分配_now_buf数组，大小为_len
    _now_buf = new uint8_t[_len];
    // 动态分配_old_buf数组，大小为_len
    _old_buf = new uint8_t[_len];

    // 将_now_buf数组中的所有元素初始化为0
    for (int i = 0; i < _len; i++)
    {
        _now_buf[i] = 0;
    }

    // 将_now_buf数组的第一个元素设置为0xAA
    _now_buf[0] = 0xAA;
    // 将_now_buf数组的第二个元素设置为0x55
    _now_buf[1] = 0x55;
}

// 析构函数，释放动态分配的内存
RobotProtocol::~RobotProtocol()
{
    // 释放_now_buf指向的内存
    delete[] _now_buf;
    // 释放_old_buf指向的内存
    delete[] _old_buf;
}

void RobotProtocol::spinOnce(void)
{
    // 检查缓冲区是否需要刷新
    int flag = checkBufRefresh();
    if (flag)
    {
        // UART_WriteBuf(); //这个会将web端的控制信息转成串口协议发出

        // Serial.println(wrobot.dir);//测试数据获取
        // Serial.println("date have send\n");
        // Serial.printf("height:%d\n", wrobot.height);
        // Serial.printf("roll:%d\n", wrobot.pitch);
        // Serial.printf("linear:%d\n", wrobot.linear);
        //        Serial.printf("\n");
        //        Serial.printf("joy_X:%d\n", wrobot.joyx);
        //        Serial.printf("joy_Y:%d\n", wrobot.joyy);
    }
}

/**************如下同时用于串口输出控制协议***************/

// 向串口发送数据
void RobotProtocol::UART_WriteBuf(void)
{
    // 循环发送数据
    for (int i = 0; i < _len; i++)
    {
        // 发送当前缓冲区中的数据
        Serial.write(_now_buf[i]);
    }
}

int RobotProtocol::checkBufRefresh(void)
{
    // 定义返回值
    int ret = 0;
    // 遍历_now_buf数组
    for (int i = 0; i < _len; i++)
    {
        // 如果_now_buf和_old_buf数组中的元素不相等
        if (_now_buf[i] != _old_buf[i])
        {
            // 返回1
            ret = 1;
            // 跳出循环
            break;
        }
        else
        {
            // 否则返回0
            ret = 0;
        }
    }

    // 遍历_now_buf数组
    for (int i = 0; i < _len; i++)
    {
        // 将_now_buf数组中的元素赋值给_old_buf数组
        _old_buf[i] = _now_buf[i];
    }
    // 返回ret
    return ret;
}

void RobotProtocol::parseBasic(StaticJsonDocument<300> &doc)
{

    // 设置当前缓冲区类型为BASIC
    _now_buf[2] = BASIC;

    // 获取方向
    String dir = doc["dir"];
    if (dir == "stop")
    {
        // 如果方向为stop，则设置当前缓冲区方向为STOP，并将wrobot的方向设置为STOP
        _now_buf[3] = STOP;
        wrobot.dir = STOP;
    }
    else
    {
        if (dir == "jump")
        {
            // 如果方向为jump，则设置当前缓冲区方向为JUMP，并将wrobot的方向设置为JUMP
            _now_buf[3] = JUMP;
            wrobot.dir = JUMP;
        }
        else if (dir == "forward")
        {
            // 如果方向为forward，则设置当前缓冲区方向为FORWARD，并将wrobot的方向设置为FORWARD
            _now_buf[3] = FORWARD;
            wrobot.dir = FORWARD;
        }
        else if (dir == "back")
        {
            // 如果方向为back，则设置当前缓冲区方向为BACK，并将wrobot的方向设置为BACK
            _now_buf[3] = BACK;
            wrobot.dir = BACK;
        }
        else if (dir == "left")
        {
            // 如果方向为left，则设置当前缓冲区方向为LEFT，并将wrobot的方向设置为LEFT
            _now_buf[3] = LEFT;
            wrobot.dir = LEFT;
        }
        else if (dir == "right")
        {
            // 如果方向为right，则设置当前缓冲区方向为RIGHT，并将wrobot的方向设置为RIGHT
            _now_buf[3] = RIGHT;
            wrobot.dir = RIGHT;
        }
        else
        {
            // 如果方向不是以上任何一种，则设置当前缓冲区方向为STOP，并将wrobot的方向设置为STOP
            _now_buf[3] = STOP;
            wrobot.dir = STOP;
        }
    }

    // 获取高度
    int height = doc["height"];
    // 设置当前缓冲区高度为获取的高度，并将wrobot的高度设置为获取的高度
    _now_buf[4] = height;
    wrobot.height = height;

    // 获取roll
    int roll = doc["roll"];
    // 设置wrobot的roll为获取的roll
    wrobot.roll = roll;
    if (roll >= 0)
    {
        // 如果roll大于等于0，则设置当前缓冲区roll为0
        _now_buf[5] = 0;
    }
    else
    {
        // 如果roll小于0，则设置当前缓冲区roll为1
        _now_buf[5] = 1;
    }
    // 设置当前缓冲区roll的绝对值为获取的roll的绝对值
    _now_buf[6] = abs(roll);

    // 获取linear
    int linear = doc["linear"];
    // 设置wrobot的linear为获取的linear
    wrobot.linear = linear;
    if (linear >= 0)
    {
        // 如果linear大于等于0，则设置当前缓冲区linear为0
        _now_buf[7] = 0;
    }
    else
    {
        // 如果linear小于0，则设置当前缓冲区linear为1
        _now_buf[7] = 1;
    }
    // 设置当前缓冲区linear的绝对值为获取的linear的绝对值
    _now_buf[8] = abs(linear);

    // 获取angular
    int angular = doc["angular"];
    // 设置wrobot的angular为获取的angular
    wrobot.angular = angular;
    if (angular >= 0)
    {
        // 如果angular大于等于0，则设置当前缓冲区angular为0
        _now_buf[9] = 0;
    }
    else
    {
        // 如果angular小于0，则设置当前缓冲区angular为1
        _now_buf[9] = 1;
    }
    // 设置当前缓冲区angular的绝对值为获取的angular的绝对值
    _now_buf[10] = abs(angular);

    // 获取stable
    int stable = doc["stable"];
    // 设置wrobot的go为获取的stable
    wrobot.go = stable;
    if (stable)
    {
        // 如果stable为真，则设置当前缓冲区stable为1
        _now_buf[11] = 1;
    }
    else
    {
        // 如果stable为假，则设置当前缓冲区stable为0
        _now_buf[11] = 0;
    }

    // 获取joy_x
    int joy_x = doc["joy_x"];
    // 设置wrobot的joyx为获取的joy_x
    wrobot.joyx = joy_x;
    if (joy_x >= 0)
    {
        // 如果joy_x大于等于0，则设置当前缓冲区joy_x为0
        _now_buf[12] = 0;
    }
    else
    {
        // 如果joy_x小于0，则设置当前缓冲区joy_x为1
        _now_buf[12] = 1;
    }
    // 设置当前缓冲区joy_x的绝对值为获取的joy_x的绝对值
    _now_buf[13] = abs(joy_x);

    // 获取joy_y
    int joy_y = doc["joy_y"];
    // 设置wrobot的joyy为获取的joy_y
    wrobot.joyy = joy_y;
    if (joy_y >= 0)
    {
        // 如果joy_y大于等于0，则设置当前缓冲区joy_y为0
        _now_buf[14] = 0;
    }
    else
    {
        // 如果joy_y小于0，则设置当前缓冲区joy_y为1
        _now_buf[14] = 1;
    }
    // 设置当前缓冲区joy_y的绝对值为获取的joy_y的绝对值
    _now_buf[15] = abs(joy_y);
}

void basicWebCallback(void)
{
  webserver.send(300, "text/html", basic_web);
}

// 定义一个WebSocket事件回调函数
void webSocketEventCallback(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if (type == WStype_TEXT)
  {
    String payload_str = String((char *)payload);
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, payload_str);

    String mode_str = doc["mode"];
    if (mode_str == "basic")
    {
      rp.parseBasic(doc);
    }
  }
}

static void AppServerTask(void *pvParameters)
{
    // Wifi初始化
    WiFi_SetAP();
    // set_sta_wifi();      // ESP-01S STA模式接入WiFi网络
    webserver.begin();
    webserver.on("/", HTTP_GET, basicWebCallback);
    websocket.begin();
    websocket.onEvent(webSocketEventCallback);
    for (;;) {
        if (WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_STA) 
        {
            Serial.println("WiFi断开，尝试重连...");
            WiFi.reconnect();
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void CarBrakeTask(void *pvParameters)
{
    for (;;)
    {
        // 处理Web请求和WebSocket事件
        webserver.handleClient();
        websocket.loop();
        
        // 更新web端回传的控制信息
        rp.spinOnce();
        
        vTaskDelay(10);
    }
};

void AppTaskInit::startTask()
{
    xTaskCreatePinnedToCore(AppServerTask, "App Sever Task", 6144, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(CarBrakeTask, "Brake Car Task", 2048, NULL, 1, NULL, 0);
};
AppTaskInit APP;