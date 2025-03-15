#include "Battery.h"

// 电压检测
uint16_t bat_check_num = 0;
int BAT_PIN = 35; // select the input pin for the ADC

// 电压检测初始化
void adc_calibration_init()
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
    }
    else
    {
        printf("eFuse Two Point: NOT supported\n");
    }
    // Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        printf("eFuse Vref: Supported\n");
    }
    else
    {
        printf("eFuse Vref: NOT supported\n");
    }
}
// 电压检测
void CheckBatForSafety()
{
    if (bat_check_num > 1000)
    {
        // 电压读取
        uint32_t sum = 0;
        sum = analogRead(BAT_PIN);
        uint32_t voltage = esp_adc_cal_raw_to_voltage(sum, &adc_chars);
        double battery = (voltage * 3.97) / 1000.0;

        // Serial.println(battery);
        // 电量显示
        if (battery > 7.8)
            digitalWrite(LED_BAT, HIGH);
        else
            digitalWrite(LED_BAT, LOW);

        bat_check_num = 0;
    }
    else bat_check_num++;
}

void BatteryVoltageCheckTask(void *pvParameters)
{
    // 电压检测
    adc_calibration_init();
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    esp_adc_cal_characterize(unit, atten, width, 0, &adc_chars);
    // 电量显示LED
    pinMode(LED_BAT, OUTPUT);
    for (;;)
    {
        CheckBatForSafety();
        vTaskDelay(50);
    }
};
void Battery::startTask()
{ 
    xTaskCreatePinnedToCore(BatteryVoltageCheckTask, "Battery_Voltage_Check", 2048, NULL, 5, NULL, 1);
};
Battery bat;

