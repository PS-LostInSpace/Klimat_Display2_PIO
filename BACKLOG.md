# Klimat_Display2_PIO – Backlog

Levande backlog för Klimat_Display2_PIO (reTerminal E1001 ESP32-S3 + 7.5" e-ink 800×480).
Fokus: stabil displaypipeline (LVGL → Seeed_GFX ePaper) + OTA, därefter MQTT.

---

## ✅ Klart (översikt)
- [x] Projektstruktur + PlatformIO (ESP32-S3, Arduino-ESP32 2.x)
- [x] WiFi + Async WebOTA (ElegantOTA) fungerar stabilt
- [x] Display “proof-of-life” med LVGL + ePaper flush/update fungerar (Steg 6)
- [x] Dummy dashboard-layout (Page 1) renderas på 800×480 (Steg 7 – pågående stabilisering)

---

## 🟢 Steg 7: Dashboard (dummy) – stabil Page 1

**Mål:**  
Visa en stabil dummy-dashboard (Page 1) på 7.5" e-ink 800×480 via LVGL.
Ingen sidhantering än. Ingen MQTT än. Uppdateringspolicy: t.ex. var 5:e minut.

### 7.1 Bas (stabilitet)
- [ ] Säkerställ att Page1-layouten alltid ritas utan crash/reset
- [ ] Sätt upp en enkel refresh-policy (t.ex. 5 min) och undvik onödiga full refresh

### 7.2 Dashboard-layout (dummy)
- [ ] Definiera layout-sektioner (containers / align)
- [ ] Kolumn vänster: vind “NU” + 1–2h (dummy text)
- [ ] Kolumn mitten: ikon placeholder + 30/60/90 min staplar (dummy)
- [ ] Kolumn höger: UTE/INNE + Atmosfär + “Uppdaterat …” (dummy)
- [ ] Kontrollera att allt får plats i 800 px (inga kapade bokstäver)

### 7.3 Text & tecken
- [ ] Byt till ASCII där det behövs (undvik “–” och åäö tills font-stöd finns)
- [ ] Standardisera enhetstexter: `m/s`, `dBm`, `C`, `%`, `mbar`

### 7.4 Assets (layoutskisser)
- [ ] Lägg layoutskisser i `/assets/` (png/jpg) och committa dem
- [ ] Kort README i `/assets/` (vad bilderna representerar)

### 7.5 Git – spara Steg 7 stabilt
- [ ] `git status`
- [ ] `git add lib/LvglPort/LvglPort.cpp BACKLOG.md assets .gitignore`
- [ ] `git commit -m "Step 7: stable Page 1 dummy dashboard layout (no paging)"`
- [ ] `git tag -a step-7-dashboard-page1-stable -m "Step 7 stable Page 1 dummy dashboard"`
- [ ] `git push && git push --tags`

---

## 🟡 Nästa steg (Steg 8)
**Mål:** MQTT-integration (read-only) + uppdatera UI från MQTT utan att stressa e-ink.

- [ ] Definiera MQTT topics + payload-format (minsta viable)
- [ ] Koppla MQTT → UI-model (struct/state)
- [ ] Triggera ePaper refresh endast vid ändring + enligt policy

---

## 🟠 Features (låg prio)
- [ ] OTA: lösenordskydd / BasicAuth
- [ ] Bättre klocka (NTP) istället för uptime
- [ ] Sidhantering (3 sidor) via knappar + debounce + no wraparound

---

## 📝 Noteringar
- Projektet bygger på Seeed_GFX (TFT_eSPI-variant) med `BOARD_SCREEN_COMBO 520` (E1001 / UC8179).
- E-ink: undvik täta uppdateringar (blink + slitage), mål ~5 min eller vid förändring.
