#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "ssd1306.h"

static const char *TAG = "OLED";

void init_display()
{
    ssd1306_128x64_i2c_init();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
}

void text_demo()
{
    ssd1306_clearScreen();
    ssd1306_printFixedN(0, 0, "Normal text", STYLE_NORMAL, 1);
    ssd1306_printFixed(0, 16, "Bold text", STYLE_BOLD);
    ssd1306_printFixed(0, 24, "Italic text", STYLE_ITALIC);
    ssd1306_negativeMode();
    ssd1306_printFixed(0, 32, "Inverted bold", STYLE_BOLD);
    ssd1306_positiveMode();
}

void app_main(void)
{
    esp_log_level_set("BLINK", ESP_LOG_INFO);

    ESP_LOGI(TAG, "Initializing display.");
    init_display();
    text_demo();
    ESP_LOGI(TAG, "Sleping...");
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
