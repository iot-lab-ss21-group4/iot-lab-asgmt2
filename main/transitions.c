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
	//TODO: Use project configuration i.e. gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT)
	gpio_set_direction(INPUT_B_OUTER_GPIO, GPIO_MODE_INPUT);
	gpio_set_direction(INPUT_B_INNER_GPIO, GPIO_MODE_INPUT);

	gpio_pulldown_en(INPUT_B_OUTER_GPIO);
	gpio_pulldown_en(INPUT_B_INNER_GPIO);


	gpio_set_intr_type(INPUT_B_OUTER_GPIO, GPIO_INTR_POSEDGE);
	gpio_set_intr_type(INPUT_B_INNER_GPIO, GPIO_INTR_POSEDGE);
	//Other EDGES
	//GPIO_INTR_ANYEDGE
	//GPIO_INTR_POSEDGE
	//GPIO_INTR_NEGEDGE
	//GPIO_INTR_LOW_LEVEL
	//GPIO_INTR_HIGH_LEVEL
}

void loop_transitions()
{
    ESP_LOGI(TAG, "Button state: %d", gpio_get_level(GPIO_NUM_19));
}
