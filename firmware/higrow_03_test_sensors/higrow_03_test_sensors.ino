#include <ricou_iot.h>

uint32_t cnt = 0;
HigrowSensors sensors;

void setup() {
  Serial.begin(115200);
  sensors.setup();
}

void loop() {
  cnt += 1; 
  struct HigrowMeas* meas = sensors.read();
  print_meas(cnt, meas);
  delay(1000);
}
