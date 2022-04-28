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
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      if (inputString == "Red\r\n") {
        tft.setTextColor(TFT_RED, TFT_BLACK);  tft.setTextSize(7);
      }
      else if (inputString == "Green\r\n") {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);  tft.setTextSize(7);
      }
      else if (inputString == "Blue\r\n") {
        tft.setTextColor(TFT_BLUE, TFT_BLACK);  tft.setTextSize(7);
      }
      else {
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);  tft.setTextSize(7);
      }
      tft.println("Color");
      inputString = "";
    }
  }
}
