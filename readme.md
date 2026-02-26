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


## OTA script
Build + upload via ElegantOTA Lite:

./scripts/ota-upload.ps1

Skip build and upload latest compiled firmware only:

./scripts/ota-upload.ps1 -SkipBuild


## UI-WX_ICON test

Use this to compare weather icon rendering (`partly`) in 128, 169 and 192.
Test mode now shows all three sizes in one view, side by side, with labels.

Prod/dev icon split:

- 192px icons are deployed to `lib/ui/img` (prod path)
- 128/169px icons are deployed to `lib/ui_dev/img` (dev/test path)

### Build flags (platformio.ini)

In `build_flags` (env `reterminal_e1001`), use:

- `-DUI_ICON_TEST=1`

When done testing, set `-DUI_ICON_TEST=0` to return to normal UI.

Note: `UI_ICON_SIZE` is not used by this compare view.

### Copy/paste workflow (PowerShell)

Generate and deploy all three icon sizes:

```powershell
.\tools\50_png_to_lvgl_c.ps1 -Size 128
.\tools\60_deploy_lvgl_to_ui_img.ps1 -Size 128
.\tools\50_png_to_lvgl_c.ps1 -Size 169
.\tools\60_deploy_lvgl_to_ui_img.ps1 -Size 169
.\tools\50_png_to_lvgl_c.ps1 -Size 192
.\tools\60_deploy_lvgl_to_ui_img.ps1 -Size 192
```

Set in `platformio.ini`:

- `-DUI_ICON_TEST=1`

Then run:

```powershell
.\scripts\ota-upload.ps1
```



