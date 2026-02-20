# Klimat_Display2_PIO – Backlog

Detta dokument innehåller idéer, buggar, förbättringar och framtida arbete
för projektet Klimat_Display2_PIO.

> 📌 Används som levande backlog / arbetslista i VS Code & PlatformIO  
> Uppdateras kontinuerligt under utveckling

---

## 🟢 Nästa steg (prioriterat)
- [ ] **Steg 11: Väderikonsystem (NY PRIORITET)**
      - Definiera ikonlista (minst): `sun`, `clear-night`, `partly`, `cloud`, `rain`, `sleet`, `snow`, `fog`, `wind`, `unknown`
      - Bestäm grafisk stil (outline optimerad för e-ink)
      - Skapa SVG → konvertera till LVGL C-array
      - Införa gemensam ikon-enum
      - Implementera symbol→image switch
      - Säker fallback (`unknown` om saknas)
      - Verifiera minnespåverkan (flash/ram)
      - Testa mörk/ljus kontrast på e-ink

- [ ] **Steg 12: Stabilisering & tuning**
      - Välj riskmodell för rain
      - Ta bort `updated_min` helt
      - Säkerställa att `icon` aldrig är null
      - Eventuell trigger-reducering i HA

- [ ] **Steg 13: Page2 (Indoor climate)**
      - Design layout
      - Ny state-struct del
      - MQTT topic: `home/display/kd2/page2/state`
      - Navigation mellan sidor

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

### Steg 10 (Page1): Home Assistant integration – klart
- Met.no + Nowcast integrerat
- DNS-problem löst
- Normaliserad data-pipeline i HA:
      - `home/weather/observed`
      - `home/weather/forecast_short`
      - `home/weather/nowcast` (A/B/C riskmodeller)
      - `home/indoor/rooms`
- Display-view: `home/display/kd2/page1/state` (`retain=true`)
- Liveness-baserad uppdatering:
      - `<6 min` → "Uppdaterat: nyss"
      - `≥6 min` → "Uppdaterat: --"
- Forecast 1–3h implementerad
- Nowcast A/B/C riskmodeller implementerade
- Rain 30/60/90 stabila
- Status: Produktionstest pågår (utvärdering av riskmodell)

### Steg 9: Data-pipeline (state → MQTT → UI) – klart
- Gemensam `ui_state_t` införd
- Dirty-flags per fält
- `page1_update(state)` uppdaterar endast ändrade objekt
- MQTT → JSON → `ui_state_t` bridge
- E-ink refresh endast vid faktisk state-ändring
- Robust mot saknade fält
- Modulär struktur (återanvändbar för KD1/KD3)
- Bevis: Stabil drift, korrekt uppdatering, inga blink vid identisk payload

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
