# Vision-Guided Line Control (Cyber-Physical System)

Sistema ciberfísico (CPS) en bucle cerrado para un robot móvil autónomo. Combina procesamiento de visión por computador en tiempo real (Transformada de Hough) sobre MATLAB/Simulink y firmware de bajo nivel en C++ alojado en un microcontrolador ESP32-S3.

## 🛠️ Arquitectura del Sistema

* **Hardware / Firmware:** ESP32-S3 (N16R8) con firmware en C++, streaming de vídeo UDP de baja latencia utilizando PSRAM (8 MB).
* **Percepción (Simulink):** Pipeline dual de visión artificial (vía reactiva por centroide y vía predictiva mediante Transformada de Hough).
* **Control:** Controlador PID lateral con Anti-Windup y control adaptativo de velocidad base (*Gain Scheduling*).

## 📁 Estructura del Repositorio

* `firmware/`: Código C++ / Arduino para la ESP32-S3 (WiFi UDP & Camera Server).
* `models/`: Modelos de simulación y control en Simulink (`.slx`).
* `src/`: Scripts y funciones en MATLAB (`.m`) para el procesado de imagen.
* `docs/`: Diagramas de bloques y documentación técnica del TFG.
* `images/`: Capturas y gráficos de resultados.

## 🚀 Requisitos y Uso

1. Cargar el firmware de `firmware/` en la ESP32-S3.
2. Conectar la ESP32-S3 y la estación de control a la misma red UDP.
3. Ejecutar el modelo principal en Simulink alojado en `models/`.