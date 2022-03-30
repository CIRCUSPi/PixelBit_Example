#include "esp_camera.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <WiFi.h>

/* #region  Select camera model */
//#define CAMERA_MODEL_M5STACK_ESP32CAM
#define CAMERA_MODEL_PIXEL_BIT
/* #endregion */

#include "camera_pins.h"
#if defined(CAMERA_MODEL_PIXEL_BIT)
#include "tca5405.h"
#endif

/* #region  WiFi Config */
const char *ssid     = "*********";
const char *password = "*********";
/* #endregion */

/* #region  Object */
TCA5405 tca5405;
/* #endregion */

void startCameraServer();

void setup()
{
#if defined(CAMERA_MODEL_PIXEL_BIT)
    tca5405.init(21);
    tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 0);
    tca5405.transmit();
    delay(100);
    tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 1);
    tca5405.transmit();
    delay(100);
#endif

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

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
    config.frame_size   = FRAMESIZE_240X240;

    // init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        config.jpeg_quality = 10;
        config.fb_count     = 2;
    } else {
        config.jpeg_quality = 12;
        config.fb_count     = 1;
    }

#if defined(CAMERA_MODEL_PIXEL_BIT)
    pinMode(25, INPUT_PULLUP);
    pinMode(26, INPUT_PULLUP);
    pinMode(27, INPUT_PULLUP);
    pinMode(32, INPUT_PULLUP);
    pinMode(33, INPUT_PULLUP);
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // drop down frame size for higher initial frame rate
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_240X240);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    startCameraServer();

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect, the stream is on a different port channel 81 ");
    Serial.print("stream Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println(":81/stream ");
    Serial.print("image Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("/capture ");
}

void loop()
{
    // put your main code here, to run repeatedly:
    delay(10000);
}
