#ifndef _BATTERY_H
#define _BATTERY_H

#include "esp_adc_cal.h"
#include "SimpleFOC.h"

static esp_adc_cal_characteristics_t adc_chars;
static const adc1_channel_t channel = ADC1_CHANNEL_7;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

// 电量显示LED
#define LED_BAT 13

class Battery
{
  public:
    void startTask();

};
extern Battery bat;

void adc_calibration_init();
void CheckBatForSafety();

#endif