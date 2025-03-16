#ifndef XBOXCONTROL_H
#define XBOXCONTROL_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "robot.h"
#include "XboxSeriesXControllerESP32_asukiaaa.hpp"

void XboxControlTask(); 

#endif