#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG true
#define DEBUG_TIMER false

#define UART_BAUDRATE 57600

#if DEBUG_TIMER
#define DEBUG_PRIMT_TIMER(x) Serial.print(x)
#define DEBUG_PRIMTLN_TIMER(x) Serial.println(x)
#else
#define DEBUG_PRIMT_TIMER(x)
#define DEBUG_PRIMTLN_TIMER(x)
#endif

#if DEBUG
#define DEBUG_PRIMT(x) Serial.print(x)
#define DEBUG_PRIMTLN(x) Serial.println(x)
#else
#define DEBUG_PRIMT(x)
#define DEBUG_PRIMTLN(x)
#endif

#define ATM_EVN_BTN_A_PRE "BTN_A_PRE"
#define ATM_EVN_BTN_B_PRE "BTN_B_PRE"
#define ATM_EVN_BTN_A_REL "BTN_A_REL"
#define ATM_EVN_BTN_B_REL "BTN_B_REL"

#define USER_AGENT "PixelBit"
#define WIFI_SSID "XXXXXXXX"
#define WIFI_PASS "XXXXXXXX"

#define API_HOST "XXXXXXXX"
#define Prediction_ID "XXXXXXXX"
#define Iterations_ID "XXXXXXXX"
#define Prediction_Key "XXXXXXXX"

#define Prediction_SIZE 20

typedef enum
{
    READY,
    IDLE,
} STATE_E;

typedef struct {
    bool   valid;              // 此筆資料是否有效
    float  probability;        // 信心度
    String tagName;            // 標籤名稱
    struct boundingBox_t {     // boundingBox 左上座標、寬、高
        float left;
        float top;
        float width;
        float height;
    } boundingBox_t;
} Prediction_t;

#endif
