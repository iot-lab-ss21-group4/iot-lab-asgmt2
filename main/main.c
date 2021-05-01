#include "common.h"
#include "oled.h"
#include "transitions.h"

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Start");
    init_common_queues();

    setup_oled();
    setup_transitions();

    while (1)
    {
        loop_oled();
        loop_transitions();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
