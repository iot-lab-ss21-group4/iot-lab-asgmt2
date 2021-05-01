#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define INPUT_B_INNER_GPIO 5
#define INPUT_B_OUTER_GPIO 26
#define ESP_INTR_FLAG_DEFAULT 0

extern const char *TAG;

extern char studentOutput[20];
extern volatile uint8_t count;

#endif
