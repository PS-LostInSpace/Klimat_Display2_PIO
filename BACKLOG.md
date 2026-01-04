# Klimat_Display2_PIO – Backlog

Detta dokument innehåller idéer, buggar, förbättringar och framtida arbete
för projektet Klimat_Display2_PIO.

> 📌 Används som levande backlog / arbetslista i VS Code & PlatformIO  
> Uppdateras kontinuerligt under utveckling

---

## 🟢 Nästa steg (prioriterat)
- [ ] **Steg 6: Display & grafik (LVGL / E-Ink)**
      - Initiera LVGL (minimal setup)
      - Dummy-layout för 7.5" E-Ink (titel, tid, status)
      - Visa WiFi-status (IP, RSSI)
      - *(Jag hjälper till att skriva Steg 6 i backlog-/checklist-format)*

---

## 🟡 Features / förbättringar
- [ ] Lösenordsskyddad OTA  
      → Server-level Basic Auth i ESPAsyncWebServer
- [ ] Visa hostname + firmware-version på landningssidan
- [ ] `/info` endpoint (JSON) för debug
- [ ] MQTT-status på display (connected / disconnected)
- [ ] Konfigurerbar refresh-rate för E-Ink
- [ ] Soft reset / reboot-knapp via web UI

---

## 🔴 Buggar / tekniska observationer
- [ ] ElegantOTA auth fungerar inte konsekvent i async-mode  
      → Utred: ersätt med explicit server auth
- [ ] SPI-hastighet för Seeed_GFX rapporteras som default (1MHz)
- [ ] Serial output tidigt i boot opålitlig på ESP32-S3

---

## 🔵 Arkitektur / refaktor (låg prio)
- [ ] Porta WebOTA-modulen tillbaka till Klimat_Display_PIO
- [ ] Gemensam Network/OTA-modul för flera projekt
- [ ] Feature-flag cleanup (`HAS_*`)
- [ ] Införa gemensam `config.h` för projektnamn, version, hostname

---

## 📝 Noteringar
- OTA via WiFi fungerar stabilt på reTerminal E1001 (ESP32-S3 + PSRAM)
- Feather ESP32-S3 utan PSRAM hade reset-problem efter OTA men löst med ElegantOTA.setAutoReboot(true);
- Async + ElegantOTA fungerar stabilt i Arduino-ESP32 2.x
