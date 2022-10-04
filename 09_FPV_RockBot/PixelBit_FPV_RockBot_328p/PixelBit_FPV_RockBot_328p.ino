#include "config.h"
#include <Arduino.h>
#include <CircusUart.h>
#include <Wire.h>

CircusUart uart(Serial);

int8_t speed = 50;

void setup()
{
    Serial.begin(UART_BAUDRATE);
    Wire.begin();

    uart.on(ESP32_CMD_FORWARD, '\0', [](const char *temp) {
        Move_direction(speed, speed, speed, speed);
    });

    uart.on(ESP32_CMD_BACK, '\0', [](const char *temp) {
        Move_direction(-speed, -speed, -speed, -speed);
    });

    uart.on(ESP32_CMD_LEFT, '\0', [](const char *temp) {
        Move_direction(-speed, speed, -speed, speed);
    });

    uart.on(ESP32_CMD_PAN_LEFT, '\0', [](const char *temp) {
        Move_direction(-speed, speed, speed, -speed);
    });

    uart.on(ESP32_CMD_RIGHT, '\0', [](const char *temp) {
        Move_direction(speed, -speed, speed, -speed);
    });

    uart.on(ESP32_CMD_PAN_RIGHT, '\0', [](const char *temp) {
        Move_direction(speed, -speed, -speed, speed);
    });

    uart.on(ESP32_CMD_STOP, '\0', [](const char *temp) {
        Move_direction(0, 0, 0, 0);
    });

    uart.on(ESP32_CMD_ANGLE, ':', [](const char *angle) {
        Servo_angle(1, (uint8_t)atoi(angle));
    });
}

void loop()
{
    uart.loop();
}

void Send_iic(uint8_t Register, uint8_t Speed)
{
    Wire.beginTransmission(0x38);
    Wire.write(Register);
    Wire.write(Speed);
    Wire.endTransmission();
}

void Move_direction(int8_t S1, int8_t S2, int8_t S3, int8_t S4)
{
    Send_iic(0x00, S1);
    Send_iic(0x01, S2);
    Send_iic(0x02, S3);
    Send_iic(0x03, S4);
}

void Servo_angle(uint8_t Servo_ch, uint8_t degree)
{
    Send_iic((Servo_ch - 1) | 0x10, int(degree));
}
