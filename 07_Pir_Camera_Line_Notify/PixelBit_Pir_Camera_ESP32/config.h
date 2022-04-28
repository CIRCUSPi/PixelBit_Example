#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG 0
#define WIFI_STA_MODE 0

#define UART_BAUDRATE 57600
#define UART_BUFFER 200

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

typedef void (*func_t)(void);
typedef struct {
    char  *string;
    int    len;
    func_t handler;
} CMD_T;

#endif
