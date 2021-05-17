#include <libwatering_ctl.h>

Network network;
Sensors sensors;
Actuators actuators;

void setup() {
  Serial.begin(115200);
  network.begin(false, true);
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
  bool state = timeinfo.tm_sec >= 30;
  Serial.printf("state %d \n", state);
  actuators.set_pump(state);
  delay(1000);
}
