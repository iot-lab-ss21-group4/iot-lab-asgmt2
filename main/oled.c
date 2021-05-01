#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "common.h"
#include "ssd1306.h"
#include "oled.h"

void setup_oled()
{
    ssd1306_128x64_i2c_init();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
}

void showRoomState()
{
	ssd1306_clearScreen();
	snprintf(studentOutput, 20, "Students: %d", count);
	ssd1306_printFixedN(0, 0, studentOutput, STYLE_NORMAL,1);
    // TODO: can be removed later, just to remember here possible calls
//    ssd1306_printFixedN(0, 0, "Normal text", STYLE_NORMAL, 1);
//    ssd1306_printFixed(0, 16, "Bold text", STYLE_BOLD);
//    ssd1306_printFixed(0, 24, "Italic text", STYLE_ITALIC);
//    ssd1306_negativeMode();
//    ssd1306_printFixed(0, 32, "Inverted bold", STYLE_BOLD);
//    ssd1306_positiveMode();
}
