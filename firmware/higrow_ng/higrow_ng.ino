#include <ricou_iot.h>
//#include <ricou_iot_cfg_plant1.h>
//#include <ricou_iot_cfg_plant2.h>
//#include <ricou_iot_cfg_plant3.h>
#include <ricou_iot_cfg_plant4.h>


HigrowSensors sensors;
HigrowNetwork network;
HigrowMqtt mqtt(MQTT_SERVER, MQTT_PORT);
HigrowDash dash;

void setup() {
  Serial.begin(115200);
  sensors.setup();
  network.connect(WIFI_SSID, WIFI_PASSWD);
  dash.init();
}

void loop() {
  struct HigrowMeas* meas = sensors.read();
  mqtt.publish1(MQTT_CLIENT_ID, MQTT_TOPIC, meas);
  dash.update(meas);
  delay(1000);
}
