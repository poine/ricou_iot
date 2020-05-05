#ifndef RICOU_IOT_H
#define RICOU_IOT_H

#include <Arduino.h>

// TTGO Higrow pin assigment
#define TTG_HG_BOOT_PIN             0
#define TTG_HG_POWER_CTRL_PIN       4        // sensors powering ??
#define TTG_HG_DHT12_PIN           16        // DHT12 pin (one wire)
#define TTG_HG_DS18B20_PIN         21        // 18b20 data pin
#define TTG_HG_I2C_SDA             25        // I2C pins
#define TTG_HG_I2C_SCL             26
#define TTG_HG_SOIL_ADC_PIN        32
#define TTG_HG_BAT_ADC_PIN         33
#define TTG_HG_SALT_ADC_PIN        34
#define TTG_HG_USER_BUTTON         35

// Translations for web interface
#if defined USE_CHINESE_WEB
#define TTG_HG_LABEL_TEMP   "BME传感器温度/C"
#define TTG_HG_LABEL_PRESS  "BME传感器压力/hPa"
#define TTG_HG_LABEL_ALT    "BME传感器高度/m"
#define TTG_HG_LABEL_TEMP2  "DHT12传感器温度/C"
#define TTG_HG_LABEL_HUM2   "DHT12传感器湿度/%"
#define TTG_HG_LABEL_LUX    "BH1750传感器亮度/lx"
#define TTG_HG_LABEL_SOIL   "土壤湿度"
#define TTG_HG_LABEL_SALT   "水分百分比"
#define TTG_HG_LABEL_BAT    "电池电压/mV"
#define TTG_HG_LABEL_TEMP3  "18B20温度/C"
#else
#define TTG_HG_LABEL_TEMP   "BME Temperature/C"
#define TTG_HG_LABEL_PRESS  "BME Pressure/hPa"
#define TTG_HG_LABEL_ALT    "BME Altitude/m"
#define TTG_HG_LABEL_TEMP2  "DHT Temperature/C"
#define TTG_HG_LABEL_HUM2   "DHT Humidity/%"
#define TTG_HG_LABEL_LUX    "BH1750/lx"
#define TTG_HG_LABEL_SOIL   "Soil"
#define TTG_HG_LABEL_SALT   "Salt"
#define TTG_HG_LABEL_BAT    "Battery/mV"
#define TTG_HG_LABEL_TEMP3  "18B20 Temperature/C"
#endif // USE_CHINESE_WEB



// Simple ds18b20 class
class DS18B20
{
public:
    DS18B20(int gpio)
    {
        pin = gpio;
    }

    float temp()
    {
        uint8_t arr[2] = {0};
        if (reset()) {
            wByte(0xCC);
            wByte(0x44);
            delay(750);
            reset();
            wByte(0xCC);
            wByte(0xBE);
            arr[0] = rByte();
            arr[1] = rByte();
            reset();
            return (float)(arr[0] + (arr[1] * 256)) / 16;
        }
        return 0;
    }
private:
    int pin;

    void write(uint8_t bit)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(5);
        if (bit)digitalWrite(pin, HIGH);
        delayMicroseconds(80);
        digitalWrite(pin, HIGH);
    }

    uint8_t read()
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(2);
        digitalWrite(pin, HIGH);
        delayMicroseconds(15);
        pinMode(pin, INPUT);
        return digitalRead(pin);
    }

    void wByte(uint8_t bytes)
    {
        for (int i = 0; i < 8; ++i) {
            write((bytes >> i) & 1);
        }
        delayMicroseconds(100);
    }

    uint8_t rByte()
    {
        uint8_t r = 0;
        for (int i = 0; i < 8; ++i) {
            if (read()) r |= 1 << i;
            delayMicroseconds(15);
        }
        return r;
    }

    bool reset()
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(500);
        digitalWrite(pin, HIGH);
        pinMode(pin, INPUT);
        delayMicroseconds(500);
        return digitalRead(pin);
    }
};


#endif // RICOU_IOT_H
