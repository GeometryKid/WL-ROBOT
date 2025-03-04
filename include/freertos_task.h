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
#include "esp_adc_cal.h"

// 任务函数声明
void create_freertos_tasks(); // 创建所有任务
void networkTask(void *pvParameters);
void basicWebCallback(void);
void webSocketEventCallback(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

extern NetworkHandler networkHandler;
extern RobotProtocol rp(20);

#endif // FREERTOS_TASK_H