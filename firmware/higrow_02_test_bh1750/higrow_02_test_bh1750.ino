//
//  Unit test for bh1750, lightmeter sensor
//

#include <BH1750.h>
#include <ricou_iot.h>

uint32_t cnt = 0;
BH1750 lightMeter(0x23); //0x23

void setup() {
  Serial.begin(115200);                // Enable and configure serial port

  pinMode(TTG_HG_POWER_CTRL, OUTPUT);  // Enable sensors power
  digitalWrite(TTG_HG_POWER_CTRL, 1);  //
  delay(1000);                         // Wait a bit for sensors to be powered on

  Wire.begin(TTG_HG_I2C_SDA, TTG_HG_I2C_SCL);
  
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 successfully initialized"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  } 
}

void loop() {
  cnt += 1; 
  float lux = lightMeter.readLightLevel();
  char buf[128];
  sprintf(buf, "loop: %d  bh1750 lux: %.1f", cnt, lux);
  Serial.println(buf);
  delay(1000);
}
