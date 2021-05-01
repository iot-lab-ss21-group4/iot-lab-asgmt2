#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "common.h"
#include "oled.h"
#include "transitions.h"

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    setup_oled();
    setup_transitions();

    while (1)
    {
        loop_oled();
        loop_transitions();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
