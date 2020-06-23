#include "ricou_iot.h"

void print_meas(uint32_t stamp, const struct HigrowMeas* meas) {
  char buf[128];
  const char* fmt = "loop: % 4d lux: %.1f temp: %.1f C hum: %.1f %% soil: %d salt: %d bat: %.0f";
  sprintf(buf, fmt, stamp, meas->lux, meas->t12, meas->h12, meas->soil, meas->salt, meas->bat);
  Serial.println(buf);
}

HigrowApp::HigrowApp() {

}


//
// Network
//

HigrowNetwork::HigrowNetwork() {
}

boolean HigrowNetwork::connect(const char* ssid, const char* passwd) {
  WiFi.begin(ssid, passwd);

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi connect fail!");
    return false;
  }
  else { 
    Serial.print("WiFi connected to ");
    Serial.println(WiFi.localIP());
    return true;
  } 
}



//
// Sensors
//
HigrowSensors::HigrowSensors():
  _light_meter(),
  _dht12(TTG_HG_DHT12_PIN, true)  // One wire mode
{}

void HigrowSensors::setup() {
  pinMode(TTG_HG_POWER_CTRL_PIN, OUTPUT); // Enable sensors power
  digitalWrite(TTG_HG_POWER_CTRL_PIN, 1);
  delay(10);                             // Wait a bit for sensors to be powered on
  Wire.begin(TTG_HG_I2C_SDA, TTG_HG_I2C_SCL);
 
  if (_light_meter.begin()) {
    Serial.println(F("BH1750: successfully initialized"));
  }
  else {
    Serial.println(F("BH1750: failed to initialize"));
  }

  _dht12.begin();
}

struct HigrowMeas* HigrowSensors::measurements() { return &_meas;}

struct HigrowMeas* HigrowSensors::read() {
  _meas.lux = _light_meter.readLightLevel();
  _meas.t12 = _dht12.readTemperature();
  _meas.h12 = _dht12.readHumidity();
  uint16_t soil_adc = analogRead(TTG_HG_SOIL_ADC_PIN);
  _meas.soil = map(soil_adc, 0, 4095, 100, 0);
  _read_salt();
  uint16_t bat_adc = analogRead(TTG_HG_BAT_ADC_PIN);
  _meas.bat = (float)bat_adc / 4095.0 * 2.0 * 3.3 * 1100;
  return &_meas;
}

void HigrowSensors::_read_salt() {
  const uint8_t samples = 120;
  uint32_t humi = 0;
  uint16_t array[samples];

  for (int i = 0; i < samples; i++) {
    array[i] = analogRead(TTG_HG_SALT_ADC_PIN);
    delay(2);
  }
  std::sort(array, array + samples);
  for (int i = 1; i < samples-1; i++) {
    //if (i == 0 || i == samples - 1)continue;
    humi += array[i];
  }
  humi /= samples - 2;
  _meas.salt = humi;
}

//
// Web Dash
//
HigrowDash::HigrowDash():
  _server(80) {
}

void HigrowDash::init() {
  ESPDash.init(_server);
  ESPDash.addTemperatureCard("temp2", TTG_HG_LABEL_TEMP2, 0, 0);
  ESPDash.addHumidityCard("hum2", TTG_HG_LABEL_HUM2, 0);
  ESPDash.addNumberCard("lux", TTG_HG_LABEL_LUX, 0);
  ESPDash.addHumidityCard("soil", TTG_HG_LABEL_SOIL, 0);
  ESPDash.addNumberCard("salt", TTG_HG_LABEL_SALT, 0);
  ESPDash.addNumberCard("batt", TTG_HG_LABEL_BAT, 0);
  _server.begin();
}

void HigrowDash::update(struct HigrowMeas* vals) {
  ESPDash.updateTemperatureCard("temp2", (int)vals->t12);
  ESPDash.updateHumidityCard("hum2", (int)vals->h12);
  ESPDash.updateNumberCard("lux", (int)vals->lux);
  ESPDash.updateHumidityCard("soil", (int)vals->soil);
  ESPDash.updateNumberCard("salt", (int)vals->salt);
  ESPDash.updateNumberCard("batt", (int)vals->bat);
}
 
//
// MQTT
//
HigrowMqtt::HigrowMqtt(const char* addr, uint16_t port):
  _wifi_client(),
  _mqtt_client(addr, port, _wifi_client) {

}

boolean HigrowMqtt::connect(const char* mqtt_client_id) {
  return _mqtt_client.connect(mqtt_client_id);
}

boolean HigrowMqtt::publish(const char* topic, struct HigrowMeas* vals) {
  uint8_t buff[128];
  memcpy(buff, &vals->lux, sizeof(float));
  memcpy(&buff[4], &vals->t12, sizeof(float));
  memcpy(&buff[8], &vals->h12, sizeof(float));
  memcpy(&buff[12], &vals->soil, sizeof(uint16_t));
  memcpy(&buff[16], &vals->salt, sizeof(uint32_t));
  memcpy(&buff[20], &vals->bat, sizeof(float));
  return _mqtt_client.publish(topic, buff, 24, false);
}

boolean HigrowMqtt::publish1(const char* mqtt_client_id, const char* topic, struct HigrowMeas* vals) {
  boolean connected = _mqtt_client.connected();
  if (!connected)
    connected = connect(mqtt_client_id);
  if (connected)
    return publish(topic, vals);
  else return false;
}



//
// DS18B20 thermometer
//
DS18B20::DS18B20(int gpio) {
  pin = gpio;
}

int16_t DS18B20::read_temp() {
  uint8_t arr[2] = {0};
  if (reset()) {
    wByte(0xCC);      // Skip Rom
    wByte(0x44);      // Convert T
    delay(750);
    reset();
    wByte(0xCC);      // Skip Rom
    wByte(0xBE);      // Read Scratchpad
    arr[0] = rByte();
    arr[1] = rByte();
    reset();
    return (arr[1] << 8) | arr[0];
  }
  return 0;
}

//#define DS18B20_RESOLUTION 0.0625
#define DS18B20_RESOLUTION 0.5
float DS18B20::read_temp_as_float() {
  return read_temp()*DS18B20_RESOLUTION;
}


void DS18B20::write(uint8_t bit) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(5);
  if (bit)digitalWrite(pin, HIGH);
  delayMicroseconds(80);
  digitalWrite(pin, HIGH);
}

uint8_t DS18B20::read() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(15);
  pinMode(pin, INPUT);
  return digitalRead(pin);
}

void DS18B20::wByte(uint8_t bytes) {
  for (int i = 0; i < 8; ++i) {
    write((bytes >> i) & 1);
  }
  delayMicroseconds(100);
}

uint8_t DS18B20::rByte() {
  uint8_t r = 0;
  for (int i = 0; i < 8; ++i) {
    if (read()) r |= 1 << i;
    delayMicroseconds(15);
  }
  return r;
}

bool DS18B20::reset() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(500);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
  delayMicroseconds(500);
  return digitalRead(pin);
}
