#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define BUZZER_PIN 4

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* telegramUsername = "@YOUR_TELEGRAM_USERNAME";
const char* apiKey = "YOUR_CALLMEBOT_API_KEY";

int eyeClosedFrames = 0;
int eyeClosedThreshold = 10;
bool drowsy = false;
unsigned long lastAlertTime = 0;
int alertCooldown = 30000;

void reconnectWifi() {
  // retry wifi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Reconnecting...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected!");
    } else {
      Serial.println("Reconnection failed.");
    }
  }
}

float getFrameBrightness(camera_fb_t* fb) {
  float total = 0;
  for (int i = 0; i < fb->len; i++) {
    total += fb->buf[i];
  }
  return total / fb->len;
}

void buzzAlert() {
  // buzzing 3 times to wake driver up
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

String getTime() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
  return String(buf);
}

void sendAlert(String message) {
  // sending telegram alert
  HTTPClient http;
  String url = "https://api.callmebot.com/text.php?user=" + String(telegramUsername) + "&apikey=" + String(apiKey) + "&text=" + message;
  http.begin(url);
  http.GET();
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  configTime(19800, 0, "pool.ntp.org");
  delay(2000);

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
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size   = FRAMESIZE_QVGA;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    return;
  }

  Serial.println("Camera ready. Monitoring driver...");
}

void loop() {
  reconnectWifi();

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Frame capture failed");
    return;
  }

  float brightness = getFrameBrightness(fb);
  esp_camera_fb_return(fb);

  if (brightness < 100) {
    eyeClosedFrames++;
    Serial.println("Eyes closed! Count: " + String(eyeClosedFrames));
  } else {
    eyeClosedFrames = 0;
    drowsy = false;
    Serial.println("Eyes open. Brightness: " + String(brightness));
  }

  if (eyeClosedFrames >= eyeClosedThreshold) {
    drowsy = true;
    Serial.println("DROWSINESS DETECTED!");
    buzzAlert();

    if (millis() - lastAlertTime > alertCooldown) {
      sendAlert("DROWSINESS ALERT! Driver appears to be sleeping at " + getTime());
      lastAlertTime = millis();
    }
  }

  delay(100);
}
