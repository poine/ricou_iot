//
// In progress
// was testing mqtt disconnect

#include <WiFi.h>
#include "esp_wifi.h"
#include <PubSubClient.h>


//Wifi
const char* ssid = "Ricou";
const char* password = NULL;//"*********";
// MQTT
const char* mqtt_server = "nina.lan";//"nhop.lan";
const char* mqtt_topic = "ricou/test";
const char* mqtt_clientID = "plant_sensor_1";

uint32_t cnt = 0;
WiFiClient wifiClient;
PubSubClient mqtt_client(mqtt_server, 1883, wifiClient);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected to ");
  Serial.println(WiFi.localIP());

  if (mqtt_client.connect(mqtt_clientID)) {
    Serial.println("Connected to MQTT Broker!");
  } 
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  
}

void loop() {
  cnt += 1;
  char* buff1 = "hello";
  if (!mqtt_client.connected()) {
    if (mqtt_client.connect(mqtt_clientID)) {
        Serial.println("Connected to MQTT Broker!");
    } 
    else {
      Serial.println("Connection to MQTT Broker failed...");
    }
  }
  if (mqtt_client.publish(mqtt_topic, buff1)) {
  }
  else {
    Serial.println("mqtt publish failed");
  }
   
  char buf[64];
  sprintf(buf, "loop: %d", cnt);
  Serial.println(buf);
  delay(1000);
}
