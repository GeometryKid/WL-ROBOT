#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// wifi控制数据传输头文件
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>
#include <FS.h>
#include "basic_web.h"
#include "robot.h"
#include "wifi_config.h"
#include "XboxSeriesXControllerESP32_asukiaaa.hpp"

#include "ChassisTask.h"

// 任务函数声明
void create_FreertosTasks(); // 创建所有任务
void networkTask(void *pvParameters);
void basicWebCallback(void);
void webSocketEventCallback(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

class NetworkHandler {
private:
    std::mutex wsMutex;
public:
    void sendWSData(String& data) {
        std::lock_guard<std::mutex> lock(wsMutex);
        websocket.broadcastTXT(data);
    }
    
    void handleEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        std::lock_guard<std::mutex> lock(wsMutex);
        // 事件处理逻辑
    }
};

#endif // FREERTOS_TASK_H