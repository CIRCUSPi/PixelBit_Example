/*
  CIRCUS Pi PixelBit Example
  Pir Camera Line Notify - ATmega328p
*/
#include "config.h"
#include <FastLED.h>

#define NUM_LEDS 2

#if DEBUG
#include <SoftwareSerial.h>
SoftwareSerial mySerial(DEBUG_PIN_RX, DEBUG_PIN_TX);     // RX, TX
#endif

CRGB ws2812[NUM_LEDS];

void m_ws2812_on(void)
{
#if DEBUG
    Serial.println("trigFlash");
#endif
    ws2812[0] = CRGB::White;
    ws2812[1] = CRGB::White;
    FastLED.show();
}

void m_ws2812_off(void)
{
#if DEBUG
    Serial.println("trigFlash");
#endif
    ws2812[0] = CRGB::Black;
    ws2812[1] = CRGB::Black;
    FastLED.show();
}

void m_buzzer_on(void)
{
#if DEBUG
    Serial.println("buzzer_on");
#endif
    tone(APIN_BUZZER, 988);
}

void m_buzzer_off(void)
{
#if DEBUG
    Serial.println("buzzer_on");
#endif
    noTone(APIN_BUZZER);
}

// Command Handler
CMD_T cmd_list[] = {
     {ESP32_CMD_WS2812_ON,  strlen(ESP32_CMD_WS2812_ON),  &m_ws2812_on },
     {ESP32_CMD_WS2812_OFF, strlen(ESP32_CMD_WS2812_OFF), &m_ws2812_off},
     {ESP32_CMD_BUZZER_ON,  strlen(ESP32_CMD_BUZZER_ON),  &m_buzzer_on },
     {ESP32_CMD_BUZZER_OFF, strlen(ESP32_CMD_BUZZER_OFF), &m_buzzer_off},
     {NULL,                 0,                            NULL         }  //  keep this elemnet in the end of list
};

String inputString    = "";        // a String to hold incoming data
bool   stringComplete = false;     // whether the string is complete

void setup()
{
    // initialize serial:
    Serial.begin(UART_BAUDRATE);
    // reserve 200 bytes for the inputString:
    inputString.reserve(UART_BUFFER);

    pinMode(APIN_BUTTON_A, INPUT_PULLUP);
    pinMode(APIN_BUTTON_B, INPUT_PULLUP);
    pinMode(APIN_PIR, INPUT_PULLUP);

    FastLED.addLeds<NEOPIXEL, APIN_WS2812>(ws2812, NUM_LEDS);
    m_ws2812_off();

#if DEBUG
    // Set up a new SoftwareSerial object
    mySerial.begin(DEBUG_BAUDRATE);
#endif
}

void loop()
{
    static int ii;
    static int btn_a_stat = BTN_LVL_RELEASE, btn_b_stat = BTN_LVL_RELEASE, pir_stat = HIGH;
    serialEvent();
    // check the command
    if (stringComplete) {
        for (ii = 0; cmd_list[ii].len != 0; ii++) {
            if (strcmp(cmd_list[ii].string, inputString.c_str()) == 0) {
                // execute command
                if (cmd_list[ii].handler != NULL) {
                    (*cmd_list[ii].handler)();
                }
                break;
            }
        }

        if (cmd_list[ii].len == 0) {
            // can't find command
#if DEBUG
            mySerial.println("command not found.");
            mySerial.println(inputString);
#endif
        }

        // clear the string:
        inputString    = "";
        stringComplete = false;
    }

    // read button a status
    if ((digitalRead(APIN_BUTTON_A) == BTN_LVL_PRESSED) && (btn_a_stat == BTN_LVL_RELEASE)) {
        btn_a_stat = BTN_LVL_PRESSED;
        Serial.println(ATM_EVN_BTNA_PRE);
    } else if ((digitalRead(APIN_BUTTON_A) == BTN_LVL_RELEASE) && (btn_a_stat == BTN_LVL_PRESSED)) {
        btn_a_stat = BTN_LVL_RELEASE;
        Serial.println(ATM_EVN_BTNA_REL);
    }

    // read button b status
    if ((digitalRead(APIN_BUTTON_B) == BTN_LVL_PRESSED) && (btn_b_stat == BTN_LVL_RELEASE)) {
        btn_b_stat = BTN_LVL_PRESSED;
        Serial.println(ATM_EVN_BTNB_PRE);
    } else if ((digitalRead(APIN_BUTTON_B) == BTN_LVL_RELEASE) && (btn_b_stat == BTN_LVL_PRESSED)) {
        btn_b_stat = BTN_LVL_RELEASE;
        Serial.println(ATM_EVN_BTNB_REL);
    }

    // read pir status
    if ((digitalRead(APIN_PIR) == LOW) && (pir_stat == HIGH)) {
        pir_stat = LOW;
        Serial.println(ATM_EVN_PIR_TRIG);
    } else if ((digitalRead(APIN_PIR) == HIGH) && (pir_stat == LOW)) {
        pir_stat = HIGH;
        Serial.println(ATM_EVN_PIR_IDLE);
    }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent()
{
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar;
        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n') {
            stringComplete = true;
        }
    }
}
