#ifndef WATERING_CTL_H
#define WATERING_CTL_H

// Dash
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <time.h>

//
// Web Dash
//
class WebDash {
 public:
  WebDash();
  void begin(void cbk(bool value));
  void update(uint32_t cnt, float temp, struct tm* timeinfo);
  void set_button(bool value);
  
 private:
  AsyncWebServer _server;
  ESPDash _dashboard;
  Card _temperature, _generic, _button;
};


//
// Network
//

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12
#include <WiFi.h>
#include <ETH.h>

class Network {

 public:
  Network();
  void begin(bool start_eth=true, bool start_wifi=false);
  bool get_time(struct tm* timeinfo);
  
 private:
  void _setup_wifi();
  void _setup_ntp();
  bool _eth_connected;
  bool _iwl_connected;

};


//
// Our watering scheduler
//
#define MAX_WORKING_PERIOD 16
struct SchedulerParam {
  uint16_t nb_period;
  uint16_t start_min[MAX_WORKING_PERIOD];
  uint16_t dur_min[MAX_WORKING_PERIOD];
};


class Scheduler {
 public:
  Scheduler();
  void begin();
  void reset();
  void save();
  void load();
  bool get_state(struct tm* timeinfo);
  struct SchedulerParam* get_schedule() { return &_params;}
  void set_schedule(struct SchedulerParam* p);
  void print();
  void override_schedule(struct tm* timeinfo, int16_t dur);
  void force_stop(bool state);
  
 private:
  struct SchedulerParam _params;
  int16_t _override_start;
  uint16_t _override_dur;
  bool _force_stop;
};


//
// Our Sensors
//
#include <OneWire.h>
#include <DS18B20.h>
#define _DS18B20_PIN         2        // 18b20 data pin
class Sensors {
 public:
   Sensors();
   void begin();
   void loop();
   float get_temperature() { return _temperature;}
   
 private:
   OneWire _one_wire;
   DS18B20 _thermometer;
   float _temperature;
};

//
// Our actuators
//
#define _PWR_SWTCH_PIN 13
class Actuators {
 public:
  Actuators();
  void begin();
  void set_pump(bool how);
};

//
// MQTT
//
#include <WiFiMulti.h>
#include <PubSubClient.h>

// FIXME
#define MQTT_TOPIC_STATUS        "ricou/watering/terrasse_s/status"
#define MQTT_TOPIC_SET_SCHEDULE  "ricou/watering/terrasse_s/setup"
#define MQTT_TOPIC_GET_SCHEDULE  "ricou/watering/terrasse_s/schedule"
#define MQTT_TOPIC_OVERRIDE      "ricou/watering/terrasse_s/override"
#define MQTT_TOPIC_FORCE_STOP    "ricou/watering/terrasse_s/stop"
  
class Mqtt {
 public:
  Mqtt(const char* addr, uint16_t port, Scheduler& scheduler);
  boolean connect(const char* mqtt_client_id);
  boolean publish_status(const char* topic, float temp);
  boolean publish_schedule(const char* topic, const struct SchedulerParam* p);
  boolean publish1(const char* mqtt_client_id, const char* topic, const struct SchedulerParam* p);
  void loop();
  //void set_scheduler(const Scheduler& sch);
 private:
  void msg_callback(char* topic, byte *payload, unsigned int length);
  WiFiClient _wifi_client;
  PubSubClient _mqtt_client;
  Scheduler& _scheduler;
};

void printLocalTime(struct tm* timeinfo);


#endif // WATERING_CTL_H
