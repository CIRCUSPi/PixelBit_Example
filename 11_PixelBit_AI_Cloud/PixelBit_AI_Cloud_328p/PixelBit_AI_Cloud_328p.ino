#include "config.h"
#include <CircusUart.h>

CircusUart uart(Serial);

void setup()
{
    Serial.begin(UART_BAUDRATE);
    pinMode(APIN_BUTTON_A, INPUT_PULLUP);
    pinMode(APIN_BUTTON_B, INPUT_PULLUP);
}

void loop()
{
    static bool preBtn_A_State = BTN_LVL_RELEASE;
    static bool preBtn_B_State = BTN_LVL_RELEASE;

    bool newBtn_A_State = digitalRead(APIN_BUTTON_A);
    if (newBtn_A_State != preBtn_A_State) {
        preBtn_A_State = newBtn_A_State;
        if (newBtn_A_State == BTN_LVL_PRESSED)
            uart.send(ATM_EVN_BTN_A_PRE);
        else
            uart.send(ATM_EVN_BTN_A_REL);
    }

    bool newBtn_B_State = digitalRead(APIN_BUTTON_B);
    if (newBtn_B_State != preBtn_B_State) {
        preBtn_B_State = newBtn_B_State;
        if (newBtn_B_State == BTN_LVL_PRESSED)
            uart.send(ATM_EVN_BTN_B_PRE);
        else
            uart.send(ATM_EVN_BTN_B_REL);
    }
}
