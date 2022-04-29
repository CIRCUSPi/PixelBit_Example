//#include "soc/soc.h"
//#include "soc/rtc_cntl_reg.h"
#include "SPI.h"
#include "esp_camera.h"
#include <TFT_eSPI.h>     // Hardware-specific library
#include <TJpg_Decoder.h>
#include <tca5405.h>

// Select camera model
//#define CAMERA_MODEL_M5STACK_ESP32CAM
#define CAMERA_MODEL_PIXEL_BIT
#include "camera_pins.h"

uint16_t  dmaBuffer1[16 * 16];     // Toggle buffer for 16*16 MCU block, 512bytes
uint16_t  dmaBuffer2[16 * 16];     // Toggle buffer for 16*16 MCU block, 512bytes
uint16_t *dmaBufferPtr = dmaBuffer1;
bool      dmaBufferSel = 0;

TFT_eSPI tft = TFT_eSPI();     // Invoke custom library

TCA5405 tca5405;

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
    camera_fb_t *fb = NULL;
    fb              = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        esp_camera_fb_return(fb);
        return;
    }
    if (fb->format != PIXFORMAT_JPEG) {
        Serial.println("Non-JPEG data not implemented");
        return;
    }
    // Must use startWrite first so TFT chip select stays low during DMA and SPI channel settings remain configured
    tft.startWrite();
    // Draw the image, top left at 0,0 - DMA request is handled in the call-back tft_output() in this sketch
    TJpgDec.drawJpg(0, 0, fb->buf, fb->len);
    //  TJpgDec.drawJpg(-40, 0,  fb->buf, fb->len); //for FRAMESIZE_QVGA getting centralize

    // Must use endWrite to release the TFT chip select and release the SPI channel
    tft.endWrite();
    esp_camera_fb_return(fb);
}
