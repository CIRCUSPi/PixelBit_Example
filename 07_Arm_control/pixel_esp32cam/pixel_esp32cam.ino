#include <Arduino.h>
#include <trash_inferencing.h>

#include <tca5405.h>
#include "esp_http_server.h"
#include "img_converters.h"
#include "image_util.h"
#include "esp_camera.h"
//#include "SPI.h"
#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();

//#include <TJpg_Decoder.h>

#define CAMERA_MODEL_PIXEL_BIT

#include "camera_pins.h"

#define PART_BOUNDARY "123456789000000000000987654321"

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

dl_matrix3du_t *resized_matrix = NULL;
size_t out_len = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
ei_impulse_result_t result = {0};

TCA5405 tca5405;

String inputString = "";


void setup() {

  tca5405.init(21);
  tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 0);
  tca5405.transmit();
  delay(100);
  tca5405.set_gpo(PIXELBIT_CAMERA_POWER, 1);
  tca5405.transmit();
  delay(100);


  Serial.begin(9600);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_240X240;
  config.jpeg_quality = 6;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    //Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, -1);
  s->set_contrast(s, 1);
  s->set_saturation(s, 1);

  // Initialise the TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

}

void loop() {
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      //inference();
      
      if (inputString == "I\r") {
        tft.fillScreen(TFT_BLACK);
        delay(100);
        for (byte k=3; k>0; k--) {
          tft.setCursor(100, 60, 2);
          tft.setTextColor(TFT_ORANGE, TFT_BLACK);  tft.setTextSize(7);
          tft.println(k);
          delay(1000);  
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
        
        inference();
      }
      /*
      else {
        error(inputString);
      }
      */
      inputString = "";
    }
    else {
      inputString += inChar;  
    }
  } 
}

int raw_feature_get_data(size_t offset, size_t out_len, float *signal_ptr)
{
  size_t pixel_ix = offset * 3;
  size_t bytes_left = out_len;
  size_t out_ptr_ix = 0;

  // read byte for byte
  while (bytes_left != 0) {
    // grab the values and convert to r/g/b
    uint8_t r, g, b;
    r = resized_matrix->item[pixel_ix];
    g = resized_matrix->item[pixel_ix + 1];
    b = resized_matrix->item[pixel_ix + 2];

    // then convert to out_ptr format
    float pixel_f = (r << 16) + (g << 8) + b;
    signal_ptr[out_ptr_ix] = pixel_f;

    // and go to the next pixel
    out_ptr_ix++;
    pixel_ix += 3;
    bytes_left--;
  }
  return 0;
}

void classify()
{
  //Serial.println("Getting signal...");
  signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_WIDTH;
  signal.get_data = &raw_feature_get_data;

  //Serial.println("Run classifier...");
  // Feed signal to the classifier
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false /* debug */);
  
  // Returned error variable "res" while data object.array in "result"
  //ei_printf("run_classifier returned: %d\n", res);
  if (res != 0)
    return;
  /*
  // print the predictions
  ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("    %s: \t%f\r\n", result.classification[ix].label, result.classification[ix].value);
  }
  */

  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);  tft.setTextSize(2);
  tft.print(" ");
  tft.print(result.classification[0].label);
  tft.print("     : ");
  tft.println(result.classification[0].value);
  tft.print(" ");
  tft.print(result.classification[1].label);
  tft.print(" : ");
  tft.println(result.classification[1].value);

  float check = 0.0;
  byte higher = 0;

  for (byte i=0; i<EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > check) {
      higher = i;
      check = result.classification[i].value;
    }
  }
  
  delay(1000);
  for (byte k=3; k>0; k--) {
    tft.setCursor(100, 120, 2);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);  tft.setTextSize(7);
    tft.println(k);
    delay(1000);  
  }
  tft.setCursor(100, 120, 2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);  tft.setTextSize(7);
  tft.println(" ");
  delay(100);
  
  if (higher == 0) {
    Serial.println("A");
  }
  else {
    Serial.println("B");
  }
  
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  //ei_printf("    anomaly score: %f\r\n", result.anomaly);
#endif
}

void inference() {
  camera_fb_t * fb = NULL;

  //Serial.println("Capture image");
  fb = esp_camera_fb_get();
  if (!fb) {
    //Serial.println("Camera capture failed");
    //httpd_resp_send_500(req);
    return;
  }

  // --- Convert frame to RGB888  ---
  //Serial.println("Converting to RGB888...");
  // Allocate rgb888_matrix buffer
  dl_matrix3du_t *rgb888_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  fmt2rgb888(fb->buf, fb->len, fb->format, rgb888_matrix->item);

  // --- Resize the RGB888 frame to 96x96 in this example ---
  //Serial.println("Resizing the frame buffer...");
  resized_matrix = dl_matrix3du_alloc(1, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3);
  image_resize_linear(resized_matrix->item, rgb888_matrix->item, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3, fb->width, fb->height);

  // --- Free memory ---
  dl_matrix3du_free(rgb888_matrix);
  esp_camera_fb_return(fb);

  classify();

  // --- Convert back the resized RGB888 frame to JPG to send it back to the web app ---
  //Serial.println("Converting resized RGB888 frame to JPG...");
  uint8_t * _jpg_buf = NULL;
  fmt2jpg(resized_matrix->item, out_len, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, PIXFORMAT_RGB888, 10, &_jpg_buf, &out_len);

  // --- Free memory ---
  dl_matrix3du_free(resized_matrix);
  free(_jpg_buf);
  _jpg_buf = NULL;
}


void error(String error_) {
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);  tft.setTextSize(2);
  tft.println(error_);
}
