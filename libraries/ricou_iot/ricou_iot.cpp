#include "ricou_iot.h"

HigrowApp::HigrowApp() {

}

HigrowWebServer::HigrowWebServer() {

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
// 
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
