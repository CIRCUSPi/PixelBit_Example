#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG 0

// #define WIFI_AP_MODE 0
// #define WIFI_STA_MODE 1

// #define WIFI_MODE WIFI_AP_MODE
// #define WIFI_SSID "PixelBit"
// #define WIFI_PASS "circuspi"

// #define QRCODE_VERSION 3 /* 3: 29x29, 4: 33x33, 5: 37x37 */

#define UART_BAUDRATE 57600

#define ATM_EVN_BTN_A_PRE "BTN_A_PRE"
#define ATM_EVN_BTN_B_PRE "BTN_B_PRE"
#define ATM_EVN_BTN_A_REL "BTN_A_REL"
#define ATM_EVN_BTN_B_REL "BTN_B_REL"

// Game setting
// 積木種類數量
#define Block_NUM 8
// ?
#define Block_SIEZ 12
// 積木正方形邊長 (Pixel)，影響 backBuffer
#define Length 11
// 遊戲區塊可容納積木 寬度 (積木格數)
#define Width 15
// 遊戲區塊可容納積木 高度 (積木格數)
#define Height 20
// 遊戲初始下降速度 (ms)
#define GAME_INIT_SPEED 200
// 每消 N 行難度增加
#define UpgradeThreshold 5
// 每升一等下降速度加快 N ms
#define SpeedReduction 40

// color
#define GREY 0x5AEB

/* #region square 座標結構 */
typedef struct {
    int X, Y;
} Point_t;
/* #endregion */

/* #region 積木結構 */
typedef struct {
    // 單顆積木占用4格，單顆積木最多四種方向
    Point_t square[4][4];
    // 旋轉?
    int numRotate;
    // 顏色
    int color;
} Block_t;
/* #endregion */

/* #region  ATmega328P 按鈕狀態列舉 */
typedef enum
{
    // 按下
    ATM_BTN_PRE,
    // 放開
    ATM_BTN_REL,
} ATM_BTN_STATE_E;
/* #endregion */

#endif
