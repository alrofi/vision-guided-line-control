#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// ===========================
// Utilitzem la teva configuració de pins funcional
// ===========================
#include "board_config.h"

// Credencials Wi-Fi
const char *ssid = "Passargada";
const char *password = "sodeusfidequenga";

// IP del teu PC (Substitueix per la IP que t'ha donat el comandament ipconfig)
const char *udpAddress = "192.168.1.152"; 
const int udpPort = 8888;

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA; // Resolució 320x240 per alta velocitat
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  if (psramFound()) {
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // Inicialització de la càmera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA); // Assegura 320x240

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false); // Desactiva l'estalvi d'energia per evitar latències

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Enviant flux UDP a %s:%d\n", udpAddress, udpPort);
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error en capturar el fotograma");
    return;
  }

  // 1. Enviar la mida total de la imatge com a capçalera (4 bytes)
  uint32_t frameSize = fb->len;
  udp.beginPacket(udpAddress, udpPort);
  udp.write((uint8_t*)&frameSize, sizeof(frameSize));
  udp.endPacket();

  // 2. Fragmentar la imatge JPEG en paquets de 1024 bytes i enviar per UDP
  size_t chunkSize = 1024;
  for (size_t i = 0; i < fb->len; i += chunkSize) {
    size_t currentChunk = (i + chunkSize > fb->len) ? (fb->len - i) : chunkSize;
    udp.beginPacket(udpAddress, udpPort);
    udp.write(fb->buf + i, currentChunk);
    udp.endPacket();
  }

  esp_camera_fb_return(fb);

  delay(10); // Pausa mínima per generar ~30-40 FPS fluids
}
