
//
//  Unit test for bme280, relative humidity, barometric pressure and ambient temperature sensor
//
//  FIXME: does not work ( but i could not find the chip on my board, so that could be a reason... )
//         shame on you TTGO for not providing a schematic
//
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ricou_iot.h>

uint32_t cnt = 0;
Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);                // Enable and configure serial port
  Wire.begin(TTG_HG_I2C_SDA, TTG_HG_I2C_SCL);
  pinMode(TTG_HG_POWER_CTRL, OUTPUT);  // Enable sensors power
  digitalWrite(TTG_HG_POWER_CTRL, 1);  //
  delay(1000);                         // Wait a bit for sensors to be powered on

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  } 
  else {
    Serial.println("Found BME280 sensor");
  }
}

void loop() {
  cnt += 1; 
  float bme_temp = bme.readTemperature();
  float bme_pressure = (bme.readPressure() / 100.0F);
  float bme_altitude = bme.readAltitude(1013.25);
  char buf[128];
  sprintf(buf, "loop: %d  bme280 temp: %f pressure %f alt %f", cnt, bme_temp, bme_pressure, bme_altitude);
  Serial.println(buf);
  delay(1000);
}
