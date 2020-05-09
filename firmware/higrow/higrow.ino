// Build Options
//#define USE_CHINESE_WEB
//#define USE_SOFTAP_MODE
#define USE_18B20_TEMP_SENSOR
#define USE_MQTT

#include <algorithm>
#include <iostream>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <ESPmDNS.h>
#include <Button2.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT12.h>
#include <Adafruit_BME280.h>
#include <WiFiMulti.h>

#ifdef USE_MQTT
#include "esp_wifi.h"
#include <PubSubClient.h>
#endif

#include <ricou_iot.h>

// WIFI
#define WIFI_SSID   "Ricou"
#define WIFI_PASSWD "you wifi password"
// MQTT
const char* mqtt_server = "nina.lan";
const char* mqtt_topic = "ricou/plant/2";
const char* mqtt_clientID = "plant_sensor_2";






BH1750 lightMeter(0x23); //0x23
Adafruit_BME280 bmp;     //0x77
DHT12 dht12(TTG_HG_DHT12_PIN, true);
AsyncWebServer server(80);
Button2 button(TTG_HG_BOOT_PIN);
Button2 useButton(TTG_HG_USER_BUTTON);
WiFiMulti multi;
#ifdef USE_18B20_TEMP_SENSOR
DS18B20 temp18B20(TTG_HG_DS18B20_PIN);
#endif
#ifdef USE_MQTT
WiFiClient wifiClient;
PubSubClient mqtt_client(mqtt_server, 1883, wifiClient);
#endif



bool bme_found = false;

void smartConfigStart(Button2 &b)
{
    Serial.println("smartConfigStart...");
    WiFi.disconnect();
    WiFi.beginSmartConfig();
    while (!WiFi.smartConfigDone()) {
        Serial.print(".");
        delay(200);
    }
    WiFi.stopSmartConfig();
    Serial.println();
    Serial.print("smartConfigStop Connected:");
    Serial.print(WiFi.SSID());
    Serial.print("PSW: ");
    Serial.println(WiFi.psk());
}

void sleepHandler(Button2 &b)
{
    Serial.println("Enter Deepsleep ...");
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    delay(1000);
    esp_deep_sleep_start();
}


bool serverBegin()
{
    static bool isBegin = false;
    if (isBegin) {
        return true;
    }

    ESPDash.init(server);

    isBegin = true;
    if (MDNS.begin("soil")) {
        Serial.println("MDNS responder started");
    }
    // Add Respective Cards
    if (bme_found) {
      ESPDash.addTemperatureCard("temp", TTG_HG_LABEL_TEMP, 0, 0);
      ESPDash.addNumberCard("press", TTG_HG_LABEL_PRESS, 0);
      ESPDash.addNumberCard("alt", TTG_HG_LABEL_ALT, 0);
    }
    ESPDash.addTemperatureCard("temp2", TTG_HG_LABEL_TEMP2, 0, 0);
    ESPDash.addHumidityCard("hum2", TTG_HG_LABEL_HUM2, 0);
    ESPDash.addNumberCard("lux", TTG_HG_LABEL_LUX, 0);
    ESPDash.addHumidityCard("soil", TTG_HG_LABEL_SOIL, 0);
    ESPDash.addNumberCard("salt", TTG_HG_LABEL_SALT, 0);
    ESPDash.addNumberCard("batt", TTG_HG_LABEL_BAT, 0);
#ifdef USE_18B20_TEMP_SENSOR
    ESPDash.addTemperatureCard("temp3", TTG_HG_LABEL_TEMP3, 0, 0);
#endif
    server.begin();
    MDNS.addService("http", "tcp", 80);
    return true;
}

void setup()
{
    Serial.begin(115200);

#ifdef USE_SOFTAP_MODE
    Serial.println("123Configuring access point...");
    uint8_t mac[6];
    char buff[128];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    sprintf(buff, "123T-Higrow-%02X:%02X", mac[4], mac[5]);
    WiFi.softAP(buff);
#else
    WiFi.mode(WIFI_STA);
    wifi_config_t current_conf;
    esp_wifi_get_config(WIFI_IF_STA, &current_conf);
    int ssidlen = strlen((char *)(current_conf.sta.ssid));
    int passlen = strlen((char *)(current_conf.sta.password));
    if (ssidlen == 0) { // || passlen == 0) {
        multi.addAP(WIFI_SSID, WIFI_PASSWD);
        Serial.println("111Connect to defalut ssid, you can long press BOOT button enter smart config mode");
        while (multi.run() != WL_CONNECTED) {
            Serial.print('.');
        }
    } else {
        WiFi.begin();
    }
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi connect fail!,please restart retry,or long press BOOT button enter smart config mode\n");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
#endif
#ifdef USE_MQTT
    if (mqtt_client.connect(mqtt_clientID)) {
      Serial.println("Connected to MQTT Broker!");
    } 
    else {
      Serial.println("Connection to MQTT Broker failed...");
    }
#endif

    button.setLongClickHandler(smartConfigStart);
    useButton.setLongClickHandler(sleepHandler);

    Wire.begin(TTG_HG_I2C_SDA, TTG_HG_I2C_SCL);

    dht12.begin();

    //! Sensor power control pin , use deteced must set high
    pinMode(TTG_HG_POWER_CTRL_PIN, OUTPUT);
    digitalWrite(TTG_HG_POWER_CTRL_PIN, 1);
    delay(1000);

    if (!bmp.begin()) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
        bme_found = false;
    } else {
        bme_found = true;
    }

    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println(F("BH1750: successfully initialized"));
    } else {
        Serial.println(F("BH1750: failed to initialize"));
    }


}


uint32_t readSalt()
{
    uint8_t samples = 120;
    uint32_t humi = 0;
    uint16_t array[120];

    for (int i = 0; i < samples; i++) {
        array[i] = analogRead(TTG_HG_SALT_ADC_PIN);
        delay(2);
    }
    std::sort(array, array + samples);
    for (int i = 0; i < samples; i++) {
        if (i == 0 || i == samples - 1)continue;
        humi += array[i];
    }
    humi /= samples - 2;
    return humi;
}

uint16_t readSoil()
{
    uint16_t soil = analogRead(TTG_HG_SOIL_ADC_PIN);
    return map(soil, 0, 4095, 100, 0);
}

float readBattery()
{
    int vref = 1100;
    uint16_t volt = analogRead(TTG_HG_BAT_ADC_PIN);
    float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);
    return battery_voltage;
}

void loop()
{
    static uint64_t timestamp;
    button.loop();
    useButton.loop();
    if (millis() - timestamp > 1000 ) {
        timestamp = millis();
        // if (WiFi.status() == WL_CONNECTED) {
        if (serverBegin()) {
            float lux = lightMeter.readLightLevel();

            if (bme_found) {
                float bme_temp = bmp.readTemperature();
                float bme_pressure = (bmp.readPressure() / 100.0F);
                float bme_altitude = bmp.readAltitude(1013.25);
                ESPDash.updateTemperatureCard("temp", (int)bme_temp);
                ESPDash.updateNumberCard("press", (int)bme_pressure);
                ESPDash.updateNumberCard("alt", (int)bme_altitude);
            }

            float t12 = dht12.readTemperature();
            float h12 = dht12.readHumidity();


            if (!isnan(t12) && !isnan(h12) ) {
                ESPDash.updateTemperatureCard("temp2", (int)t12);
                ESPDash.updateHumidityCard("hum2", (int)h12);
            }
            ESPDash.updateNumberCard("lux", (int)lux);

            uint16_t soil = readSoil();
            uint32_t salt = readSalt();
            float bat = readBattery();
            ESPDash.updateHumidityCard("soil", (int)soil);
            ESPDash.updateNumberCard("salt", (int)salt);
            ESPDash.updateNumberCard("batt", (int)bat);

#ifdef USE_18B20_TEMP_SENSOR
            //Single data stream upload
            float temp = temp18B20.read_temp_as_float();
            ESPDash.updateTemperatureCard("temp3", (int)temp);
#endif
#ifdef USE_MQTT
            uint8_t foo[128];
            memcpy(foo, &lux, sizeof(float));
            memcpy(&foo[4], &t12, sizeof(float));
            memcpy(&foo[8], &h12, sizeof(float));
            memcpy(&foo[12], &soil, sizeof(uint16_t));
            memcpy(&foo[16], &salt, sizeof(uint32_t));
            memcpy(&foo[20], &bat, sizeof(float));
            mqtt_client.publish(mqtt_topic, foo, 24, false);
#endif
       }
    }
}
