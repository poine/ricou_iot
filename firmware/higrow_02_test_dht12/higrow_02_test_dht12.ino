
//
//  Unit test for DHT12 Digital temperature and humidity sensor
//

#include <DHT12.h>
#include <ricou_iot.h>

uint32_t cnt = 0;
DHT12 dht12(TTG_HG_DHT12_PIN, true);   // One wire mode

void setup() {
  Serial.begin(115200);                // Enable and configure serial port
  dht12.begin();                       // Initialize sensor
  pinMode(TTG_HG_POWER_CTRL, OUTPUT);  // Enable sensors power
  digitalWrite(TTG_HG_POWER_CTRL, 1);  //
  delay(1000);                         // Wait a bit for sensors to be powered on
}

void loop() {
  cnt += 1; 
  float t12 = dht12.readTemperature();
  float h12 = dht12.readHumidity();
  char buf[128];
  sprintf(buf, "loop: %d  dht12 temp: %.1f C humidity %.1f %%", cnt, t12, h12);
  Serial.println(buf);
  delay(1000);
}
