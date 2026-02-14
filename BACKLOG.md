# Klimat_Display2_PIO – Backlog

Detta dokument innehåller idéer, buggar, förbättringar och framtida arbete
för projektet Klimat_Display2_PIO.

> 📌 Används som levande backlog / arbetslista i VS Code & PlatformIO  
> Uppdateras kontinuerligt under utveckling

---

## 🟢 Nästa steg (prioriterat)
- [ ] **Steg 9: Data-pipeline till Page 1 (state → MQTT → UI)**
      - Införa gemensam `ui_state_t` för vind, temp, regn, atmosfär, ikon och updated-tid
      - Implementera `page1_update(state)` som uppdaterar LVGL-objekt via dirty-flags
      - Skapa MQTT→UI-bridge som:
            - tar emot JSON-payload
            - mappar till `ui_state_t`
            - triggar `page1_update()` + e-ink refresh
      - Införa ikon-switch (`ui_set_weather_icon(symbol)`) med fallback-ikon
      - Verifiera uppdateringsloop utan Home Assistant (dummy JSON först)
      - Säkerställa modulär struktur återanvändbar i KD1/KD3

- [ ] **Förberedelse Steg 10: Home Assistant-integration**
      - Definiera MQTT-topicstruktur (`kd2/page1/state`, ev. page2 senare)
      - Skapa HA-automation som publicerar kompakt JSON till displayen
      - Verifiera stabil lokal uppdatering utan extern exponering av HA


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

### Steg 8: UI-polish + fonter + ikon – klart
- Bytt från Montserrat till NotoSans (svenska tecken ÅÄÖ fungerar)
- Genererat LVGL-fontfiler (12/16/24/28/40/48/64/80 )
- Infört UI-fontroller (UI_FONT_TINY/SMALL/BODY/SUBTITLE/H2/H1/DISPLAY1/DISPLAY2)
- Fixat att LVGL verkligen använder projektets lv_conf.h (LV_CONF_H “handshake”)
- Förfinat Page 1 typografi: större siffror, °C som separat label med baseline-känsla
- Förfinat vindkompass: layout/centrering, NOSV, m/s-placering, bättre spacing för vindtext
- Lagt in riktig väderikon (LVGL image .c/.h) och ersatt “[WX ICON]”
- Justerat nederbördsdiagram (30/60/90, procent, spacing, (minuter)-label under ytan)

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
