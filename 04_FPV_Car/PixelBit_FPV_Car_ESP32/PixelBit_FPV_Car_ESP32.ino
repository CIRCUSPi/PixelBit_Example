/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-cam-projects-ebook/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include "Arduino.h"
#include "config.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "qrcode.h"
#include "soc/rtc_cntl_reg.h"     // disable brownout problems
#include "soc/soc.h"              // disable brownout problems
#include "webPage.h"
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <tca5405.h>

TFT_eSPI tft = TFT_eSPI();
TCA5405  tca5405;

// Create the QR code
QRCode qrcode;
char   qrcode_str[128];

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
#define CAMERA_MODEL_PIXEL_BIT     // Has PSRAM

#include "camera_pins.h"

// Replace with your network credentials
const char *ssid     = WIFI_SSID;
const char *password = WIFI_PASS;

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb           = NULL;
    esp_err_t    res          = ESP_OK;
    size_t       _jpg_buf_len = 0;
    uint8_t     *_jpg_buf     = NULL;
    char        *part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            if (fb->width > 400) {
                if (fb->format != PIXFORMAT_JPEG) {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted) {
                        Serial.println("JPEG compression failed");
                        res = ESP_FAIL;
                    }
                } else {
                    _jpg_buf_len = fb->len;
                    _jpg_buf     = fb->buf;
                }
            }
        }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res         = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (fb) {
            esp_camera_fb_return(fb);
            fb       = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK) {
            break;
        }
        // Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
    }
    return res;
}

static esp_err_t cmd_handler(httpd_req_t *req)
{
    char  *buf;
    size_t buf_len;
    char   variable[32] = {
           0,
    };

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int res = 0;

    if (!strcmp(variable, "forward")) {
        Serial.print(ESP32_CMD_FORWARD);
    } else if (!strcmp(variable, "left")) {
        Serial.print(ESP32_CMD_LEFT);
    } else if (!strcmp(variable, "right")) {
        Serial.print(ESP32_CMD_RIGHT);
    } else if (!strcmp(variable, "left_spin")) {
        Serial.print(ESP32_CMD_LSPIN);
    } else if (!strcmp(variable, "right_spin")) {
        Serial.print(ESP32_CMD_RSPIN);
    } else if (!strcmp(variable, "backward")) {
        Serial.print(ESP32_CMD_BACK);
    } else if (!strcmp(variable, "stop")) {
        Serial.print(ESP32_CMD_STOP);
    } else {
        res = -1;
    }

    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port    = 80;
    httpd_uri_t index_uri = {.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL};

    httpd_uri_t cmd_uri    = {.uri = "/action", .method = HTTP_GET, .handler = cmd_handler, .user_ctx = NULL};
    httpd_uri_t stream_uri = {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL};
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
    }
    config.server_port += 1;
    config.ctrl_port += 1;
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}

String inputString    = "";        // a String to hold incoming data
bool   stringComplete = false;     // whether the string is complete

void draw_qrcode()
{
    const int offset_x = 18;
    const int offset_y = 4;
    const int scale    = 7;

    for (int y = 0; y < qrcode.size; y++) {
        for (int x = 0; x < qrcode.size; x++) {
            int newX = offset_x + (x * scale);
            int newY = offset_y + (y * scale);

            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect(newX, newY, scale, scale, TFT_BLACK);
            } else {
                tft.fillRect(newX, newY, scale, scale, TFT_WHITE);
            }
        }
    }
}

void setup()
{
    //  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

#if defined(CAMERA_MODEL_PIXEL_BIT)
    tca5405.init(21);
    tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 0);
    tca5405.transmit();
    delay(100);
    tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 1);
    tca5405.transmit();
    delay(100);
#endif

    // initialize serial:
    Serial.begin(UART_BAUDRATE);
    // reserve 200 bytes for the inputString:
    inputString.reserve(UART_BUFFER);

    // Initialise the TFT
    tft.begin();
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_WHITE);
    tft.setRotation(3);

    tft.setTextSize(1);
    tft.setTextColor(TFT_BLUE, TFT_WHITE);

    sprintf(qrcode_str, "WIFI:S:%s;T:WPA;P:%s;;", ssid, password);
    uint8_t qrcodeData[qrcode_getBufferSize(QRCODE_VERSION)];
    qrcode_initText(&qrcode, qrcodeData, QRCODE_VERSION, ECC_LOW, qrcode_str);

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
        config.frame_size   = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count     = 1;
    }

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    sensor_t *s = esp_camera_sensor_get();
    s->set_hmirror(s, 1);

    // Wi-Fi connection
#if (WIFI_MODE == WIFI_STA_MODE)
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    IPAddress myIP = WiFi.localIP();
    Serial.println("");
    Serial.println("WiFi connected");

    Serial.print("Camera Stream Ready! Go to: http://");

#elif (WIFI_MODE == WIFI_AP_MODE)
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");

#else
    ## #ERROR : Please check wifi mode setting
#endif
    Serial.println(myIP);
    // Start streaming web server
    startCameraServer();

    draw_qrcode();
    char ip_str[16];
    sprintf(ip_str, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
    tft.drawString(ip_str, 30, 210, 4);
}

void loop()
{
#if ESP_IDF_VERSION_MAJOR <= 3
    serialRead();
#endif

    if (stringComplete) {
        // clear the string:
        inputString    = "";
        stringComplete = false;
    }
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
