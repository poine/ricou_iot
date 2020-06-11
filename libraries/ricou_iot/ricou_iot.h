#ifndef RICOU_IOT_H
#define RICOU_IOT_H

#include <Arduino.h>

#include <WiFiMulti.h>
#include <PubSubClient.h>

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
#else  // USE_CHINESE_WEB
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
#endif // !USE_CHINESE_WEB


struct HigrowMeas {
  float lux;
  float t12;
  float h12;
  uint16_t soil;
  uint32_t salt;
  float bat;
};

class HigrowApp {
 public:
  HigrowApp();
};

class HigrowWebServer {
 public:
  HigrowWebServer();
};


//
// MQTT transactions
//
class HigrowMqtt {
 public:
  HigrowMqtt(const char*, uint16_t);
  boolean connect(const char* mqtt_client_id);
  boolean publish(const char* topic, struct HigrowMeas* vals);
  boolean publish1(const char* mqtt_client_id, const char* topic, struct HigrowMeas* vals);
 private:
  WiFiClient _wifi_client;
  PubSubClient _mqtt_client;
};


//
// DS18B20 driver
// Programmable Resolution 1-Wire Digital Thermometer 
// https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
//
class DS18B20
{
public:
  DS18B20(int gpio);
  int16_t read_temp();
  float   read_temp_as_float();
private:
    int pin;
    void    write(uint8_t bit);
    uint8_t read();
    void    wByte(uint8_t bytes);
    uint8_t rByte();
    bool    reset();
};


#endif // RICOU_IOT_H
