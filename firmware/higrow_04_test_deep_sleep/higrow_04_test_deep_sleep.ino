#include <ricou_iot_cfg_plant1.h>
#include <ricou_iot.h>

#include <WiFi.h>

#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR uint32_t boot_cnt = 0;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup() {
  Serial.begin(115200);
  boot_cnt += 1;
  Serial.println("Boot number: " + String(boot_cnt));
  print_wakeup_reason();

  HigrowSensors sensors;
  sensors.setup();

  HigrowNetwork network;
  if (network.connect(WIFI_SSID, WIFI_PASSWD)) {
    struct HigrowMeas* meas = sensors.read();
    HigrowMqtt mqtt(MQTT_SERVER, MQTT_PORT);
    mqtt.publish1(MQTT_CLIENT_ID, MQTT_TOPIC, meas);
    print_meas(boot_cnt, meas);
  }
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*1e6);
  Serial.println("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP) + " seconds"); 
  Serial.println("Going to sleep now");
  //delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  //Serial.println("This will never be printed");
}

void loop() {}
