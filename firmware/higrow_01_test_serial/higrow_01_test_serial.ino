
uint32_t cnt = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  cnt += 1; 
  char buf[64];
  sprintf(buf, "loop: %d", cnt);
  Serial.println(buf);
  delay(1000);
}
