# Klimat_Display2_PIO – Backlog

Detta dokument innehåller idéer, buggar, förbättringar och framtida arbete
för projektet Klimat_Display2_PIO.

> 📌 Används som levande backlog / arbetslista i VS Code & PlatformIO  
> Uppdateras kontinuerligt under utveckling

---

## 🟢 Nästa steg (prioriterat)
- [ ] **Steg 8: Sidstruktur (multi-page foundation, utan paging/knappar)**
      - Skapa `PageManager`-struktur (create/show/hide)
      - Bryt ut Page 1 till egen modul: `page1_build(parent)` + `page1_update(state)`
      - Skapa Page 2 stub: `page2_build(parent)` (tom layout + label “Page 2 (stub)”)
      - Skapa gemensam `ui_state_t` (dummy-data nu) för framtida MQTT/API
      - Standardisera UI-konstanter (marginaler, Y_SECTION_DIV, etc.)
      - Inför “dirty flags” per sektion (för framtida e-ink update-policy)

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

---

## ✅ Avklarat

### Steg 7: Page 1 layout (3 kolumner) – klart
- 3-kolumns dashboard (Vind / Nederbörd / Ute-Inne-Atmosfär)
- Gemensam vertikal rytm över kolumner (delare + rubrik + innehåll)
- Vindkolumn: bort med redundant rubrik, större kompassyta, bättre visuell balans mot WX-ikon
- Nederbörd: dynamiska staplar (30/60/90) med procenttext, headroom vid 100%, baseline och justerade label offsets
- Kodstruktur förbättrad med tydliga blockkommentarer och layout-konstanter
- Stabil LVGL→ePaper flush/update-loop (full_refresh=1 i nuläget)

---

### Steg 6: Stabil baseline (ePaper + LVGL) – klart
- Verifierad fungerande ePaper-driver med LVGL flush/update (reTerminal E1001 + UC8179)
- Stabil PlatformIO setup och fungerande OTA/WiFi/Serial
- Återställd fungerande Seeed_GFX-konfig (efter tidigare HSPI/board-strul)
- Taggad baseline: `step-6-epaper-ok`
