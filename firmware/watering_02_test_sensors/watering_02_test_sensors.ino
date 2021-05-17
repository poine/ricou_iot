#include <libwatering_ctl.h>
#include <libwatering_ctl__cfg_sdesk.h>

Network network;
Sensors sensors;
Scheduler scheduler;
Mqtt mqtt(_MQTT_SERVER, _MQTT_PORT, scheduler);

void setup() {
  Serial.begin(115200);
  network.begin(false, true);
  sensors.begin();
}

void loop() {
  sensors.loop();  
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.print(&timeinfo, "  %A, %B %d %Y %H:%M:%S  ");
  float temp = sensors.get_temperature();
  Serial.printf("temp %.1f C\n", temp);
  delay(1000);
}
