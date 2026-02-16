\# Klimat\_Display2\_PIO



ESP32-S3 based weather display project for Seeed reTerminal E1001  

Monochrome 7.5" 800x480 display using LVGL and MQTT.



\## Hardware

\- Seeed reTerminal E1001 (ESP32-S3, 8MB PSRAM)

\- 7.5" monochrome display (4-level grayscale)

\- 2 buttons (no touch)



\## Software

\- PlatformIO

\- Arduino framework

\- LVGL

\- WiFi + OTA

\- MQTT (via Home Assistant)



\## Project structure

\- `src/` – main application

\- `lib/` – reusable modules (display, network, OTA)

\- `include/` – configuration headers



\## Status

🚧 Work in progress


\## Git
Before startig to code
git checkout develop
git fetch --all --prune
git pull --ff-only

git checkout -b feature/<namn>


When done coding
git checkout develop 
git merge feature/page1-dummy-json 
push

Delete locally
git branch -d feature/<namn>

Delete GitHub
git push origin --delete feature/<namn>

Check Branches
git branch

Check Branches GitBub
git branch -r


