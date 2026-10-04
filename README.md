# Vision-Guided Line Control (Cyber-Physical System)

Closed-loop Cyber-Physical System (CPS) for an autonomous line-following mobile robot. It pairs low-level C++ firmware on an ESP32-S3 microcontroller with real-time computer vision in MATLAB/Simulink over a low-latency UDP stream. The vision pipeline processes incoming video through a dual approach: calculating centroid lateral displacement for immediate steering error, and applying a Hough Transform algorithm to estimate line geometry and angle for predictive curve detection, complete with real-time visual tracking overlays.

## 🛠️ System architecture

* **Hardware / Firmware**: ESP32-S3 (N16R8) running C++ firmware, low-latency UDP video streaming using PSRAM (8 MB).
* **Perception (Simulink):** Dual computer vision pipeline (reactive path via centroid and predictive path via Hough Transform).
* **Control:** Lateral PID controller with Anti-Windup and adaptive base speed control (Gain Scheduling).

## 📁 Repository Structure

* `firmware/`: C++ / Arduino code for the ESP32-S3 (WiFi UDP & Camera Server).
* `models/`: Simulation and control models in Simulink (.slx).
* `src/`: MATLAB scripts and functions (.m) for image processing.
* `docs/`: Currently empty but created to add block diagrams and technical documentation for the Bachelor's Thesis (TFG).
* `images/`: Screenshots and result plots.

## 🚀 Requirements and Usage

1. Flash the firmware from `firmware/` onto the ESP32-S3.
2. Connect the ESP32-S3 and the control station to the same UDP network.
3. Run the main Simulink model located in `models/`. --> test_2.slx is a program that simulates the PID response from the servos and motors speed, but the main program is error_recta.slx
