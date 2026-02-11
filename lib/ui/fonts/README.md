# UI fonts

Place generated LVGL font .c files in this folder so PlatformIO compiles them with the UI code.

Suggested workflow for NotoSans-Regular.ttf:
1) Put the TTF in assets/fonts/NotoSans-Regular.ttf
2) Use lv_font_conv to generate sizes used in the UI (12, 16, 24, 28, 40, 48).
3) Name outputs like noto_sans_regular_12.c, noto_sans_regular_16.c, etc.
4) Add LV_FONT_DECLARE(...) declarations where you use the fonts and switch the UI style calls.
