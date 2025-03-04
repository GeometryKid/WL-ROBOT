#include "freertos_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 定义任务句柄
TaskHandle_t bleCheckTaskHandle = NULL; // 蓝牙检查任务句柄
TaskHandle_t xboxTaskHandle = NULL; // Xbox任务句柄
TaskHandle_t ChassisTaskHandle = NULL;

uint8_t xbox_datas[28] = {0}; // Xbox数据数组

// 定义XBOX控制器实例
XboxSeriesXControllerESP32_asukiaaa::Core
    xboxController("28:ea:0b:ff:97:ae"); 

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
                Serial.print(xbox_string());
                //  demoVibration();
                //   demoVibration_2();
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

// Xbox 数据处理任务，250Hz，绑定到 Core 0
void xboxTask(void *pvParameters)
{
    const TickType_t xFrequency = pdMS_TO_TICKS(2); // 250Hz = 4ms

    for (;;)
    {
        xbox_datas[0] = xboxController.xboxNotif.btnY;
        xbox_datas[1] = xboxController.xboxNotif.btnB;
        xbox_datas[2] = xboxController.xboxNotif.btnA;
        xbox_datas[3] = xboxController.xboxNotif.btnX;
        xbox_datas[4] = xboxController.xboxNotif.btnShare;
        xbox_datas[5] = xboxController.xboxNotif.btnStart;
        xbox_datas[6] = xboxController.xboxNotif.btnSelect;
        xbox_datas[7] = xboxController.xboxNotif.btnXbox;
        xbox_datas[8] = xboxController.xboxNotif.btnLB;
        xbox_datas[9] = xboxController.xboxNotif.btnRB;
        xbox_datas[10] = xboxController.xboxNotif.btnLS;
        xbox_datas[11] = xboxController.xboxNotif.btnRS;
        xbox_datas[12] = xboxController.xboxNotif.btnDirUp;
        xbox_datas[13] = xboxController.xboxNotif.btnDirLeft;
        xbox_datas[14] = xboxController.xboxNotif.btnDirRight;
        xbox_datas[15] = xboxController.xboxNotif.btnDirDown;
        // 0~14位为按键bool值
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

// 定义底盘控制任务函数
void ChassisTask(void *pvParameters)
{
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 100Hz = 10ms

    for (;;)
    {
        // 控制机器人底盘运动的代码
        vTaskDelay(xFrequency); // 使用相对延时，延时 10ms
    }
}

// 创建任务函数
void create_freertos_tasks()
{
    // 创建 BLE 连接检查任务，绑定到 Core 1
    xTaskCreatePinnedToCore(
        bleCheckTask,        // 任务函数
        "BLE Check Task",    // 任务名称
        10000,               // 栈大小
        NULL,                // 任务参数
        3,                   // 优先级
        &bleCheckTaskHandle, // 任务句柄
        1);                  // 绑定到 Core 1

    // 创建 Xbox 数据处理任务，绑定到 Core 0
    xTaskCreatePinnedToCore(
        xboxTask,        // 任务函数
        "Xbox Task",     // 任务名称
        10000,           // 栈大小
        NULL,            // 任务参数
        4,               // 优先级
        &xboxTaskHandle, // 任务句柄
        0);              // 绑定到 Core 0
    
    // 创建底盘控制任务，绑定到 Core 0
    xTaskCreatePinnedToCore(
        ChassisTask,        // 任务函数
        "Chassis Task",     // 任务名称
        10000,              // 栈大小
        NULL,               // 任务参数
        5,                  // 优先级
        &ChassisTaskHandle, // 任务句柄
        0);                 // 绑定到 Core 0

    xboxController.begin();
}