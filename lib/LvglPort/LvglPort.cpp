#include "LvglPort.h"

#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include "Seeed_GFX.h"
#include <lvgl.h>

// ---- Display (Seeed_GFX EPaper) ----
static EPaper epaper;

// ---- LVGL draw buffers ----
// Vi kör en liten line-buffer (t.ex. 40 rader). LVGL flushar i chunks.
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;

static volatile bool g_need_epaper_update = false;

static inline uint16_t map_lv_color_to_epaper(lv_color_t c) {
    // LV_COLOR_DEPTH=8 -> c.full (0..255-ish luma i praktiken)
    // Vi mappar till 4 nivåer om de finns, annars fallback svart/vit.
    uint8_t v = c.full;

    // Försök använda grå om definierade, annars svart/vit:
    #ifdef TFT_DARKGREY
    if (v < 64)  return TFT_BLACK;
    if (v < 128) return TFT_DARKGREY;
    if (v < 192) return TFT_LIGHTGREY;
    return TFT_WHITE;
    #else
    return (v < 128) ? TFT_BLACK : TFT_WHITE;
    #endif
}

static void my_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    // Rita in LVGLs area i epaper-spritebuffer
    // OBS: drawPixel per pixel är långsamt, men OK för första “proof-of-life”.
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            uint16_t col = map_lv_color_to_epaper(*color_p);
            epaper.drawPixel(x, y, col);
            color_p++;
        }
    }

    g_need_epaper_update = true;
    lv_disp_flush_ready(disp);
}

void lvgl_port_begin() {
    // Init EPaper
    epaper.begin();
    epaper.fillScreen(TFT_WHITE);

    // Init LVGL
    lv_init();

    // Allocate LVGL buffer (W * lines)
    const uint32_t buf_lines = 40;
    buf1 = (lv_color_t*)heap_caps_malloc(800 * buf_lines * sizeof(lv_color_t), MALLOC_CAP_INTERNAL);
    if (!buf1) {
        // Fallback: normal malloc (kan vara långsammare)
        buf1 = (lv_color_t*)malloc(800 * buf_lines * sizeof(lv_color_t));
    }

    lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, 800 * buf_lines);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = my_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1; // ePaper gillar full refresh i början
    lv_disp_drv_register(&disp_drv);

    // ---- Minimal UI ----
    lv_obj_t *scr = lv_scr_act();

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Klimat_Display2");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *status = lv_label_create(scr);
    lv_label_set_text(status, "LVGL: OK  |  ePaper: OK");
    lv_obj_align(status, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Steg 6: dummy layout");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -16);

    // Första refresh
    g_need_epaper_update = true;
}

void lvgl_port_loop() {
    // LVGL tick kommer från lv_conf via millis()
    lv_timer_handler();

    // När något ritats av LVGL -> gör ePaper update här (samlat)
    if (g_need_epaper_update) {
        epaper.update();               // Explicit refresh krävs för ePaper :contentReference[oaicite:3]{index=3}
        g_need_epaper_update = false;
    }
}

bool lvgl_port_needs_epaper_update() {
    return g_need_epaper_update;
}

void lvgl_port_clear_epaper_update_flag() {
    g_need_epaper_update = false;
}
