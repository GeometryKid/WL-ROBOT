#include "XboxControl.h"

uint8_t xbox_datas[28] = {0}; // Xbox数据数组
// 定义XBOX控制器实例
XboxSeriesXControllerESP32_asukiaaa::Core
    xboxController("0c:35:26:56:a0:22"); 

String xbox_string()
{
    String str = String(xboxController.xboxNotif.btnY) + "," +
                 String(xboxController.xboxNotif.btnX) + "," +
                 String(xboxController.xboxNotif.btnB) + "," +
                 String(xboxController.xboxNotif.btnA) + "," +
                 String(xboxController.xboxNotif.btnLB) + "," +
                 String(xboxController.xboxNotif.btnRB) + "," +
                 String(xboxController.xboxNotif.btnSelect) + "," +
                 String(xboxController.xboxNotif.btnStart) + "," +
                 String(xboxController.xboxNotif.btnXbox) + "," +
                 String(xboxController.xboxNotif.btnShare) + "," +
                 String(xboxController.xboxNotif.btnLS) + "," +
                 String(xboxController.xboxNotif.btnRS) + "," +
                 String(xboxController.xboxNotif.btnDirUp) + "," +
                 String(xboxController.xboxNotif.btnDirDown) + "," +
                 String(xboxController.xboxNotif.btnDirLeft) + "," +
                 String(xboxController.xboxNotif.btnDirRight) + "," +
                 String(xboxController.xboxNotif.joyLHori) + "," +
                 String(xboxController.xboxNotif.joyLVert) + "," +
                 String(xboxController.xboxNotif.joyRHori) + "," +
                 String(xboxController.xboxNotif.joyRVert) + "," +
                 String(xboxController.xboxNotif.trigLT) + "," +
                 String(xboxController.xboxNotif.trigRT) + "\n";
    return str;
};

// 蓝牙连接检查任务，500Hz，绑定到 Core 1
void bleCheckTask(void *pvParameters)
{
    const TickType_t xFrequency = pdMS_TO_TICKS(2); // 500Hz = 2ms

    for (;;)
    {
        // 蓝牙连接检查逻辑
        xboxController.onLoop();
        if (xboxController.isConnected())
        {
            if (xboxController.isWaitingForFirstNotification())
            {
                Serial.println("waiting for first notification");
            }
            else
            {
                // Serial.print(xbox_string());
                //  demoVibration();
                //  demoVibration_2();
            }
        }
        else
        {
            Serial.println("not connected");
            if (xboxController.getCountFailedConnection() > 2)
            {
                ESP.restart();
            }
        }

        vTaskDelay(xFrequency); // 使用相对延时，延时 2ms
    }
}

// Xbox 数据处理任务，250Hz，绑定到 Core 1
void xboxTask(void *pvParameters)
{
    const TickType_t xFrequency = pdMS_TO_TICKS(2); // 250Hz = 4ms

    for (;;)
    {
        xbox_datas[0] = xboxController.xboxNotif.btnY;
        xbox_datas[1] = xboxController.xboxNotif.btnX;
        xbox_datas[2] = xboxController.xboxNotif.btnB;
        xbox_datas[3] = xboxController.xboxNotif.btnA;
        xbox_datas[4] = xboxController.xboxNotif.btnLB;
        xbox_datas[5] = xboxController.xboxNotif.btnRB;
        xbox_datas[6] = xboxController.xboxNotif.btnSelect;
        xbox_datas[7] = xboxController.xboxNotif.btnStart;
        xbox_datas[8] = xboxController.xboxNotif.btnXbox;
        xbox_datas[9] = xboxController.xboxNotif.btnShare;
        xbox_datas[10] = xboxController.xboxNotif.btnLS;
        xbox_datas[11] = xboxController.xboxNotif.btnRS;
        xbox_datas[12] = xboxController.xboxNotif.btnDirUp;
        xbox_datas[13] = xboxController.xboxNotif.btnDirDown;
        xbox_datas[14] = xboxController.xboxNotif.btnDirLeft;
        xbox_datas[15] = xboxController.xboxNotif.btnDirRight;
        // 1~14位为按键bool值
        // 15~28为霍尔值
        xbox_datas[16] = (xboxController.xboxNotif.joyLHori >> 8) & 0xFF;
        xbox_datas[17] = xboxController.xboxNotif.joyLHori & 0xFF;
        xbox_datas[18] = (xboxController.xboxNotif.joyLVert >> 8) & 0xFF;
        xbox_datas[19] = xboxController.xboxNotif.joyLVert & 0xFF;
        xbox_datas[20] = (xboxController.xboxNotif.joyRHori >> 8) & 0xFF;
        xbox_datas[21] = xboxController.xboxNotif.joyRHori & 0xFF;
        xbox_datas[22] = (xboxController.xboxNotif.joyRVert >> 8) & 0xFF;
        xbox_datas[23] = xboxController.xboxNotif.joyRVert & 0xFF;
        xbox_datas[24] = (xboxController.xboxNotif.trigLT >> 8) & 0xFF;
        xbox_datas[25] = xboxController.xboxNotif.trigLT & 0xFF;
        xbox_datas[26] = (xboxController.xboxNotif.trigRT >> 8) & 0xFF;
        xbox_datas[27] = xboxController.xboxNotif.trigRT & 0xFF;
        vTaskDelay(xFrequency); // 使用相对延时，延时 4ms
    }
}

void controlTask(void *pvParameters)
{
    const TickType_t xFrequency = pdMS_TO_TICKS(2); // 250Hz = 4ms
    for (;;) // 任务循环
    {
        // 获取方向
        if (xbox_datas[0] == 1)
        {
            // 按下Y键停止，wrobot的方向设置为STOP
            wrobot.dir = STOP;
        }
        else
        {
            if (xbox_datas[3] == 1)
            {
                // 按下A键跳跃，wrobot的方向设置为JUMP
                wrobot.dir = JUMP;
            }
            else if (xbox_datas[12] == 1)
            {
                // 按下前进键,wrobot的方向设置为FORWARD
                wrobot.dir = FORWARD;
            }
            else if (xbox_datas[13] == 1)
            {
                // 按下后退键,wrobot的方向设置为BACK
                wrobot.dir = BACK;
            }
            else if (xbox_datas[14] == 1)
            {
                // 按下向左键,wrobot的方向设置为LEFT
                wrobot.dir = LEFT;
            }
            else if (xbox_datas[15] == 1)
            {
                // 按下向右键,wrobot的方向设置为RIGHT
                wrobot.dir = RIGHT;
            }
            else
            {
                // 如果方向不是以上任何一种，wrobot的方向设置为STOP
                wrobot.dir = STOP;
            }
        }

        // // 获取高度
        // int height = doc["height"];
        // // 设置当前缓冲区高度为获取的高度，并将wrobot的高度设置为获取的高度
        // wrobot.height = height;

        // // 获取roll
        // int roll = doc["roll"];
        // // 设置wrobot的roll为获取的roll
        // wrobot.roll = roll;
        // if (roll >= 0)
        // {
        //     // 如果roll大于等于0，则设置当前缓冲区roll为0
        //     _now_buf[5] = 0;
        // }
        // else
        // {
        //     // 如果roll小于0，则设置当前缓冲区roll为1
        //     _now_buf[5] = 1;
        // }
        // // 设置当前缓冲区roll的绝对值为获取的roll的绝对值
        // _now_buf[6] = abs(roll);

        // // 获取linear
        // int linear = doc["linear"];
        // // 设置wrobot的linear为获取的linear
        // wrobot.linear = linear;
        // if (linear >= 0)
        // {
        //     // 如果linear大于等于0，则设置当前缓冲区linear为0
        //     _now_buf[7] = 0;
        // }
        // else
        // {
        //     // 如果linear小于0，则设置当前缓冲区linear为1
        //     _now_buf[7] = 1;
        // }
        // // 设置当前缓冲区linear的绝对值为获取的linear的绝对值
        // _now_buf[8] = abs(linear);

        // // 获取angular
        // int angular = doc["angular"];
        // // 设置wrobot的angular为获取的angular
        // wrobot.angular = angular;
        // if (angular >= 0)
        // {
        //     // 如果angular大于等于0，则设置当前缓冲区angular为0
        //     _now_buf[9] = 0;
        // }
        // else
        // {
        //     // 如果angular小于0，则设置当前缓冲区angular为1
        //     _now_buf[9] = 1;
        // }
        // // 设置当前缓冲区angular的绝对值为获取的angular的绝对值
        // _now_buf[10] = abs(angular);

        // 按下X键,切换一次wrobot的go
        if (xbox_datas[1] == 1)
        {
            wrobot.go = 1 - wrobot.go;
        }

        // 设置wrobot的joyx为获取的joy_x
        wrobot.joyx = xbox_datas[16];

        // 设置wrobot的joyy为获取的joy_y
        wrobot.joyy = xbox_datas[17];
        vTaskDelay(xFrequency); // 使用相对延时，延时 4ms
    }
}
// 创建任务函数
void XboxControlTask()
{
    // 创建 BLE 连接检查任务，绑定到 Core 1
    xTaskCreatePinnedToCore(
        bleCheckTask,        // 任务函数
        "BLE Check Task",    // 任务名称
        10000,               // 栈大小
        NULL,                // 任务参数
        3,                   // 优先级
        NULL,                // 任务句柄
        1);                  // 绑定到 Core 1

    // 创建 Xbox 数据处理任务，绑定到 Core 1
    xTaskCreatePinnedToCore(
        xboxTask,        // 任务函数
        "Xbox Task",     // 任务名称
        10000,           // 栈大小
        NULL,            // 任务参数
        1,               // 优先级
        NULL,            // 任务句柄
        1);              // 绑定到 Core 1
    // 创建 Control 任务，绑定到 Core 1
    xTaskCreatePinnedToCore(
        controlTask,     // 任务函数
        "Control Task",  // 任务名称
        10000,           // 栈大小
        NULL,            // 任务参数
        2,               // 优先级
        NULL,            // 任务句柄
        1);              // 绑定到 Core 1
    xboxController.begin();
}