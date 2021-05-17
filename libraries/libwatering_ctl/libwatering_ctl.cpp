
#include "libwatering_ctl.h"

//
// Web Dash
//
WebDash::WebDash():
  _server(80),
  _dashboard(&_server),
  _temperature(&_dashboard, TEMPERATURE_CARD, "Temperature", "°C"),
  _generic(&_dashboard, GENERIC_CARD, "Time"),
  _button(&_dashboard, BUTTON_CARD, "Pump") {
}

void WebDash::begin(void cbk(bool value)) {
  _server.begin();
  _button.attachCallback(cbk);
}

void WebDash::set_button(bool value) {
   _button.update(value);
   _dashboard.sendUpdates();
}


void WebDash::update(uint32_t cnt, float temp, struct tm* timeinfo) {
  _temperature.update(temp);//(int)random(0, 50));

  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", timeinfo);
  String txt(timeStringBuff);// = "foo";
  _generic.update(txt);//timeStringBuff);

  _dashboard.sendUpdates();
}
 


//
// Network
//
Network::Network():
  _eth_connected(false),
  _iwl_connected(false) {

}

extern void WiFiEvent(WiFiEvent_t event);

void Network::begin(bool start_eth, bool start_wifi) {
  if (start_eth) {ETH.begin();}
  WiFi.onEvent(WiFiEvent);
  if (start_wifi) {_setup_wifi();}
  _setup_ntp();
}

const char* hostname = "tank_terrasse";
const char* ssid = "Ricou";
const char* password = NULL;//"*********";

void Network::_setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected to ");
  Serial.println(WiFi.localIP());
}

void printLocalTime(struct tm* timeinfo) {
  Serial.print(timeinfo, "%A, %B %d %Y %H:%M:%S");
}


const char* ntpServer = "pool.ntp.org";
//const long  gmtOffset_sec = 3600; // france is UTC+1
//const int   daylightOffset_sec = 0000;//3600;
// france: see https://remotemonitoringsystems.ca/time-zone-abbreviations.php
const char* myTimezone = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

void Network::_setup_ntp() {
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  configTzTime(myTimezone, ntpServer);
}


bool Network::get_time(struct tm* timeinfo) {
  if(!getLocalTime(timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }
  return true;
}



static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname(hostname);//"esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      Serial.printf("uncaught WifiEvent %d\n", event);
      break;
  }
}

//
//
//
// library to read and write from flash memory
#include <EEPROM.h>
#define EEPROM_SIZE sizeof(struct SchedulerParam)

void print_schedule(struct SchedulerParam* p) {
  Serial.printf("nb periods %d\n", p->nb_period);
  for (int i=0; i<p->nb_period; i++) {
    Serial.printf("start %d dur %d\n", p->start_min[i], p->dur_min[i]);
  }
}

Scheduler::Scheduler():
  _override_start(-1),
  _override_dur(0),
  _force_stop(false) {

}


void Scheduler::reset() {
  Serial.println(" Setting default schedule");
  _params.nb_period = 2;
  _params.start_min[0] = 1;
  _params.dur_min[0] = 1;
  _params.start_min[1] = 3;
  _params.dur_min[1] = 1;

  uint8_t* buf = (uint8_t*)(&_params);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    Serial.print(byte(buf[i])); Serial.print(" ");
    if (i%16==15) Serial.println();
  }
  Serial.println();
}

void Scheduler::print() {
  print_schedule(&_params);
}


void Scheduler::begin() {
  Serial.print(" Scheduler eeprom size: "); Serial.println(EEPROM_SIZE);
  EEPROM.begin(EEPROM_SIZE);
}



void Scheduler::load() {
  //Serial.println(" bytes read from Flash . Values are:");
  uint8_t* buf = (uint8_t*)(&_params);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    uint8_t v = EEPROM.read(i);
    buf[i] = v;
    //Serial.print(byte(v)); Serial.print(" ");
    //if (i%16==15) Serial.println();
  }
  //Serial.println();
}

void Scheduler::save() {
  Serial.println(" writing bytes to Flash . Values are:");
  uint8_t* buf = (uint8_t*)(&_params);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    uint8_t v = buf[i];
    Serial.print(byte(v)); Serial.print(" ");
    if (i%16==15) Serial.println();
    EEPROM.write(i, v);
    EEPROM.commit();
  }
  Serial.println();
}


bool Scheduler::get_state(struct tm* timeinfo) {
  uint16_t min_nb = timeinfo->tm_min + 60*timeinfo->tm_hour;
  if (_force_stop)
    return false;
  if (_override_start >= 0 && min_nb >= _override_start)
    if (min_nb < _override_start+_override_dur)
      return true;
    else {
      _override_start = -1;
      _override_dur = 0;
    }
  //Serial.printf("  min nb: %d ", min_nb);
  //Serial.printf("  this: %x ", this);
  //Serial.println("Scheduler::get_state");
  //print_schedule(&_params);
  for (int i=0; i<_params.nb_period; i++) {
    //Serial.printf("  p: %d %d", _params.start_min[i], _params.start_min[i]+_params.dur_min[i]);
    if (min_nb >= _params.start_min[i] && min_nb < _params.start_min[i]+_params.dur_min[i])
      return true;
  }
  return false;
}

void Scheduler::set_schedule(struct SchedulerParam* p) {
  memcpy(&_params, p, sizeof(struct SchedulerParam));
  //Serial.printf("  this: %x ", this);
  //Serial.println("Scheduler::set_schedule");
  //print_schedule(&_params);
  save();
}


void Scheduler::override_schedule(struct tm* timeinfo, int16_t dur) {
  if (dur >=0) {
    uint16_t min_nb = timeinfo->tm_min + 60*timeinfo->tm_hour;
    _override_start = min_nb;
    _override_dur = dur;
    Serial.printf(" override_schedule: %d\n", min_nb);
  }
  else {
    Serial.printf(" clearing override_schedule\n");
    _override_start = -1;
    _override_dur = 0;
  }
}

void Scheduler::force_stop(bool state) {
  Serial.printf(" force_stop: %d ", state);
  _force_stop = state;
}
				  
//
//  Sensors
//
Sensors::Sensors():
  _one_wire(_DS18B20_PIN),
  _thermometer(&_one_wire)
{

}

void Sensors::begin() {
  _thermometer.begin();
}

void Sensors::loop() {
  _thermometer.requestTemperatures();
  _temperature = _thermometer.getTempC();
}

//
// Actuators
//
Actuators::Actuators() {}

void Actuators::begin() {
  pinMode(_PWR_SWTCH_PIN, OUTPUT);
  set_pump(false);
}
void Actuators::set_pump(bool how) {
  digitalWrite(_PWR_SWTCH_PIN, how);
}


//
// MQTT
//

using namespace std::placeholders;
Mqtt::Mqtt(const char* addr, uint16_t port, Scheduler& scheduler):
  _wifi_client(),
  _mqtt_client(addr, port, _wifi_client),
  _scheduler(scheduler) {
  _mqtt_client.setCallback(std::bind(&Mqtt::msg_callback, this, _1, _2, _3));
}



void Mqtt::msg_callback(char* topic, byte *payload, unsigned int length) {
  Serial.println("Mqtt::msg_callback");
  Serial.println(topic);
  if (!strcmp(topic, MQTT_TOPIC_SET_SCHEDULE)) {
    //assert(length == sizeof(struct SchedulerParam));
    struct SchedulerParam* P = (struct SchedulerParam*)payload;
    print_schedule(P);
    _scheduler.set_schedule(P);
  }
  else if (!strcmp(topic, MQTT_TOPIC_OVERRIDE)) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    int16_t foo = length>0 ? (int16_t)*payload : 1;
    Serial.println(foo);
    int16_t dur = length>0 ? 1 : 1; 
    _scheduler.override_schedule(&timeinfo, dur);
  }
  else if (!strcmp(topic, MQTT_TOPIC_FORCE_STOP)) {
    _scheduler.force_stop(length<=0);
  }
}

boolean Mqtt::connect(const char* mqtt_client_id) {
  bool res = _mqtt_client.connect(mqtt_client_id);
  if (res) {
    _mqtt_client.subscribe(MQTT_TOPIC_SET_SCHEDULE);
    Serial.printf("subscribed to %s\n", MQTT_TOPIC_SET_SCHEDULE);
    _mqtt_client.subscribe(MQTT_TOPIC_OVERRIDE);
    Serial.printf("subscribed to %s\n", MQTT_TOPIC_OVERRIDE);
    _mqtt_client.subscribe(MQTT_TOPIC_FORCE_STOP);
    Serial.printf("subscribed to %s\n", MQTT_TOPIC_FORCE_STOP);
  }
  return res;
}

void Mqtt::loop() {
  _mqtt_client.loop();
}

boolean Mqtt::publish_status(const char* topic, float temp) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  _scheduler.get_state(&timeinfo);
  uint8_t buff[128];
  memcpy(buff, &temp, sizeof(float));
  // memcpy(&buff[4], &vals->t12, sizeof(float));
  return _mqtt_client.publish(topic, buff, 4, false);
}

boolean Mqtt::publish_schedule(const char* topic, const struct SchedulerParam* p) {
  uint8_t* buff = (uint8_t*)p;
  return _mqtt_client.publish(topic, buff, sizeof(struct SchedulerParam), false);
}


boolean Mqtt::publish1(const char* mqtt_client_id, const char* topic, const struct SchedulerParam* p) {
  boolean connected = _mqtt_client.connected();
  if (!connected)
    connected = connect(mqtt_client_id);
  if (connected)
    return publish_schedule(topic, p);
  else return false;
}



