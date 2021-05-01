#include "common.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <stdint.h>

const char *TAG = "OLED";
char studentOutput [20];

volatile uint8_t count;
