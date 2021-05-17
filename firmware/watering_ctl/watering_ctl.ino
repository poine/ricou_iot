#include <libwatering_ctl.h>
#include <libwatering_ctl__cfg_sdesk.h>

Network network;
Scheduler scheduler;
Sensors sensors;
Actuators actuators;
Mqtt mqtt(_MQTT_SERVER, _MQTT_PORT, scheduler);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Watering Ctl starting");
  network.begin(false, true);
  scheduler.begin();
  scheduler.load();
  scheduler.print();
  sensors.begin();
  actuators.begin();
}

void loop() {
  sensors.loop(); 
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.print(&timeinfo, "  %A, %B %d %Y %H:%M:%S  ");
  float temp = sensors.get_temperature();
  Serial.printf("temp %.1f C  ", temp);
  bool state = scheduler.get_state(&timeinfo);
  Serial.printf("pump %s \n", state?"on":"off");
  actuators.set_pump(state);
  mqtt.publish1(MQTT_CLIENT_ID, MQTT_TOPIC_GET_SCHEDULE, scheduler.get_schedule());
  mqtt.publish_status(MQTT_TOPIC_STATUS, temp);
  mqtt.loop();
  delay(1000);
}
