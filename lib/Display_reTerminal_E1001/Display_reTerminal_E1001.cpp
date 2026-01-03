#include "Display_reTerminal_E1001.h"

// Seeed_GFX expects driver.h to be available so it knows what hardware combo to build for.
#include "driver.h"

#include <TFT_eSPI.h>    // Seeed_GFX is a fork; many types live here

// EPaper is provided by Seeed_GFX (see README example usage)
static EPaper epaper;

bool display_begin() {
  epaper.begin();
  return true;
}

void display_show_test_screen() {
  epaper.fillScreen(TFT_WHITE);

  epaper.drawString("Klimat_Display2_PIO", 20, 20, 4);
  epaper.drawString("E1001 + Seeed_GFX OK", 20, 70, 2);

  // Simple “frame” + a few blocks so du ser att 800x480 och orientering stämmer
  epaper.drawRect(10, 10, 780, 460, TFT_BLACK);
  epaper.fillRect(20, 120, 200, 80, TFT_BLACK);
  epaper.fillRect(240, 120, 200, 80, TFT_WHITE);
  epaper.drawRect(240, 120, 200, 80, TFT_BLACK);

  // IMPORTANT: ePaper needs explicit update/refresh
  epaper.update();
}
