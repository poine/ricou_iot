#include <libwatering_ctl__cfg_sdesk.h>
#include <libwatering_ctl.h>

//uint32_t cnt = 0;
Network network;
Scheduler scheduler;


Mqtt mqtt(_MQTT_SERVER, _MQTT_PORT, scheduler);

void setup() {
  Serial.begin(115200);
  network.begin(false, true);
  scheduler.begin();
  scheduler.load();
  scheduler.print();
}

void loop() {
//  cnt += 1; 
//  char buf[64];
//  sprintf(buf, "loop: %d", cnt);
//  Serial.println(buf);

//  if (cnt==2)
//    scheduler.reset();
    
//  if (cnt==4)
//    scheduler.save();

//  if (cnt==6)
//    scheduler.load();
  
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.print(&timeinfo, "  %A, %B %d %Y %H:%M:%S  ");
  bool state = scheduler.get_state(&timeinfo);
  Serial.printf("state %s \n", state?"on":"off");
  mqtt.publish1(MQTT_CLIENT_ID, MQTT_TOPIC_GET_SCHEDULE, scheduler.get_schedule());
  mqtt.loop();
  delay(1000);
}
