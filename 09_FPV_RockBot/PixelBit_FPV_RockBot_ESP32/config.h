#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG 0

#define WIFI_AP_MODE 0
#define WIFI_STA_MODE 1

#define WIFI_MODE WIFI_AP_MODE
#define WIFI_SSID "PixelBit_Rockbot"
#define WIFI_PASS "circuspi"

#define QRCODE_VERSION 3 /* 3: 29x29, 4: 33x33, 5: 37x37 */

#define UART_BAUDRATE 57600

/*
 * ESP32-ATmega328p UART Communication
 * data format: D_T_STRING:
 * D: Tx Side Device, E-ESP32
 *                    A-ATmega328P
 * T: Data Type, C-Command
 *
 * STRING: command name
 *    FORWARD
 *    BACK
 *    LEFT
 *    RIGHT
 */
#define ESP32_CMD_FORWARD "E_C_FORWARD"
#define ESP32_CMD_BACK "E_C_BACK"
#define ESP32_CMD_LEFT "E_C_LEFT"
#define ESP32_CMD_PAN_LEFT "E_C_PAN_LEFT"
#define ESP32_CMD_RIGHT "E_C_RIGHT"
#define ESP32_CMD_PAN_RIGHT "E_C_PAN_RIGHT"
#define ESP32_CMD_STOP "E_C_STOP"
#define ESP32_CMD_ANGLE "E_C_ANGLE"

#endif
