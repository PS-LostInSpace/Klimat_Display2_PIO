# Klimat_Display2_PIO – Backlog

Detta dokument innehåller idéer, buggar, förbättringar och framtida arbete
för projektet Klimat_Display2_PIO.

> 📌 Levande backlog / arbetslista i VS Code & PlatformIO  
> Uppdateras kontinuerligt under utveckling

---

## 🏷️ Git-tag strategi (milstolpar)

Projektet använder **enkla, beskrivande tags** för större steg.
Varje tag motsvarar ett **stabilt läge som går att återgå till**.

### Principer
- En tag = en tydlig milstolpe
- Tags flyttas **inte i efterhand**
- Skapas **efter push** när steget är verifierat

### Namnstandard
- `step-6-proof-of-life`
- `step-7-dashboard-dummy`
- `step-8-mqtt-integration`

### Kommando (exempel)
```bash
git tag step-7-dashboard-dummy
git push --tags

## 🟢 Steg 7: Dashboard-layout & refresh-policy (LVGL / E-Ink)

---

**Mål:**  
Skapa en **strukturerad dummy-dashboard** som efterliknar en väderdisplay
och samtidigt minska blink/flicker genom kontrollerad ePaper-refresh.

Ingen riktig väderdata ännu – endast layout, typografi och uppdateringspolicy.


### 7.1 Refresh-policy (anti-blink)
- [ ] Begränsa `epaper.update()` till max 1 gång per minut
- [ ] Separera:
      - [ ] UI-uppdatering (LVGL labels)
      - [ ] ePaper refresh (batchad)
- [ ] Dokumentera observerad refresh-tid (ms)

### 7.2 Dashboard-layout (dummy)
- [ ] Definiera layout-sektioner (containers / align)
      - [ ] Topp: plats / datum
      - [ ] Center: stor temperatur (dummyvärde)
      - [ ] Sekundär info: vind / luftfuktighet (dummy)
      - [ ] Nederkant: WiFi / systemstatus
- [ ] Använd konsekvent typografi (stor/liten text)
- [ ] Anpassa layout för 800 × 480 (landscape)

### 7.2.1 Container-layout (baserad på referensbild)

**Mål:** Skapa ett layout-skelett som efterliknar referensbilden (OUT / SENSOR / SKY / INFO + FORECAST-rad)
för att kunna testa täthet och typografi på 7.5" E-Ink (800×480).

#### Layout-grid (övergripande)
- Skärmen delas i 2 huvudrader:
  - **Row A (Top)**: “cards” (OUT, SENSOR, SKY, INFO)
  - **Row B (Bottom)**: FORECAST-rad med 6–7 “forecast cards”

##### Rekommenderade höjder
- Row A: ~240 px (övre halvan)
- Row B: ~240 px (nedre halvan)

---

#### Row A – Top cards (4 kolumner)
Row A delas i 4 kolumner (cards):
1) **OUT card** (vänster, ca 35% bredd)
2) **SENSOR card** (nästa, ca 25% bredd)
3) **SKY card** (nästa, ca 25% bredd)
4) **INFO card** (höger, ca 15% bredd)

**Card-styling (dummy)**
- Tunn ram (1 px)
- Liten “header badge” överst: t.ex. `OUT Helsinki`
- Inre padding: 10–14 px

---

#### OUT card (detalj)
Container: `card_out`
- Header: `out_header` (badge)
- Main value: `out_temp_big` (stor temp)
- Meta rows: `out_meta_col` (vertikal lista)
  - `out_feelslike`
  - `out_humidity`
  - `out_pressure`
  - `out_wind`
- Icon area: `out_icon_box` (stor väderikon / placeholder)

**Placering**
- Vänster kolumn: textblock (temp + metainfo)
- Höger kolumn: ikon/placeholder-ruta

---

#### SENSOR card (detalj)
Container: `card_sensor`
- Header: `sensor_header` (badge)
- Sensor 1 block: `sensor1_block`
  - `sensor1_temp_big`
  - `sensor1_meta_row` (wifi/ikon + RH)
- Sensor 2 block: `sensor2_block`
  - `sensor2_temp_big`
  - `sensor2_meta_row` (wifi/ikon + RH)
- Option: liten batteriikon/dummy längst upp i varje block

**Placering**
- 2 rader (sensor1 över, sensor2 under), lika höjd

---

#### SKY card (detalj)
Container: `card_sky`
- Header: `sky_header` (badge)
- Timeline area: `sky_timeline`
  - vänster: tider (t.ex. 9:09, 9:45, …)
  - mitten: vertikal bar/linje (dummy)
  - höger: markör (triangel / dummy)
- Moon/sun area: `sky_moon_box`
  - stor cirkel (dummy)
  - text under: `sky_daylength` (t.ex. “9h 39m”)

**Placering**
- Vänster: timeline (ca 60–65%)
- Höger: moon/sun (ca 35–40%)

---

#### INFO card (detalj)
Container: `card_info`
- Header: `info_header` (badge)
- Big time: `info_time_big` (t.ex. 10:22)
- Date lines: `info_date_lines`
  - `info_weekday_date`
  - `info_weekno`
  - `info_year`
- Footer: `info_mode` (t.ex. “DEV MODE”)

**Placering**
- Vertikal stack, centrerad

---

#### Row B – FORECAST (6–7 cards i rad)
Container: `row_forecast`
- Header badge: `forecast_header` (t.ex. `FORECAST Helsinki`)
- Cards container: `forecast_cards_row` (horisontell layout)
  - `fc_1` … `fc_6` (eller 7 beroende på bredd)

Varje forecast card (`fc_n`) innehåller:
- `fc_time` (t.ex. 15:00)
- `fc_icon_box` (ikon/placeholder)
- `fc_temp` (t.ex. 26°C)
- `fc_wind` (t.ex. 19 m/s)
- `fc_moonphase` (liten cirkel/dummy längst ner)
- Option: liten varningstriangel i hörn (dummy)

**Card-styling**
- Liten ram + inner padding
- Ikonruta kan vara grå “panel” (simuleras med ram + fylld bakgrund senare)

---

#### Dummy-data (för layouttest)
- OUT: -26.6°C, RH 11%, 1099 hPa, Wind 15–24 m/s
- SENSOR: 19.4°C / 18.2°C, RH 20% / 28%
- SKY: tider + daylength “9h 39m”
- INFO: 10:22, Sun 6.8, Week 31, 2023, DEV MODE
- FORECAST: 6 rutor med tid + temp + vind

---

#### Definition of Done för 7.2.1
- Layout-skelett finns (rutor/containers + rubriker)
- All text ryms utan att klippa
- Visuellt test på ePaper visar om layouten är “för tät” eller ok


### 7.3 Förberedelse för framtida data
- [ ] Abstrakta “data labels” (temp, vind, status)
- [ ] Enkla placeholder-värden
- [ ] Kommentarer i kod: var MQTT / sensordata ska in

### 7.4 Visuell verifiering
- [ ] Dashboard känns balanserad på ePaper
- [ ] Ingen onödig blink vid uppdatering
- [ ] Uppdateringar upplevs stabila (acceptabel latens)

### 7.5 Git – spara Steg 7
- [ ] `git status`
- [ ] `git add .`
- [ ] `git commit -m "Step 7: Weather-style dashboard layout + ePaper refresh policy"`
- [ ] `git push`
- [ ] `Tag: step-7-dashboard-dummy`

---

## 🟢 Steg 6: Display & grafik (LVGL / E-Ink) — KLAR

**Mål (uppnått):**  
Visa en stabil “proof-of-life”-vy på 7.5" E-Ink (reTerminal E1001) med LVGL:
titel, tid och WiFi-status. Full refresh OK, ingen optimering ännu.

### 6.1 Grundsetup
- [x] Aktivera `HAS_LVGL` i `config.h`
- [x] Säkerställ att `lvgl_port_begin()` körs i `setup()`
- [x] Säkerställ att `lvgl_port_loop()` körs i `loop()`
- [x] Initiera Seeed_GFX ePaper (UC8179)
- [x] Rensa skärmen (`fillScreen(WHITE)`)

### 6.2 LVGL-init
- [x] `lv_init()`
- [x] LVGL tick via `esp_timer` → `lv_tick_inc(1)` (1 ms)
- [x] Initiera draw buffer (line-buffer, 800 × 40)
- [x] Registrera LVGL display driver
  - [x] `hor_res = 800`
  - [x] `ver_res = 480`
  - [x] `flush_cb` → ritar via `epaper.drawPixel()`
  - [x] `full_refresh = 1` (ePaper proof-of-life)

### 6.3 Dummy-layout (grafik)
- [x] Titel högst upp  
      Text: **"Klimat_Display2 (E1001)"**
- [x] Tid i mitten  
      Format: `HH:MM:SS` (uptime via `millis()`)
- [x] WiFi-status under tiden  
      - [x] IP-adress
      - [x] RSSI i dBm
      - [x] Fallback-text vid offline

### 6.4 UI-uppdatering
- [x] LVGL timer uppdaterar UI periodiskt (nu 60 s)
- [x] Flagga sätts när UI ändras (`g_need_epaper_update = true`)
- [x] Faktisk ePaper refresh sker i `lvgl_port_loop()`
      - [x] `epaper.update()` endast när flaggan är satt

### 6.5 Verifiering
- [x] Skärmen uppdateras korrekt vid boot
- [x] Tid räknar upp korrekt (med förväntad ePaper-blockering)
- [x] WiFi-status visas korrekt (IP + RSSI)
- [x] Ingen crash / watchdog-reset
- [x] ElegantOTA fungerar parallellt

### 6.6 Git – spara Steg 6
- [x] `git status`
- [x] `git add .`
- [x] `git commit -m "Step 6: LVGL proof-of-life layout on 7.5in ePaper"`
- [x] `git push`
- [x] Tag: step-6-proof-of-life


---



---

## 🟡 Features / förbättringar (ej tidskritiska)
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
- [ ] ePaper full refresh blockerar ~2 s (förväntat beteende)
- [ ] SPI-hastighet för Seeed_GFX oklar / ej verifierad

---

## 🔵 Arkitektur / refaktor (låg prio)
- [ ] Gemensam Network/OTA-modul för flera projekt
- [ ] Feature-flag cleanup (`HAS_*`)
- [ ] Införa gemensam `config.h` för projektnamn, version, hostname

---

## 📝 Noteringar
- LVGL + Seeed_GFX fungerar stabilt på reTerminal E1001
- ePaper (7.5") kräver lång refresh → policy nödvändig
- ElegantOTA fungerar stabilt parallellt med LVGL
- Blink/invertering vid full refresh är normalt för e-ink
