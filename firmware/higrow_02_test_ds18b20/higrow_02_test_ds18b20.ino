
//
//  Unit test for auxilary (soil) temperature sensor
//

#include <ricou_iot.h>

uint32_t cnt = 0;
DS18B20 thermometer(TTG_HG_DS18B20_PIN);

void setup() {
  Serial.begin(115200);                // Enable and configure serial port
  pinMode(TTG_HG_POWER_CTRL, OUTPUT);  // Enable sensors power
  digitalWrite(TTG_HG_POWER_CTRL, 1);  //
  delay(1000);                         // Wait a bit for sensors to be powered on
}

void loop() {
  cnt += 1; 
  float temp = thermometer.temp();
  char buf[64];
  sprintf(buf, "loop: %d  DS18B20: %f", cnt, temp);
  Serial.println(buf);
  delay(1000);
}
