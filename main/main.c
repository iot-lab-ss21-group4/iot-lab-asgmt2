#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "common.h"
#include "oled.h"
#include "transitions.h"


static xQueueHandle lightBarrierEventQueue;


static void IRAM_ATTR gpio_outer_barrier_handler(void* arg){
	uint32_t gpio_num = (uint32_t) arg;
	ets_printf("GPIO %d triggered event.\n", gpio_num);
	gpio_isr_handler_remove(gpio_num);
	xQueueSendFromISR(lightBarrierEventQueue, &gpio_num, NULL);
}

static void IRAM_ATTR gpio_inner_barrier_handler(void* arg){
	uint32_t gpio_num = (uint32_t) arg;
	ets_printf("GPIO %d triggered event.\n", gpio_num);
	gpio_isr_handler_remove(gpio_num);
	xQueueSendFromISR(lightBarrierEventQueue, &gpio_num, NULL);
}

void vTaskWrite(void *arg){
	uint32_t io_num;
	while(1){
		//TODO: only increasing so far and no sophisticated mechanism so far
		if(xQueueReceive(lightBarrierEventQueue, &io_num, portMAX_DELAY)) {
			++count;
			vTaskDelay(1000 / portTICK_RATE_MS);
			if(io_num == INPUT_B_INNER_GPIO){
				gpio_isr_handler_add(io_num, gpio_inner_barrier_handler, (void *)io_num);
			}
			if(io_num == INPUT_B_OUTER_GPIO){
				gpio_isr_handler_add(io_num, gpio_outer_barrier_handler, (void *)io_num);
			}
		}
	}
}


void vTaskDisplay(void * arg){
	//TODO: task runs always but could be also event-driven to consume less resources, i.e. writeTask sends signal to displayTask
	while(1){
		showRoomState();
		vTaskDelay((2000 / portTICK_PERIOD_MS));
	}
}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
	ESP_LOGI(TAG, "Start");

    setup_oled();
    setup_transitions();

	// Queues
	lightBarrierEventQueue = xQueueCreate(10, sizeof(uint32_t));

	// Tasks
	xTaskCreate(vTaskDisplay, "vTaskDisplay", 2048, NULL, 10, NULL);
	xTaskCreate(vTaskWrite, "vTaskWrite", 2048, NULL, 10, NULL);

    // Install GPIO ISR Service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // Hook handlers
    gpio_isr_handler_add(INPUT_B_OUTER_GPIO, gpio_outer_barrier_handler, (void*) INPUT_B_OUTER_GPIO);
    gpio_isr_handler_add(INPUT_B_INNER_GPIO, gpio_inner_barrier_handler, (void*) INPUT_B_INNER_GPIO);
}
