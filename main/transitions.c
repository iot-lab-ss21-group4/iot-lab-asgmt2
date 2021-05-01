#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "common.h"
#include "transitions.h"

void setup_transitions()
{
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_NUM_19);
}

void loop_transitions()
{
    ESP_LOGI(TAG, "Button state: %d", gpio_get_level(GPIO_NUM_19));
}