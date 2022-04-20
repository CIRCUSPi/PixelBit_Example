/*
  CIRCUS Pi PixelBit Example
  Dino Game - ESP32
  A - start
  B - jump
*/
#include <TFT_eSPI.h>
#include "gameover.h"
#include "noInternet.h"
#include "imgData.h"
#include "config.h"

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);
TFT_eSprite img2 = TFT_eSprite(&tft);
TFT_eSprite e = TFT_eSprite(&tft);
TFT_eSprite e2 = TFT_eSprite(&tft);

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

int dinoW = 33;
int dinoH = 35;
float linesX[6];
int linesW[6];
float linesX2[6];
int linesW2[6];
float clouds[2] = {(float)random(0, 80), (float)random(100, 180)};
float bumps[2];
int bumpsF[2];
int eW = 18;
int eH = 38;

float eX[2] = {(float)random(240, 310), (float)random(380, 460)};
int ef[2] = {0, 1};

float roll_speed = GAME_SPEED;
float cloudSpeed = 0.4;
int x = 30;
int y = 58;
float dir = -1.4;
int frames = 0;
int f = 0;
bool gameRun = 0;
int score = 0;
unsigned long start_t = 0;
int t = 0;
bool button_jump = 0;
bool button_start = 0;

void btn_b_pre(void) {
#if DEBUG
  Serial.println("bjump p");
#endif
  button_jump = true;
}

void btn_b_rel(void) {
#if DEBUG
  Serial.println("bjump r");
#endif
  // ignore this event, auto reset button_jump state
  //  button_jump = false;
}

void btn_a_pre(void) {
#if DEBUG
  Serial.println("bstart p");
#endif
  button_start = true;
}

void btn_a_rel(void) {
#if DEBUG
  Serial.println("a r");
#endif
  button_start = false;

}

// Command Handler
CMD_T cmd_list[] = {
  {ATM_EVN_BTNA_PRE,      strlen(ATM_EVN_BTNA_PRE),     &btn_a_pre},
  {ATM_EVN_BTNA_REL,      strlen(ATM_EVN_BTNA_REL),     &btn_a_rel},
  {ATM_EVN_BTNB_PRE,      strlen(ATM_EVN_BTNB_PRE),     &btn_b_pre},
  {ATM_EVN_BTNB_REL,      strlen(ATM_EVN_BTNB_REL),     &btn_b_rel},
  {NULL, 0, NULL} //keep this elemnet in the end of list
};

void setup() {
  // initialize serial:
  Serial.begin(UART_BAUDRATE);
  // reserve 200 bytes for the inputString:
  inputString.reserve(UART_BUFFER);

  tft.begin();
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(3);
  img.setTextColor(TFT_BLACK, TFT_WHITE);
  img.setColorDepth(1);
  img2.setColorDepth(1);
  e.setColorDepth(1);
  e2.setColorDepth(1);

  img.createSprite(240, 100);
  img2.createSprite(33, 35);
  e.createSprite(eW, eH);
  e2.createSprite(eW, eH);
  tft.fillScreen(TFT_WHITE);


  for (int i = 0; i < 6; i++) {
    linesX[i] = random(i * 40, (i + 1) * 40);
    linesW[i] = random(1, 14);
    linesX2[i] = random(i * 40, (i + 1) * 40);
    linesW2[i] = random(1, 14);
  }

  for (int n = 0; n < 2; n++) {
    bumps[n] = random(n * 90, (n + 1) * 120);
    bumpsF[n] = random(0, 2);
  }
  tft.pushImage(0, 0, 217, 135, noInternet);

}


void loop() {
  static int  ii;

#if ESP_IDF_VERSION_MAJOR <= 3
  serialRead();
#endif

  if (stringComplete) {
    for (ii = 0; cmd_list[ii].len != 0; ii++) {
      if (strcmp(cmd_list[ii].string, inputString.c_str()) == 0) {
        //execute command
        if (cmd_list[ii].handler != NULL) {
          (*cmd_list[ii].handler)();
        }
        break;
      }
    }

    if (cmd_list[ii].len == 0) {
      //can't find command
#if DEBUG
      Serial.println("command not found.");
      Serial.println(inputString);
#endif
    }

    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  if (gameRun == 1) {
    if (button_jump == 1) {
      f = 0;
    }

    if (button_jump == 1) {
      y = y + dir * roll_speed;
      if (y <= 2) {
        y = 2; 
        dir = dir * -1.00;
      } else if (y >= 58) {
        button_jump = 0;
        dir = dir * -1.00;
      }
    }

    if (frames < 8 && button_jump == 0) {
      f = 1;
    } if (frames > 8 && button_jump == 0) {
      f = 2;
    }

    drawS(x, y, f);
    frames++;
    if (frames == 16)
      frames = 0;

    checkColision();
  }

  if (button_start == 1) {
    button_start = 0;
    if (gameRun == 0) {
      gameRun = 1;
      start_t = millis();
      tft.fillScreen(TFT_WHITE);
      eX[0] = random(240, 310);
      eX[1] = random(380, 460);
      button_jump = 0;
      x = 30;
      y = 58;
      dir = -1.4;
      roll_speed = GAME_SPEED;
    }
  }


}

void drawS(int x, int y, int frame) {

  img.fillSprite(TFT_WHITE);
  img.drawLine(0, 84, 240, 84, TFT_BLACK);

  for (int i = 0; i < 6; i++) {
    img.drawLine(linesX[i], 87 , linesX[i] + linesW[i], 87, TFT_BLACK);
    linesX[i] = linesX[i] - roll_speed;
    if (linesX[i] < -14) {
      linesX[i] = random(245, 280);
      linesW[i] = random(1, 14);
    }
    img.drawLine(linesX2[i], 98 , linesX2[i] + linesW2[i], 98, TFT_BLACK);
    linesX2[i] = linesX2[i] - roll_speed;
    if (linesX2[i] < -14) {
      linesX2[i] = random(245, 280);
      linesW2[i] = random(1, 14);
    }
  }

  for (int j = 0; j < 2; j++) {
    img.drawXBitmap(clouds[j], 20, cloud, 38, 11, TFT_BLACK, TFT_WHITE);
    clouds[j] = clouds[j] - cloudSpeed;
    if (clouds[j] < -40)
      clouds[j] = random(244, 364);
  }

  for (int n = 0; n < 2; n++) {
    img.drawXBitmap(bumps[n], 80, bump[bumpsF[n]], 34, 5, TFT_BLACK, TFT_WHITE);
    bumps[n] = bumps[n] - roll_speed;
    if (bumps[n] < -40) {
      bumps[n] = random(244, 364);
      bumpsF[n] = random(0, 2);
    }
  }

  for (int m = 0; m < 2; m++) {

    eX[m] = eX[m] - roll_speed;
    if (eX[m] < -20)
      eX[m] = random(240, 300);
    ef[m] = random(0, 2);
  }

  e.drawXBitmap(0, 0, enemy[0], eW, eH, TFT_BLACK, TFT_WHITE);
  e2.drawXBitmap(0, 0, enemy[1], eW, eH, TFT_BLACK, TFT_WHITE);
  img2.drawXBitmap(0, 0, dino[frame], 33, 35, TFT_BLACK, TFT_WHITE);

  e.pushToSprite(&img, eX[0], 56, TFT_WHITE);
  e2.pushToSprite(&img, eX[1], 56, TFT_WHITE);
  img2.pushToSprite(&img, x, y, TFT_WHITE);

  score = (millis() - start_t) / 120;
  img.drawString(String(score), 204, 0, 2);
  //  img.drawString(String(roll_speed), 160, 0, 2);
  img.pushSprite(0, 17);

  if (score > t + GAME_SPEEDUP_SCORE) {
    t = score;
    roll_speed = roll_speed + GAME_SPEEDUP_GAP;
  }

}

void checkColision()
{

  for (int i = 0; i < 2; i++) {
    if (eX[i] < x + dinoW / 2 && eX[i] > x && y > 25) {
      gameRun = 0;
      tft.fillRect(0, 30, 240, 110, TFT_WHITE);
      tft.drawXBitmap(10, 30, gameover, 223, 100, TFT_BLACK, TFT_WHITE);
      delay(500);

    }
  }

}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
#if ESP_IDF_VERSION_MAJOR >= 4
void serialEvent()
#elif ESP_IDF_VERSION_MAJOR <= 3
void serialRead()
#endif
{
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
