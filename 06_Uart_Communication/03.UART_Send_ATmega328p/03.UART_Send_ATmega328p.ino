int text = 1;

void setup() {
  Serial.begin(57600);

}

void loop() {
  Serial.println(text);
  text++;
  if (text > 100) text = 1;
  delay(1000);
}
