#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Initialise the TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Show text
  tft.setCursor(0, 0, 2); //(1, 2, 4, 6, 7, 8)
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextSize(2);     // 1~7
  tft.println("Hello pixel:bit.");
}

void loop() {
  
}
