#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

// ===========================
// Utilitzem la teva configuració de pins funcional
// ===========================
#include "board_config.h"

Servo MyServo;
// Credencials Wi-Fi
const char *ssid = "Passargada";
const char *password = "sodeusfidequenga";

// IP del teu PC (Substitueix per la IP que t'ha donat el comandament ipconfig)
const char *udpAddress = "192.168.1.192"; 
const int udpPort = 8888;
const int pinGPIO14 = 14; // Canviar el número de pin
const int pinServo = 1; // GPIO14 lliure de la teva ESP32-S3
const int pinVelocitat = 2;

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  //setup pin servo i LED 
  pinMode(pinGPIO14, OUTPUT);
  digitalWrite(pinGPIO14, LOW); // Estat inicial apagat
  MyServo.attach(pinServo, 900, 2100);

  //setup pin velocitat (Motors DC)
  pinMode(pinVelocitat, OUTPUT);
  analogWrite(pinVelocitat, 0);

  //Camera configuration
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
  //Si logramos conectarnos mostramos la ip a la que nos conectamos
  Serial.println("\nWiFi connected!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Enviant flux UDP a %s:%d\n", udpAddress, udpPort);

  //Serial.print("Nivell de senyal (RSSI): ");
  //Serial.print(WiFi.RSSI());
  //Serial.println(" dBm");
  udp.begin(udpPort);
}


void loop() {

  int packetSize = udp.parsePacket();
    if (packetSize >= 3) {
      uint8_t dades[3];

      // Llegeix l'únic byte enviat per Simulink
      udp.read(dades, 3);
    
    // Separem les dades
    uint8_t dadaVel = dades[0]; // Primer byte
    uint8_t dadaLED   = dades[1]; // El segon byte (Ex: 0 o 1)
    uint8_t dadaServo = dades[2]; // El tercer byte (Ex: 0 a 180)

    // Acció 1: Controlar el LED amb la primera dada
    digitalWrite(pinGPIO14, dadaLED);

    // Acció 2: Controlar el Servo amb la segona dada (si fas servir servo)
    MyServo.write(dadaServo);
    
    //Acció 3: Controlar velocitat motors dc
    analogWrite(pinVelocitat, dadaVel);

  }

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error en capturar el fotograma");
    return;
  }

  // 1. Enviar la mida total de la imatge com a capçalera (4 bytes)
  uint32_t frameSize = fb->len; //Guardar MIDA total en bytes de la imatge capturada dins variable de 32 bits
  udp.beginPacket(udpAddress, udpPort); //Obre un nou paquet en blanc amb direcció udpAdress i port udpPort
  udp.write((uint8_t*)&frameSize, sizeof(frameSize)); // &framesize. '&' obté adreça memòria framesize (on està guardada MIDA foto)
    // uint8_t = cast. Tracta números (en aquest cas de frameSize) com a seqüència de bytes individuals
    // sizeof(frameSize) retorna mida variable (frameSize) en bytes (framseSize és un uint32_t, per tant són sempre 32 bites)
    // udp.write((byte1 byte2 byte3 byte4), 4)
    //Agafa els 4 bytes crus de la RAM de la esp32 i els posa dins paquet UDP.
  udp.endPacket(); // Acaba el paquet (està "llest") --> s'envia info: frameSize des de la esp32 a IP pc

  // 2. Fragmentar la imatge JPEG en paquets de 1024 bytes i enviar per UDP
  size_t chunkSize = 1436;  // -----> potser canviar en funció de la meva wifi x millorar rendiment?
  for (size_t i = 0; i < fb->len; i += chunkSize) {
    size_t currentChunk = (i + chunkSize > fb->len) ? (fb->len - i) : chunkSize; // línia que només serveix per si l'últim paquet és menor de 1024 bytes
    udp.beginPacket(udpAddress, udpPort);
    udp.write(fb->buf + i, currentChunk);
    udp.endPacket(); // va enviant els bytes de chunkSize en chunkSize fins arribar al final de frameSize (fins que s'hagi enviat tota la imatge)
  }

  esp_camera_fb_return(fb);
  yield();
  delay(10); // Pausa mínima per generar ~30-40 FPS fluids --> 10 ~ 30 sol anar bé
}