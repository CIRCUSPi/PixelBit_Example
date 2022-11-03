/*
  CIRCUS Pi PixelBit Example
  FPV RC Car - ATmega328p
*/
#include "config.h"

#if DEBUG
#include <SoftwareSerial.h>
SoftwareSerial mySerial(DEBUG_PIN_RX, DEBUG_PIN_TX);     // RX, TX
#endif

void m_forward(void)
{
#if DEBUG
    Serial.println("forward");
#endif
    digitalWrite(APIN_WHEEL_R_A, HIGH);
    digitalWrite(APIN_WHEEL_R_B, LOW);
    digitalWrite(APIN_WHEEL_L_A, HIGH);
    digitalWrite(APIN_WHEEL_L_B, LOW);
}

void m_back(void)
{
#if DEBUG
    Serial.println("back");
#endif

    digitalWrite(APIN_WHEEL_R_A, LOW);
    digitalWrite(APIN_WHEEL_R_B, HIGH);
    digitalWrite(APIN_WHEEL_L_A, LOW);
    digitalWrite(APIN_WHEEL_L_B, HIGH);
}

void m_left(void)
{
#if DEBUG
    Serial.println("left");
#endif

    digitalWrite(APIN_WHEEL_R_A, HIGH);
    digitalWrite(APIN_WHEEL_R_B, LOW);
    digitalWrite(APIN_WHEEL_L_A, LOW);
    digitalWrite(APIN_WHEEL_L_B, LOW);
}

void m_right(void)
{
#if DEBUG
    Serial.println("right");
#endif
    digitalWrite(APIN_WHEEL_R_A, LOW);
    digitalWrite(APIN_WHEEL_R_B, LOW);
    digitalWrite(APIN_WHEEL_L_A, HIGH);
    digitalWrite(APIN_WHEEL_L_B, LOW);
}

void m_lspin(void)
{
#if DEBUG
    Serial.println("left spin");
#endif

    digitalWrite(APIN_WHEEL_R_A, HIGH);
    digitalWrite(APIN_WHEEL_R_B, LOW);
    digitalWrite(APIN_WHEEL_L_A, LOW);
    digitalWrite(APIN_WHEEL_L_B, HIGH);
}

void m_rspin(void)
{
#if DEBUG
    Serial.println("right spin");
#endif
    digitalWrite(APIN_WHEEL_R_A, LOW);
    digitalWrite(APIN_WHEEL_R_B, HIGH);
    digitalWrite(APIN_WHEEL_L_A, HIGH);
    digitalWrite(APIN_WHEEL_L_B, LOW);
}

void m_stop(void)
{
#if DEBUG
    Serial.println("stop");
#endif
    digitalWrite(APIN_WHEEL_R_A, LOW);
    digitalWrite(APIN_WHEEL_R_B, LOW);
    digitalWrite(APIN_WHEEL_L_A, LOW);
    digitalWrite(APIN_WHEEL_L_B, LOW);
}

// Command Handler
CMD_T cmd_list[] = {
     {ESP32_CMD_FORWARD, strlen(ESP32_CMD_FORWARD), &m_forward},
     {ESP32_CMD_BACK,    strlen(ESP32_CMD_BACK),    &m_back   },
     {ESP32_CMD_LEFT,    strlen(ESP32_CMD_LEFT),    &m_left   },
     {ESP32_CMD_RIGHT,   strlen(ESP32_CMD_RIGHT),   &m_right  },
     {ESP32_CMD_LSPIN,   strlen(ESP32_CMD_LSPIN),   &m_lspin  },
     {ESP32_CMD_RSPIN,   strlen(ESP32_CMD_RSPIN),   &m_rspin  },
     {ESP32_CMD_STOP,    strlen(ESP32_CMD_STOP),    &m_stop   },
     {NULL,              0,                         NULL      }  //  keep this elemnet in the end of list
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

    pinMode(APIN_WHEEL_R_A, OUTPUT);
    pinMode(APIN_WHEEL_R_B, OUTPUT);
    pinMode(APIN_WHEEL_L_A, OUTPUT);
    pinMode(APIN_WHEEL_L_B, OUTPUT);

#if DEBUG
    // Set up a new SoftwareSerial object
    mySerial.begin(DEBUG_BAUDRATE);
#endif
}

void loop()
{
    static int ii;
    static int btn_a_stat = BTN_LVL_RELEASE, btn_b_stat = BTN_LVL_RELEASE;

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
