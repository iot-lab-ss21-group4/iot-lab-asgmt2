#include "common.h"
#include "oled.h"
#include "transitions.h"

void app_main(void)
{
    init_logging();
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
