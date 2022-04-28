#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG 0

#define UART_BAUDRATE 57600
#define UART_BUFFER 200
#define DEBUG_BAUDRATE 19200
#define DEBUG_PIN_TX A2
#define DEBUG_PIN_RX 8
/*
 * ESP32-ATmega328p UART Communication
 * data format: D_T_STRING:
 * D: Tx Side Device, E-ESP32
 *                    A-ATmega328P
 * T: Data Type, C-Command
 *
 * STRING: command name
 */
#define ESP32_CMD_WS2812_ON "E_C_WS2812_ON\r\n"
#define ESP32_CMD_WS2812_OFF "E_C_WS2812_OFF\r\n"
#define ESP32_CMD_BUZZER_ON "E_C_BUZZER_ON\r\n"
#define ESP32_CMD_BUZZER_OFF "E_C_BUZZER_OFF\r\n"

#define ATM_EVN_BTNA_PRE "A_E_BTN_A_PRE\r\n"     // button a pressed event
#define ATM_EVN_BTNA_REL "A_E_BTN_A_REL\r\n"     // button a release event
#define ATM_EVN_BTNB_PRE "A_E_BTN_B_PRE\r\n"     // button b pressed event
#define ATM_EVN_BTNB_REL "A_E_BTN_B_REL\r\n"     // button b release event
#define ATM_EVN_PIR_TRIG "A_E_PIR_TIRG\r\n"      // pir trig event
#define ATM_EVN_PIR_IDLE "A_E_PIR_IDLE\r\n"      // pir idle event

#define MICRO_BIT_PIN_0 3
#define MICRO_BIT_PIN_1 A0
#define MICRO_BIT_PIN_2 A1
#define MICRO_BIT_PIN_3 A2
#define MICRO_BIT_PIN_4 A3
#define MICRO_BIT_PIN_5 4
#define MICRO_BIT_PIN_6 6
#define MICRO_BIT_PIN_7 7
#define MICRO_BIT_PIN_8 2
#define MICRO_BIT_PIN_9 8
#define MICRO_BIT_PIN_10 A7
#define MICRO_BIT_PIN_11 5
#define MICRO_BIT_PIN_12 9
#define MICRO_BIT_PIN_13 13
#define MICRO_BIT_PIN_14 12
#define MICRO_BIT_PIN_15 11
#define MICRO_BIT_PIN_16 10
#define MICRO_BIT_PIN_19 A5
#define MICRO_BIT_PIN_20 A4

#define APIN_BUTTON_A MICRO_BIT_PIN_5
#define APIN_BUTTON_B MICRO_BIT_PIN_11
#define APIN_PIR MICRO_BIT_PIN_6
#define APIN_WS2812 MICRO_BIT_PIN_12
#define APIN_BUZZER MICRO_BIT_PIN_0

#define BTN_LVL_PRESSED LOW
#define BTN_LVL_RELEASE HIGH

typedef void (*func_t)(void);
typedef struct {
    char  *string;
    int    len;
    func_t handler;
} CMD_T;

#endif     //_CONFIG_H_
