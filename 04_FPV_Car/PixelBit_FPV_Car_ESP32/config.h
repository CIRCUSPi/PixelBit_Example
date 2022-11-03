#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG               0

#define WIFI_AP_MODE        0
#define WIFI_STA_MODE       1

#define WIFI_MODE           WIFI_AP_MODE
#define WIFI_SSID           "PixelBit_MoonCar"
#define WIFI_PASS           "circuspi"

#define QRCODE_VERSION      3   /* 3: 29x29, 4: 33x33, 5: 37x37 */

#define UART_BAUDRATE       57600
#define UART_BUFFER         200

#define MOTOR_1_PIN_1       14
#define MOTOR_1_PIN_2       15
#define MOTOR_2_PIN_1       13
#define MOTOR_2_PIN_2       12

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
#define ESP32_CMD_FORWARD   "E_C_FORWARD\r\n"
#define ESP32_CMD_BACK      "E_C_BACK\r\n"
#define ESP32_CMD_LEFT      "E_C_LEFT\r\n"
#define ESP32_CMD_RIGHT     "E_C_RIGHT\r\n"
#define ESP32_CMD_LSPIN     "E_C_LSPIN\r\n"
#define ESP32_CMD_RSPIN     "E_C_RSPIN\r\n"
#define ESP32_CMD_STOP      "E_C_STOP\r\n"

#define ATM_EVN_BTNA_PRE    "A_E_BTN_A_PRE\r\n"   // button a pressed event
#define ATM_EVN_BTNA_REL    "A_E_BTN_A_REL\r\n"   // button a release event
#define ATM_EVN_BTNB_PRE    "A_E_BTN_B_PRE\r\n"   // button b pressed event
#define ATM_EVN_BTNB_REL    "A_E_BTN_B_REL\r\n"   // button b release event


typedef void (*func_t) (void);
typedef struct  {
  char    *string;
  int     len;
  func_t  handler;
} CMD_T;

#endif
