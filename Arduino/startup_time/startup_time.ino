long t;
void setup() {
  // put your setup code here, to run once:
  t = millis();
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.printf("%d ms\n", t);
  delay(1000);
}
