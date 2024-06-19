# Driver Drowsiness Detector

## What does it do?
Monitors a driver's eye movements in real time using ESP32-CAM.
If eyes remain closed for too long, it triggers a buzzer alarm and sends an instant Telegram alert to prevent road accidents.

## Why I built this
Driver drowsiness is one of the leading causes of road accidents in India.
I built this as a low cost embedded solution that can be fitted in any vehicle to detect drowsiness early and alert the driver before an accident happens.

## Hardware used
- ESP32-CAM
- Buzzer
- Power supply module

## How it works
- Camera captures frames continuously
- Each frame is analyzed for brightness level
- Low brightness = eyes closed
- If eyes are closed for 10 consecutive frames = drowsiness detected
- Buzzer fires immediately
- Telegram alert sent to registered number

## Features
- Real time eye closure detection
- Instant buzzer alert on drowsiness
- Telegram notification with timestamp
- 30 second cooldown between alerts to avoid spam

## How to run
1. Open main.cpp in Arduino IDE
2. Install ESP32 board package
3. Select board: AI Thinker ESP32-CAM
4. Enter your WiFi name and password
5. Enter your Telegram username and CallMeBot API key
6. Upload to ESP32-CAM
7. Open Serial Monitor at 115200 baud

## Built by
Gauresh Pathak — Electronics and Telecommunication Engineering Student, Mumbai
