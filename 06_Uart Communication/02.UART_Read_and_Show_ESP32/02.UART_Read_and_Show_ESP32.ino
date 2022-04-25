#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

String inputString = "";

void setup() {
  // initialize serial:
  Serial.begin(57600);

  // Initialise the TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  if (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;

    if (inChar == '\n') {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);  tft.setTextSize(7);
      tft.println(inputString);
      inputString = "";
    }
  }
}
