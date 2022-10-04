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
#define APIN_WHEEL_R_A MICRO_BIT_PIN_2
#define APIN_WHEEL_R_B MICRO_BIT_PIN_13
#define APIN_WHEEL_L_A MICRO_BIT_PIN_8
#define APIN_WHEEL_L_B MICRO_BIT_PIN_14

#define BTN_LVL_PRESSED LOW
#define BTN_LVL_RELEASE HIGH

#endif     //_CONFIG_H_
