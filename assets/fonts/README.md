# Font sources

Store source font files here (for example NotoSans-Regular.ttf) before converting to LVGL .c files.



Generate in Powershell;

npx -y lv\_font\_conv@1.5.2 `

&nbsp; --font "assets/fonts/NotoSans-Regular.ttf" `

&nbsp; -r 0x20-0xFF `

&nbsp; --size 80 `

&nbsp; --bpp 2 `

&nbsp; --format lvgl `

&nbsp; -o "lib/ui/fonts/font\_kd2\_epaper\_80.c" `

&nbsp; --lv-include "lvgl.h":

