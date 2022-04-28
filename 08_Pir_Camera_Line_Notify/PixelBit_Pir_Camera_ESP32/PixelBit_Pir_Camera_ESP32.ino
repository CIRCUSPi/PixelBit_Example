#include "FS.h"         // SD Card ESP32
#include "SD_MMC.h"     // SD Card ESP32
#include "SPI.h"
#include "config.h"
#include "esp_camera.h"
#include "esp_system.h"
#include <EEPROM.h>       // read and write from flash memory
#include <TFT_eSPI.h>     // Hardware-specific library
#include <TJpg_Decoder.h>
#include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include <tca5405.h>

#define CAMERA_MODEL_PIXEL_BIT
#include "camera_pins.h"

#define PIR_PIN 3
#define EEPROM_SIZE 1

hw_timer_t *timer          = NULL;
bool        pir_trig_state = false;
TCA5405     tca5405;
String      inputString    = "";        // a String to hold incoming data
bool        stringComplete = false;     // whether the string is complete
int         pictureNumber  = 0;

void IRAM_ATTR resetModule()
{
    ets_printf("reboot\n");
    esp_restart();
}

void pir_trig(void)
{
#if DEBUG
    Serial.println("pir trig");
#endif
    pir_trig_state = true;
}

void pir_idle(void)
{
#if DEBUG
    Serial.println("pir idle");
#endif
    pir_trig_state = false;
}

#define SSID "xxxxxxxx"
#define PASSWORD "xxxxxxxx"
#define LINE_TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

// Command Handler
CMD_T cmd_list[] = {
     {ATM_EVN_PIR_TRIG, strlen(ATM_EVN_PIR_TRIG), &pir_trig},
     {ATM_EVN_PIR_IDLE, strlen(ATM_EVN_PIR_IDLE), &pir_idle},
     {NULL,             0,                        NULL     }  //  keep this elemnet in the end of list
};

void setup()
{
    Serial.begin(UART_BAUDRATE);
#if defined(CAMERA_MODEL_PIXEL_BIT)
    tca5405.init(21);
    tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 0);
    tca5405.transmit();
    delay(100);
    tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 1);
    tca5405.transmit();
    delay(100);
#endif
    WiFi.begin(SSID, PASSWORD);
    Serial.printf("WiFi connecting to %s\n", SSID);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(400);
    }
    Serial.printf("\nWiFi connected\nIP : ");
    Serial.println(WiFi.localIP());
    LINE.setToken(LINE_TOKEN);

    timer = timerBegin(0, 80, true);     // timer 0, div 80Mhz
    timerAttachInterrupt(timer, &resetModule, true);
    timerAlarmWrite(timer, 40000000, false);     // set time in us 40s
    timerAlarmEnable(timer);                     // enable interrupt

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size   = FRAMESIZE_VGA;
        config.jpeg_quality = 10;
        config.fb_count     = 2;
    } else {
        config.frame_size   = FRAMESIZE_QQVGA;
        config.jpeg_quality = 12;
        config.fb_count     = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
}
void loop()
{
    timerWrite(timer, 0);     // feed WTD
    static int ii;

#if ESP_IDF_VERSION_MAJOR <= 3
    serialRead();
#endif
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
            Serial.println("command not found.");
            Serial.println(inputString);
#endif
        }

        // clear the string:
        inputString    = "";
        stringComplete = false;
    }
    if (stringComplete) {
        // clear the string:
        inputString    = "";
        stringComplete = false;
    }
    if (pir_trig_state) {
        Serial.print(ESP32_CMD_WS2812_ON);
        // Serial.print(ESP32_CMD_BUZZER_ON);
        Camera_capture();
        Serial.print(ESP32_CMD_WS2812_OFF);
        // Serial.print(ESP32_CMD_BUZZER_OFF);
        pir_trig_state = false;
    }
}

void Camera_capture()
{
    camera_fb_t *fb = NULL;
    fb              = esp_camera_fb_get();
    if (!fb) {
        // Serial.println("Camera capture failed");
        return;
    }

    LINE.notifyPicture("PixelBit_nono", fb->buf, fb->len);

    // Serial.println("Starting SD Card");
    uint8_t cardType = SD_MMC.cardType();
    if (SD_MMC.begin() && cardType != CARD_NONE) {
        // initialize EEPROM with predefined size
        EEPROM.begin(EEPROM_SIZE);
        pictureNumber = EEPROM.read(0) + 1;
        // Path where new picture will be saved in SD Card
        String  path = "/picture" + String(pictureNumber) + ".jpg";
        fs::FS &fs   = SD_MMC;
        // Serial.printf("Picture file name: %s\n", path.c_str());
        File file = fs.open(path.c_str(), FILE_WRITE);
        if (!file) {
            // Serial.println("Failed to open file in writing mode");
        } else {
            file.write(fb->buf, fb->len);     // payload (image), payload length
            // Serial.printf("Saved file to path: %s\n", path.c_str());
            EEPROM.write(0, pictureNumber);
            EEPROM.commit();
        }
        file.close();
    }
    esp_camera_fb_return(fb);
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
#if ESP_IDF_VERSION_MAJOR >= 4
void serialEvent()
#elif ESP_IDF_VERSION_MAJOR <= 3
void serialRead()
#endif
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
