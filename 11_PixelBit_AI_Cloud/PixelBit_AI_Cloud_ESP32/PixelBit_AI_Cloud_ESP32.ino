#include "config.h"
#include "esp_camera.h"
#include <CircusUart.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <tca5405.h>

#define CAMERA_MODEL_PIXEL_BIT
#include "camera_pins.h"

#define PIR_PIN 3
#define EEPROM_SIZE 1

uint16_t  dmaBuffer1[16 * 16];     // Toggle buffer for 16*16 MCU block, 512bytes
uint16_t  dmaBuffer2[16 * 16];     // Toggle buffer for 16*16 MCU block, 512bytes
uint16_t *dmaBufferPtr = dmaBuffer1;
bool      dmaBufferSel = 0;

uint8_t      prediction_idx = 0;
Prediction_t predictions[Prediction_SIZE];

bool btnA_state = false;
bool btnB_state = false;

int count = 0;

TCA5405          tca5405;
CircusUart       uart(Serial);
TFT_eSPI         tft = TFT_eSPI();
WiFiClientSecure _clientSecure;

inline String readStringUntil(char terminator, uint32_t timeout = 5);

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

    uart.on(ATM_EVN_BTN_A_PRE, '\0', [](const char *temp) {
        btnA_state = true;
    });

    uart.on(ATM_EVN_BTN_B_PRE, '\0', [](const char *temp) {
        btnB_state = true;
    });

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("WiFi connecting to %s\n", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(400);
    }
    _clientSecure.setInsecure();
    Serial.printf("\nWiFi connected\nIP : ");
    DEBUG_PRIMTLN(WiFi.localIP());

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
    config.frame_size   = FRAMESIZE_240X240;     // FRAMESIZE_240X240, FRAMESIZE_QVGA(320X240)
    config.jpeg_quality = 6;                     //< Quality of JPEG output. 0-63 lower means higher quality
    config.fb_count     = 2;                     // Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed)
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_brightness(s, -1);
    s->set_contrast(s, 1);
    s->set_saturation(s, 1);

    // init var
    for (int i = 0; i < Prediction_SIZE; i++) {
        Prediction_t *p         = (predictions + i);
        p->valid                = false;
        p->probability          = 0.0;
        p->tagName              = "";
        p->boundingBox_t.left   = 0.0;
        p->boundingBox_t.top    = 0.0;
        p->boundingBox_t.height = 0.0;
        p->boundingBox_t.width  = 0.0;
    }

    // Initialise the TFT
    tft.begin();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setRotation(3);     // 1:landscape 3:inv. landscape
    tft.initDMA();          // To use SPI DMA you must call initDMA() to setup the DMA engine
    // The jpeg image can be scaled down by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);
    // The colour byte order can be swapped by the decoder
    // using TJpgDec.setSwapBytes(true); or by the TFT_eSPI library:
    tft.setSwapBytes(true);
    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
}

void loop()
{
    static uint8_t state = READY;
    uart.loop();

    switch (state) {
    case READY: {
        camera_fb_t *fb = NULL;
        fb              = esp_camera_fb_get();
        if (fb->format != PIXFORMAT_JPEG) {
            DEBUG_PRIMTLN("Non-JPEG data not implemented");
            break;
        }
        tft.startWrite();
        TJpgDec.drawJpg(0, 0, fb->buf, fb->len);
        tft.endWrite();

        if (btnA_state) {
            btnA_state = false;
            if (!fb) {
                DEBUG_PRIMTLN("Camera capture failed");
                break;
            }
            uint32_t pre_ms = millis();
            prediction_idx  = 0;

            // Prediction
            bool ret = AzurePrediction(fb->buf, fb->len);
            // Show ret
            DEBUG_PRIMTLN(ret ? "Success" : "Fail");
            DEBUG_PRIMTLN(prediction_idx);
            for (int i = 0; i < prediction_idx; i++) {
                Prediction_t *p = (predictions + i);
                DEBUG_PRIMT(p->tagName);
                DEBUG_PRIMT(" -> ");
                DEBUG_PRIMT(p->probability);
                DEBUG_PRIMT("[ ");
                DEBUG_PRIMT(p->boundingBox_t.left);
                DEBUG_PRIMT(" , ");
                DEBUG_PRIMT(p->boundingBox_t.top);
                DEBUG_PRIMT(" , ");
                DEBUG_PRIMT(p->boundingBox_t.width);
                DEBUG_PRIMT(" , ");
                DEBUG_PRIMT(p->boundingBox_t.height);
                DEBUG_PRIMT(" ] ");
                DEBUG_PRIMTLN();
                uint32_t color = TFT_CYAN;
                if (p->tagName == "Dinosaur") {
                    color = TFT_RED;
                } else if (p->tagName == "Ghost")
                    color = TFT_BLUE;
                else if (p->tagName == "Shark")
                    color = TFT_GREEN;
                tft.startWrite();
                tft.drawRoundRect(p->boundingBox_t.left * 240, p->boundingBox_t.top * 240, p->boundingBox_t.width * 240, p->boundingBox_t.height * 240, 4, color);
                tft.drawString(p->tagName, (p->boundingBox_t.left * 240) + 3, (p->boundingBox_t.top * 240) + 3, 4);
                tft.endWrite();
            }

            DEBUG_PRIMTLN();
            Serial.print(count++);
            Serial.print(" => Spend time: ");
            Serial.println(millis() - pre_ms);
            DEBUG_PRIMTLN();
            state = IDLE;
        }
        esp_camera_fb_return(fb);
    } break;
    case IDLE: {
        if (btnB_state) {
            btnB_state = false;
            state      = READY;
        }
    } break;
    default:
        break;
    }
}

bool AzurePrediction(uint8_t *image_data, size_t image_sz)
{
    if (image_data == NULL || image_sz == 0)
        return false;
    if (WiFi.status() != WL_CONNECTED)
        return false;
#if DEBUG_TIMER
    uint32_t preMs = millis();
#endif
    // 連線到 Azure Custom Vision
    if (!_clientSecure.connect(API_HOST, 443)) {
        DEBUG_PRIMTLN("connection Azure Custom Vision failed");
        return false;
    }
    DEBUG_PRIMT_TIMER("Connect to Server: ");
    DEBUG_PRIMTLN_TIMER(millis() - preMs);

#if DEBUG_TIMER
    preMs = millis();
#endif
    bool Success_h = false;
    int  httpCode  = 404;

    size_t image_size = image_sz;
    String boundary   = "----Azure_Custom_Vision--";
    String body       = " \r\n";
    body.reserve(200);

    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"image.jpg\"\r\n";
    body += "Content-Type: image/jpeg\r\n\r\n";

    String body_end    = "--" + boundary + "--\r\n";
    size_t body_length = body.length() + image_size + body_end.length();

    // Set HTTPS Header
    String header = "";
    header.reserve(400);
    header += "POST /customvision/v3.0/Prediction/" Prediction_ID "/detect/iterations/" Iterations_ID "/image HTTP/1.1\r\n";
    header += "Host: " API_HOST "\r\n";
    header += "Prediction-Key: " Prediction_Key "\r\n";
    header += "User-Agent: " + String(USER_AGENT) + "\r\n";
    header += "Connection: close\r\n";
    header += "Content-Length: " + String(body_length) + "\r\n";
    header += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n\r\n";

    // Write HTTP Header and body
    String packet = "";
    packet.reserve(600);
    packet = header + body;
    _clientSecure.print(packet);
    DEBUG_PRIMT_TIMER("send header and body packet: ");
    DEBUG_PRIMTLN_TIMER(millis() - preMs);

#if DEBUG_TIMER
    preMs = millis();
#endif
    // Write HTTP bin jpg image
    if (image_size > 0) {
        size_t BUF_SIZE = 1024;
        if (image_data != NULL) {
            uint8_t *p  = image_data;
            size_t   sz = image_size;
            while (p != NULL && sz) {
                if (sz >= BUF_SIZE) {
                    _clientSecure.write(p, BUF_SIZE);
                    p += BUF_SIZE;
                    sz -= BUF_SIZE;
                } else {
                    _clientSecure.write(p, sz);
                    p += sz;
                    sz = 0;
                }
            }
        }
    }
    // Write body end
    _clientSecure.print("\r\n" + body_end);
    DEBUG_PRIMT_TIMER("send image packet: ");
    DEBUG_PRIMTLN_TIMER(millis() - preMs);

#if DEBUG_TIMER
    preMs = millis();
#endif
    // Wait Server packet available
    while (_clientSecure.connected() && !_clientSecure.available())
        delay(10);

    DEBUG_PRIMT_TIMER("wait server response: ");
    DEBUG_PRIMTLN_TIMER(millis() - preMs);
    String resp = "";
    resp.reserve(500);

    // Read response
    if (_clientSecure.connected() && _clientSecure.available()) {
        // Check response code
        if (!findValueFromKey(readStringUntil('\n').c_str(), "HTTP/1.1", " ", " OK", httpCode))
            return false;
        Success_h = (httpCode == 200);
        if (!Success_h) {
            while (_clientSecure.available()) {
                char ch = _clientSecure.read();
                DEBUG_PRIMT(ch);
                DEBUG_PRIMTLN();
            }
            return false;
        }
#if DEBUG_TIMER
        preMs = millis();
#endif
        // _clientSecure.setTimeout(5);
        // skip response header
        while (_clientSecure.available()) {
            resp = readStringUntil('\n');
            if (resp == "\r") {
                break;
            }
        }
        DEBUG_PRIMT_TIMER("Read response header: ");
        DEBUG_PRIMTLN_TIMER(millis() - preMs);

        // while (_clientSecure.available()) {
        //     String ch = readStringUntil(',');
        //     DEBUG_PRIMT(ch);
        //     DEBUG_PRIMTLN();
        // }
#if DEBUG_TIMER
        preMs = millis();
#endif
        // Read response body
        while (_clientSecure.available()) {
            Prediction_t newp;
            if (findValueFromKey(readStringUntil(',').c_str(), "\"probability\"", ":", NULL, newp.probability)) {
                if (newp.probability < 0.75)
                    continue;
                readStringUntil(',');
                if (findValueFromKey(readStringUntil(',').c_str(), "\"tagName\"", ":\"", "\"", newp.tagName))
                    if (findValueFromKey(readStringUntil(',').c_str(), "\"left\"", ":", NULL, newp.boundingBox_t.left))
                        if (findValueFromKey(readStringUntil(',').c_str(), "\"top\"", ":", NULL, newp.boundingBox_t.top))
                            if (findValueFromKey(readStringUntil(',').c_str(), "\"width\"", ":", NULL, newp.boundingBox_t.width))
                                if (findValueFromKey(readStringUntil(',').c_str(), "\"height\"", ":", "}}", newp.boundingBox_t.height)) {
                                    newp.valid = true;
                                    // remove duplicates
                                    bool flag = false;
                                    for (int i = 0; i < prediction_idx; i++) {
                                        if (newp.tagName == predictions[i].tagName) {
                                            if (newp.probability > predictions[i].probability) {
                                                // reWrite date
                                                predictions[i].probability          = newp.probability;
                                                predictions[i].boundingBox_t.left   = newp.boundingBox_t.left;
                                                predictions[i].boundingBox_t.top    = newp.boundingBox_t.top;
                                                predictions[i].boundingBox_t.width  = newp.boundingBox_t.width;
                                                predictions[i].boundingBox_t.height = newp.boundingBox_t.height;
                                            }
                                            flag = true;
                                            break;
                                        }
                                    }
                                    if (flag) {
                                        break;
                                    } else {
                                        predictions[prediction_idx++] = newp;
                                    }
                                    if (prediction_idx >= Prediction_SIZE) {
                                        DEBUG_PRIMTLN("Out of Range");
                                        break;
                                    }
                                }
            }
        }
        DEBUG_PRIMT_TIMER("Read response buff: ");
        DEBUG_PRIMTLN_TIMER(millis() - preMs);
    }
    delay(10);
    _clientSecure.stop();
    return Success_h;
}

inline String readStringUntil(char terminator, uint32_t timeout)
{
    String   ret;
    uint32_t myTimeout = millis() + timeout;
    ret.reserve(100);
    int c = _clientSecure.read();
    while (c >= 0 && c != terminator && millis() < myTimeout) {
        ret += (char)c;
        c = _clientSecure.read();
    }
    return ret;
}

inline bool findValueFromKey(const char *source, const char *key, const char *head, const char *tail, float &value)
{
    String sValue;
    if (findValueFromKey(source, key, head, tail, sValue)) {
        value = sValue.toFloat();
        return true;
    }
    return false;
}

inline bool findValueFromKey(const char *source, const char *key, const char *head, const char *tail, int &value)
{
    String sValue;
    if (findValueFromKey(source, key, head, tail, sValue)) {
        value = sValue.toInt();
        return true;
    }
    return false;
}

inline bool findValueFromKey(const char *source, const char *key, const char *head, const char *tail, String &value)
{
    // DEBUG_PRIMTLN(source);
    // find probability
    char *ptr = strstr(source, key);
    if (ptr == NULL) {
        return false;
    }
    // DEBUG_PRIMTLN(ptr);

    ptr += strlen(key);
    // DEBUG_PRIMTLN(ptr);
    ptr = strstr((const char *)ptr, head);
    if (ptr == NULL) {
        return false;
    }
    ptr += strlen(head);

    char buff[24] = {0};
    if (tail != NULL) {
        char *ptrTail = strstr((const char *)ptr, tail);
        strncpy(buff, ptr, ptrTail - ptr);
    } else {
        strncpy(buff, ptr, (source + strlen(source)) - ptr);
    }
    value = String((const char *)buff);
    return true;
}

// This next function will be called during decoding of the jpeg file to render each
// 16x16 or 8x8 image tile (Minimum Coding Unit) to the TFT.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft.height())
        return 0;
    if (dmaBufferSel)
        dmaBufferPtr = dmaBuffer2;
    else
        dmaBufferPtr = dmaBuffer1;
    dmaBufferSel = !dmaBufferSel;     // Toggle buffer selection
    //  pushImageDMA() will clip the image block at screen boundaries before initiating DMA
    tft.pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr);     // Initiate DMA - blocking only if last DMA is not complete
    // The DMA transfer of image block to the TFT is now in progress...
    // Return 1 to decode next block.
    return 1;
}
