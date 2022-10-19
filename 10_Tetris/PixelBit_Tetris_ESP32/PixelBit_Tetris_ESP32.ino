/**
 * @file PixelBit_Tetris_ESP32.ino
 * @author Zack Huang (zackhuang0513@gmail.com)
 * @brief PixelBit Tetris Game
 * @version 1.0.0
 * @date 2022-10-06
 *
 * @copyright Copyright (c) 2022
 *
 */
/* #region  include */
#include "CircusUart.h"
#include "config.h"
#include "tet.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
/* #endregion */

/* #region  buff */
uint16_t BlockImage[Block_NUM][Block_SIEZ][Block_SIEZ];     // 8 種積 Pixel，包含分隔線
uint16_t backBuffer[Height * Length][Width * Length];       // 遊戲區塊 Pixel，[Height*Length][Width*Length]
int      screen[Width][Height] = {0};                       // 存放積木區塊顏色 index (積木格數)
/* #endregion */

/* #region  建立 7 個積木形狀、各種方向、顏色 index */
Block_t blocks[7] = {
     {{{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}, {{0, -1}, {0, 0}, {0, 1}, {0, 2}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},       2, 1}, // 長條型
     {{{{0, -1}, {1, -1}, {0, 0}, {1, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},       1, 2}, // 正方形
     {{{{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, {{-1, 1}, {0, 1}, {0, 0}, {0, -1}}, {{-1, 0}, {0, 0}, {1, 0}, {1, 1}}, {{1, -1}, {0, -1}, {0, 0}, {0, 1}}}, 4, 3}, //
     {{{{-1, 0}, {0, 0}, {0, 1}, {1, 1}}, {{0, -1}, {0, 0}, {-1, 0}, {-1, 1}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},     2, 4},
     {{{{-1, 0}, {0, 0}, {1, 0}, {1, -1}}, {{-1, -1}, {0, -1}, {0, 0}, {0, 1}}, {{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}, {{0, -1}, {0, 0}, {0, 1}, {1, 1}}}, 4, 5},
     {{{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}, {{0, -1}, {0, 0}, {1, 0}, {1, 1}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},       2, 6},
     {{{{-1, 0}, {0, 0}, {1, 0}, {0, -1}}, {{0, -1}, {0, 0}, {0, 1}, {-1, 0}}, {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}, {{0, -1}, {0, 0}, {0, 1}, {1, 0}}},   4, 7}
};
/* #endregion */

/* #region  動態變數 */
Point_t pos;       // 當前積木座標
Block_t block;     // 當前積木
int     rot     = 0;
bool    started = false, gameover = false;

boolean btn_AB     = false;               // 觸發積木 旋轉
boolean btn_LEFT   = false;               // 觸發積木 往左
boolean btn_RIGHT  = false;               // 觸發積木 往右
int     game_speed = GAME_INIT_SPEED;     // 下降速度

// 紀錄按鍵狀態，避免重複觸發
// TODO: 將按鍵觸發狀態細分，並交由 328P 處理
int pom  = 0;
int pom2 = 0;
int pom3 = 0;

int score = 0;
int lvl   = 1;

ATM_BTN_STATE_E btn_b_state = ATM_BTN_REL;
ATM_BTN_STATE_E btn_a_state = ATM_BTN_REL;
/* #endregion */

/* #region  Object */
TFT_eSPI   tft = TFT_eSPI();
CircusUart uart(Serial);
/* #endregion */

/* #region  Arduino Setup */
void setup(void)
{
    Serial.begin(UART_BAUDRATE);

    /* #region  註冊按鍵事件 */
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
    /* #endregion */

    /* #region  初始化 TFT */
    tft.init();
    tft.setRotation(3);
    tft.setSwapBytes(true);

    /* #endregion */
    /* #region  初始化 SPIFFS */
    if (!SPIFFS.begin()) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED);
        tft.drawString(String("SPIFFS FAILED"), 30, 55, 4);
        while (1)
            yield();
    }
    /* #endregion */

    /* #region  設定 TJpgDec 比例、解碼 Callback*/
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(onTJpgDecoded);
    /* #endregion */

    /* #region  顯示遊戲開機畫面 */
    tft.pushImage(52, 0, 135, 240, tet);
    delay(3000);
    /* #endregion */

    /* #region TODO: 顯示遊戲左右 */
    // tft.fillScreen(TFT_BLACK);
    // TJpgDec.drawFsJpg(0, 0, "/tetris.jpg");
    // delay(3000);
    /* #endregion */

    /* #region  繪製遊戲邊框 */
    tft.fillScreen(TFT_BLACK);
    tft.drawLine(35, 19, 201, 19, GREY);
    tft.drawLine(35, 19, 35, 240, GREY);
    tft.drawLine(201, 19, 201, 240, GREY);
    /* #endregion */

    /* #region  繪製積木顏色 */
    make_block(0, TFT_BLACK);     // Type No, Color
    make_block(1, 0x00F0);        // DDDD     RED
    make_block(2, 0xFBE4);        // DD,DD    PUPLE
    make_block(3, 0xFF00);        // D__,DDD  BLUE
    make_block(4, 0xFF87);        // DD_,_DD  GREEN
    make_block(5, 0x87FF);        // __D,DDD  YELLO
    make_block(6, 0xF00F);        // _DD,DD_  LIGHT GREEN
    make_block(7, 0xF8FC);        // _D_,DDD  PINK
    /* #endregion */
    initGame();
}
/* #endregion */

/* #region  Arduino Loop */
void loop()
{
    static uint32_t update_timer = 0;
    // polling ATmega328P even
    uart.loop();

    if (gameover && btn_b_state == ATM_BTN_PRE) {
        initGame();
        return;
    }

    if (!gameover) {
        if (millis() > update_timer) {
            Point_t next_pos;
            int     next_rot = rot;
            GetNextPosRot(&next_pos, &next_rot);
            update_timer = millis() + 20;
            ReviseScreen(next_pos, next_rot);
        }
    }
}
/* #endregion */

/* #region  更新 backBuffer，繪製 backBuffer 到 TFT */
void Draw()
{                                                    // Draw 120x240 in the center
    for (int i = 0; i < Width; ++i)                  // 水平尋訪 square
        for (int j = 0; j < Height; ++j)             // 垂直尋訪 square
            for (int k = 0; k < Length; ++k)         // 垂直尋訪 square 中 Pixel
                for (int l = 0; l < Length; ++l)     // 水平尋訪 square 中 Pixel
                                                     // 設定 backBuffer 每一點像素言顏色
                    backBuffer[j * Length + l][i * Length + k] = BlockImage[screen[i][j]][k][l];
    // 顯示 backBuffer 到 TFT
    tft.pushImage(36, 20, 165, 220, *backBuffer);
}
/* #endregion */

/* #region  初始化遊戲積木 */
void PutStartPos()
{
    game_speed = GAME_INIT_SPEED;
    pos.X      = 7;                           // 初始化積木 X 座標，遊戲區正中間 Width/2
    pos.Y      = 1;                           // 初始化積木 Y 座標，遊戲區最上方
    block      = blocks[random(7)];           // 隨機取積木
    rot        = random(block.numRotate);     // 隨機設定方向
}
/* #endregion */

/* #region  檢查是否重疊或超出邊界 */
bool GetSquares(Block_t block, Point_t pos, int rot, Point_t *squares)
{
    bool overlap = false;
    for (int i = 0; i < 4; ++i) {
        Point_t p;
        p.X = pos.X + block.square[rot][i].X;
        p.Y = pos.Y + block.square[rot][i].Y;
        overlap |= p.X < 0 || p.X >= Width || p.Y < 0 || p.Y >= Height || screen[p.X][p.Y] != 0;
        squares[i] = p;
    }
    return !overlap;
}
/* #endregion */

/* #region  遊戲結束、將所有積木設為統一顏色 */
void GameOver()
{
    // 將所有區塊
    for (int i = 0; i < Width; ++i)
        for (int j = 0; j < Height; ++j)
            if (screen[i][j] != 0)
                screen[i][j] = 4;
    gameover = true;
}
/* #endregion */

/* #region  清除按鍵狀態 flag */
void ClearKeys()
{
    btn_AB    = false;
    btn_LEFT  = false;
    btn_RIGHT = false;
}
/* #endregion */

/* #region  根據按鍵狀態設定動作 flag */
bool KeyPadLoop()
{
    // 按 A 放 B
    if (btn_b_state == ATM_BTN_REL && btn_a_state == ATM_BTN_PRE) {
        if (pom == 0) {
            pom = 1;
            ClearKeys();
            btn_LEFT = true;
            return true;
        }
    } else {
        pom = 0;
    }
    // 按 B 放 A
    if (btn_a_state == ATM_BTN_REL && btn_b_state == ATM_BTN_PRE) {
        if (pom2 == 0) {
            pom2 = 1;
            ClearKeys();
            btn_RIGHT = true;
            return true;
        }
    } else {
        pom2 = 0;
    }
    // 按 A、B
    if (btn_a_state == ATM_BTN_PRE && btn_b_state == ATM_BTN_PRE) {
        if (pom3 == 0) {
            pom3 = 1;
            ClearKeys();
            btn_AB = true;
            return true;
        }
    } else {
        pom3 = 0;
    }

    return false;
}
/* #endregion */

/* #region  取得下一個積木位置、旋轉方向 */
void GetNextPosRot(Point_t *pnext_pos, int *pnext_rot)
{
    static uint32_t timer = 0;

    KeyPadLoop();

    if (btn_LEFT)
        // 遊戲開始
        started = true;
    if (!started)
        // 遊戲已結束
        return;

    pnext_pos->X = pos.X;
    pnext_pos->Y = pos.Y;

    if (millis() > timer) {
        timer = millis() + game_speed;
        pnext_pos->Y += 1;
    }

    if (btn_LEFT) {
        btn_LEFT = false;
        // 往左一格
        pnext_pos->X -= 1;
    } else if (btn_RIGHT) {
        btn_RIGHT = false;
        // 往右一格
        pnext_pos->X += 1;
    } else if (btn_AB) {
        btn_AB = false;
        // 往左旋轉
        *pnext_rot = (*pnext_rot + block.numRotate - 1) % block.numRotate;
    }
}
/* #endregion */

/* #region  檢查並消除整行 */
void ChkDeleteLine()
{
    // 尋訪 screen row
    for (int j = 0; j < Height; ++j) {
        bool Delete = true;
        //
        // 尋訪 screen col，檢整行是否都有積木
        for (int i = 0; i < Width; ++i)
            if (screen[i][j] == 0)
                Delete = false;
        if (Delete) {
            // 增加分數
            score++;
            // 難度升級，下降速度加快
            if (score % UpgradeThreshold == 0) {
                lvl++;
                game_speed = game_speed - SpeedReduction;
                tft.drawString("LVL:" + String(lvl), 167, 8, 1);
            }
            tft.drawString("SCORE:" + String(score), 38, 8, 1);
            // 從下到上更新積木
            for (int k = j; k >= 1; --k) {
                for (int i = 0; i < Width; ++i) {
                    screen[i][k] = screen[i][k - 1];
                }
            }
        }
    }
}
/* #endregion */

/* #region  修改 Screen */
void ReviseScreen(Point_t next_pos, int next_rot)
{
    if (!started)
        return;
    Point_t next_squares[4];
    // 清除積木四個區塊顏色
    for (int i = 0; i < 4; ++i)
        screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = 0;

    if (GetSquares(block, next_pos, next_rot, next_squares)) {
        // 無重疊或超出邊界
        for (int i = 0; i < 4; ++i) {
            screen[next_squares[i].X][next_squares[i].Y] = block.color;
        }
        pos = next_pos;
        rot = next_rot;
    } else {
        // 重疊或超出邊界
        // 回填積木四個區塊顏色
        for (int i = 0; i < 4; ++i)
            screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
        // 檢查積木 Y 座標是否到底
        if (next_pos.Y == pos.Y + 1) {
            ChkDeleteLine();
            PutStartPos();
            if (!GetSquares(block, pos, rot, next_squares)) {
                // 設定新積木
                for (int i = 0; i < 4; ++i)
                    screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
                // 積木已重疊，遊戲結束
                GameOver();
            }
        }
    }
    Draw();
}
/* #endregion */

/* #region  繪製積木顏色、分隔線 */
void make_block(int n, uint16_t color)
{     // Make Block_t color
    for (int i = 0; i < Block_SIEZ; i++)
        for (int j = 0; j < Block_SIEZ; j++) {
            BlockImage[n][i][j] = color;
            if (i == 0 || j == 0) {
                // 設定正方形區隔線
                BlockImage[n][i][j] = 0;
            }
        }
}
/* #endregion */

/* #region  初始化遊戲 */
void initGame()
{
    // 清除 screen 內容
    for (int j = 0; j < Height; ++j)
        for (int i = 0; i < Width; ++i)
            screen[i][j] = 0;
    // 變數初始化
    gameover   = false;
    score      = 0;
    game_speed = GAME_INIT_SPEED;
    lvl        = 1;
    // 產生新積木
    PutStartPos();
    /*  根據當前旋轉方向(rot)選擇 Block_t 內其中一種方向積木，
        取得 X 座標(block.square[rot][i].X)加上 X 開始座標(pos.X)，
        取得 Y 座標(block.square[rot][i].Y)加上 Y 開始座標(pos.Y)，
        設定積木顏色 index 到積木空間 buff(screen)內
    */
    for (int i = 0; i < 4; ++i)
        screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
    // 繪製分數、難度等級
    tft.drawString("SCORE:" + String(score), 38, 8, 1);
    tft.drawString("LVL:" + String(lvl), 167, 8, 1);

    // 繪製所有積木到 TFT
    Draw();
}
/* #endregion */

/* #region  jpg 解碼完成 Callback */
bool onTJpgDecoded(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= tft.height())
        return 0;
    // FIXME: Use DMA ?
    tft.pushImage(x, y, w, h, bitmap);
    return 1;
}
/* #endregion */
