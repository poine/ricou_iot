#ifndef RICOU_IOT_CFG_PLANT1_H
#define RICOU_IOT_CFG_PLANT1_H

// Build Options
//#define USE_CHINESE_WEB
//#define USE_SOFTAP_MODE
//#define USE_18B20_TEMP_SENSOR
#define USE_MQTT

// WIFI
#define WIFI_SSID      "Ricou"
#define WIFI_PASSWD    NULL

// MQTT
#define MQTT_SERVER    "nhop.lan"
#define MQTT_PORT      1883
#define MQTT_TOPIC     "ricou/plant/1"
#define MQTT_CLIENT_ID "plant_sensor_1"


#endif // RICOU_IOT_CFG_PLANT1_H
