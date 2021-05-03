#include "common.h"

const char *TAG = "OLED";
volatile uint8_t count = 0;
const uint16_t COUNT_DISPLAY_Q_SIZE = 32;
xQueueHandle count_display_q = NULL;

void init_logging()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Start");
}

void init_common_queues()
{
    if (count_display_q == NULL)
    {
        count_display_q = xQueueCreate(COUNT_DISPLAY_Q_SIZE, sizeof(count_display_q_item));
    }
}