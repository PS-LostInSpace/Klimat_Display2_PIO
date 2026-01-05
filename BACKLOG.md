# Klimat_Display2_PIO – Backlog

Detta dokument innehåller idéer, buggar, förbättringar och framtida arbete
för projektet Klimat_Display2_PIO.

> 📌 Levande backlog / arbetslista i VS Code & PlatformIO  
> Uppdateras kontinuerligt under utveckling

---

## 🟢 Steg 7: Dashboard-layout & refresh-policy (LVGL / E-Ink)

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
