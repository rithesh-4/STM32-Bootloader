void setup() {
  Serial.begin(115200);
  Serial.println("NodeMCU Ready");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    Serial.print(c);
  }
}