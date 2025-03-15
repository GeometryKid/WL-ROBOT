#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "XboxSeriesXControllerESP32_asukiaaa.hpp"

// 任务函数声明
void create_FreertosTasks(); // 创建所有任务

#endif // FREERTOS_TASK_H