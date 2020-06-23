#include <WiFi.h>

const char* ssid = "Ricou";
const char* password = NULL;//"*********";

uint32_t cnt = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected to ");
  Serial.println(WiFi.localIP());
}

void loop() {
  cnt += 1; 
  char buf[64];
  sprintf(buf, "loop: %d", cnt);
  Serial.println(buf);
  delay(1000);
}
