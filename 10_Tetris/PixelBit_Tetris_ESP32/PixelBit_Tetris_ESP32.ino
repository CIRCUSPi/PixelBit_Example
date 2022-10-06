#include "CircusUart.h"
#include "config.h"
#include "tet.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

TFT_eSPI tft = TFT_eSPI();

uint16_t  BlockImage[8][12][12];           // Block
uint16_t  backBuffer[220][165];            // GAME AREA
const int Length                = 11;      // the number of pixels for a side of a block
const int Width                 = 15;      // the number of horizontal blocks
const int Height                = 20;      // the number of vertical blocks
int       screen[Width][Height] = {0};     // it shows color-numbers of all positions

struct Point {
    int X, Y;
};

struct Block {
    Point square[4][4];
    int   numRotate, color;
};

typedef enum
{
    ATM_BTN_PRE,
    ATM_BTN_REL,
} ATM_BTN_STATE_E;

Point pos;
Block block;
int   rot, fall_cnt = 0;
bool  started = false, gameover = false;

boolean but_A = false, but_LEFT = false, but_RIGHT = false;

int   game_speed = 20;     // 25msec
Block blocks[7]  = {
      {{{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}, {{0, -1}, {0, 0}, {0, 1}, {0, 2}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},       2, 1},
      {{{{0, -1}, {1, -1}, {0, 0}, {1, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},       1, 2},
      {{{{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, {{-1, 1}, {0, 1}, {0, 0}, {0, -1}}, {{-1, 0}, {0, 0}, {1, 0}, {1, 1}}, {{1, -1}, {0, -1}, {0, 0}, {0, 1}}}, 4, 3},
      {{{{-1, 0}, {0, 0}, {0, 1}, {1, 1}}, {{0, -1}, {0, 0}, {-1, 0}, {-1, 1}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},     2, 4},
      {{{{-1, 0}, {0, 0}, {1, 0}, {1, -1}}, {{-1, -1}, {0, -1}, {0, 0}, {0, 1}}, {{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}, {{0, -1}, {0, 0}, {0, 1}, {1, 1}}}, 4, 5},
      {{{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}, {{0, -1}, {0, 0}, {1, 0}, {1, 1}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},       2, 6},
      {{{{-1, 0}, {0, 0}, {1, 0}, {0, -1}}, {{0, -1}, {0, 0}, {0, 1}, {-1, 0}}, {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}, {{0, -1}, {0, 0}, {0, 1}, {1, 0}}},   4, 7}
};
extern uint8_t tetris_img[];
#define GREY 0x5AEB
int pom  = 0;
int pom2 = 0;
int pom3 = 0;
int pom4 = 0;

int score = 0;
int lvl   = 1;

CircusUart      uart(Serial);
ATM_BTN_STATE_E btn_b_state = ATM_BTN_REL;
ATM_BTN_STATE_E btn_a_state = ATM_BTN_REL;

void setup(void)
{
    Serial.begin(UART_BAUDRATE);

    uart.on(ATM_EVN_BTN_A_PRE, '\0', [](const char *temp) {
        btn_a_state = ATM_BTN_PRE;
    });

    uart.on(ATM_EVN_BTN_B_PRE, '\0', [](const char *temp) {
        btn_b_state = ATM_BTN_PRE;
    });

    uart.on(ATM_EVN_BTN_A_REL, '\0', [](const char *temp) {
        btn_a_state = ATM_BTN_REL;
    });

    uart.on(ATM_EVN_BTN_B_REL, '\0', [](const char *temp) {
        btn_b_state = ATM_BTN_REL;
    });

    tft.init();
    tft.setRotation(3);
    tft.setSwapBytes(true);

    if (!SPIFFS.begin()) {
        // DEBUG_PRINTLN(F("SPIFFS initialisation failed!"));
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED);
        tft.drawString(String("SPIFFS FAILED"), 30, 55, 4);
        while (1)
            yield();
    }
    // DEBUG_PRINTLN(F("\r\nSPIFFS Initialisation done."));

    // TJpgDec
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(onTJpgDecoded);

    tft.pushImage(52, 0, 135, 240, tet);
    delay(3000);
    // tft.fillScreen(TFT_BLACK);
    // TJpgDec.drawFsJpg(0, 0, "/tetris.jpg");
    // delay(3000);

    tft.fillScreen(TFT_BLACK);
    tft.drawLine(35, 19, 201, 19, GREY);
    tft.drawLine(35, 19, 35, 240, GREY);
    tft.drawLine(201, 19, 201, 240, GREY);

    tft.drawString("SCORE:" + String(score), 38, 8, 1);
    tft.drawString("LVL:" + String(lvl), 167, 8, 1);

    //----------------------------// Make Block ----------------------------
    make_block(0, TFT_BLACK);     // Type No, Color
    make_block(1, 0x00F0);        // DDDD     RED
    make_block(2, 0xFBE4);        // DD,DD    PUPLE
    make_block(3, 0xFF00);        // D__,DDD  BLUE
    make_block(4, 0xFF87);        // DD_,_DD  GREEN
    make_block(5, 0x87FF);        // __D,DDD  YELLO
    make_block(6, 0xF00F);        // _DD,DD_  LIGHT GREEN
    make_block(7, 0xF8FC);        // _D_,DDD  PINK
    //----------------------------------------------------------------------

    PutStartPos();     // Start Position
    for (int i = 0; i < 4; ++i)
        screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
    Draw();     // Draw block
}
//========================================================================
void loop()
{
    uart.loop();
    if (gameover) {
        if (btn_b_state == ATM_BTN_PRE) {
            for (int j = 0; j < Height; ++j)
                for (int i = 0; i < Width; ++i)
                    screen[i][j] = 0;
            gameover   = false;
            score      = 0;
            game_speed = 20;
            lvl        = 1;
            PutStartPos();     // Start Position
            for (int i = 0; i < 4; ++i)
                screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
            tft.drawString("SCORE:" + String(score), 38, 8, 1);
            tft.drawString("LVL:" + String(lvl), 167, 8, 1);
            Draw();
        }
        return;
    }

    static uint32_t timer = 0;
    if (gameover == false) {
        if (millis() > timer) {
            timer = millis() + game_speed;
            Point next_pos;
            int   next_rot = rot;
            GetNextPosRot(&next_pos, &next_rot);
            ReviseScreen(next_pos, next_rot);
            // delay(game_speed);
        }     // SPEED ADJUST
    }
}
//========================================================================
void Draw()
{     // Draw 120x240 in the center
    for (int i = 0; i < Width; ++i)
        for (int j = 0; j < Height; ++j)
            for (int k = 0; k < Length; ++k)
                for (int l = 0; l < Length; ++l)
                    backBuffer[j * Length + l][i * Length + k] = BlockImage[screen[i][j]][k][l];
    tft.pushImage(36, 20, 165, 220, *backBuffer);
}
//========================================================================
void PutStartPos()
{
    game_speed = 20;
    pos.X      = 4;
    pos.Y      = 1;
    block      = blocks[random(7)];
    rot        = random(block.numRotate);
}
//========================================================================
bool GetSquares(Block block, Point pos, int rot, Point *squares)
{
    bool overlap = false;
    for (int i = 0; i < 4; ++i) {
        Point p;
        p.X = pos.X + block.square[rot][i].X;
        p.Y = pos.Y + block.square[rot][i].Y;
        overlap |= p.X < 0 || p.X >= Width || p.Y < 0 || p.Y >= Height || screen[p.X][p.Y] != 0;
        squares[i] = p;
    }
    return !overlap;
}
//========================================================================
void GameOver()
{
    for (int i = 0; i < Width; ++i)
        for (int j = 0; j < Height; ++j)
            if (screen[i][j] != 0)
                screen[i][j] = 4;
    gameover = true;
}
//========================================================================
void ClearKeys()
{
    but_A     = false;
    but_LEFT  = false;
    but_RIGHT = false;
}
//========================================================================

bool KeyPadLoop()
{
    if (btn_b_state == ATM_BTN_REL && btn_a_state == ATM_BTN_PRE) {
        if (pom == 0) {
            pom = 1;
            ClearKeys();
            but_LEFT = true;
            return true;
        }
    } else {
        pom = 0;
    }

    if (btn_a_state == ATM_BTN_REL && btn_b_state == ATM_BTN_PRE) {
        if (pom2 == 0) {
            pom2 = 1;
            ClearKeys();
            but_RIGHT = true;
            return true;
        }
    } else {
        pom2 = 0;
    }

    // if (digitalRead(37) == 0) {
    //     if (pom3 == 0) {
    //         pom3 = 1;
    //         ClearKeys();
    //         but_A = true;
    //         return true;
    //     }
    // } else {
    //     pom3 = 0;
    // }

    if (btn_a_state == ATM_BTN_PRE && btn_b_state == ATM_BTN_PRE) {
        if (pom4 == 0) {
            pom4 = 1;
            ClearKeys();
            but_A = true;
            return true;
        }
    } else {
        pom4 = 0;
    }

    return false;
}
//========================================================================
void GetNextPosRot(Point *pnext_pos, int *pnext_rot)
{
    KeyPadLoop();

    if (but_LEFT)
        started = true;
    if (!started)
        return;
    pnext_pos->X = pos.X;
    pnext_pos->Y = pos.Y;
    if ((fall_cnt = (fall_cnt + 1) % 10) == 0)
        pnext_pos->Y += 1;
    else if (1) {
        if (but_LEFT) {
            but_LEFT = false;
            pnext_pos->X -= 1;
        } else if (but_RIGHT) {
            but_RIGHT = false;
            pnext_pos->X += 1;
        } else if (but_A) {
            but_A      = false;
            *pnext_rot = (*pnext_rot + block.numRotate - 1) % block.numRotate;
        }
    }
}
//========================================================================
void DeleteLine()
{
    for (int j = 0; j < Height; ++j) {
        bool Delete = true;
        for (int i = 0; i < Width; ++i)
            if (screen[i][j] == 0)
                Delete = false;
        if (Delete) {
            score++;
            if (score % 5 == 0) {
                lvl++;
                game_speed = game_speed - 4;
                tft.drawString("LVL:" + String(lvl), 151, 8, 1);
            }
            tft.drawString("SCORE:" + String(score), 77, 8, 1);
            for (int k = j; k >= 1; --k) {

                for (int i = 0; i < Width; ++i) {
                    screen[i][k] = screen[i][k - 1];
                }
            }
        }
    }
}
//========================================================================
void ReviseScreen(Point next_pos, int next_rot)
{
    if (!started)
        return;
    Point next_squares[4];
    for (int i = 0; i < 4; ++i)
        screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = 0;
    if (GetSquares(block, next_pos, next_rot, next_squares)) {
        for (int i = 0; i < 4; ++i) {
            screen[next_squares[i].X][next_squares[i].Y] = block.color;
        }
        pos = next_pos;
        rot = next_rot;
    } else {
        for (int i = 0; i < 4; ++i)
            screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
        if (next_pos.Y == pos.Y + 1) {
            DeleteLine();
            PutStartPos();
            if (!GetSquares(block, pos, rot, next_squares)) {
                for (int i = 0; i < 4; ++i)
                    screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
                GameOver();
            }
        }
    }
    Draw();
}
//========================================================================
void make_block(int n, uint16_t color)
{     // Make Block color
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++) {
            BlockImage[n][i][j] = color;     // Block color
            if (i == 0 || j == 0)
                BlockImage[n][i][j] = 0;     // TFT_BLACK Line
        }
}
//========================================================================
bool onTJpgDecoded(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= tft.height())
        return 0;
    // FIXME: Use DMA ?
    tft.pushImage(x, y, w, h, bitmap);
    return 1;
}
