#include "common.h"
#include "transitions.h"

#define FLAG_COUNT 2 // number of barrier bits (flags)
#define INNER_BARRIER_FLAG (1 << 0)
#define OUTER_BARRIER_FLAG (1 << 1)
#define INNER_BARRIER_PIN CONFIG_INNER_BARRIER_PIN
#define OUTER_BARRIER_PIN CONFIG_OUTER_BARRIER_PIN
#define ESP_INTR_FLAG_DEFAULT 0
#define POWER_OF_TWO(x) (1 << (x))
#define MIN_ROOM_COUNT 0
#define MAX_ROOM_COUNT POWER_OF_TWO((sizeof((count))*8)) - 1

typedef uint8_t barrier_evt_q_item;

static barrier_evt_q_item FSM_STATE = 0;
static const uint16_t BARRIER_EVT_Q_SIZE = 16;
static xQueueHandle barrier_evt_q = NULL;

static void apply_state_change(barrier_evt_q_item);
static void transition_handling_task(void *);
static void IRAM_ATTR inner_barrier_pin_isr(void *_)
{
    barrier_evt_q_item state_change = INNER_BARRIER_FLAG;
    xQueueSendFromISR(barrier_evt_q, &state_change, NULL);
}
static void IRAM_ATTR outer_barrier_pin_isr(void *_)
{
    barrier_evt_q_item state_change = OUTER_BARRIER_FLAG;
    xQueueSendFromISR(barrier_evt_q, &state_change, NULL);
}

static void apply_state_change(barrier_evt_q_item state_change)
{
    barrier_evt_q_item prev_new_bits = FSM_STATE & FIRST_N_BITMASK(FLAG_COUNT);
    // Replace old state bits with new state bits.
    FSM_STATE = (FSM_STATE << FLAG_COUNT) & FIRST_N_BITMASK(2 * FLAG_COUNT);
    // XOR with changed state bits (1 for changed 0 for unchanged).
    FSM_STATE ^= prev_new_bits ^ (state_change & FIRST_N_BITMASK(FLAG_COUNT));
    ESP_LOGI(TAG, "State: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(FSM_STATE));
}
static void transition_handling_task(void *_)
{
    barrier_evt_q_item state_change;
    while (1)
    {
        while (xQueueReceive(barrier_evt_q, &state_change, portMAX_DELAY) != pdTRUE)
            ;
        // TODO: check which pins have changed from "state_change" and change
        // count depending on the FSM state transitions defined in the documents.
        apply_state_change(state_change);
        switch (state_change)
        {
        case INNER_BARRIER_FLAG:
            count = (count < MAX_ROOM_COUNT) ? count + 1 : count;
            break;
        case OUTER_BARRIER_FLAG:
            count = (count > MIN_ROOM_COUNT) ? count - 1 : count;
            break;
        default:
            break;
        }
        xQueueSend(count_display_q, (const void *)&count, portMAX_DELAY);
    }
}

void setup_transitions()
{
    if (barrier_evt_q == NULL)
    {
        barrier_evt_q = xQueueCreate(BARRIER_EVT_Q_SIZE, sizeof(barrier_evt_q_item));
    }

    gpio_set_direction(OUTER_BARRIER_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(INNER_BARRIER_PIN, GPIO_MODE_INPUT);

    gpio_pulldown_en(OUTER_BARRIER_PIN);
    gpio_pulldown_en(INNER_BARRIER_PIN);

    gpio_set_intr_type(OUTER_BARRIER_PIN, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(INNER_BARRIER_PIN, GPIO_INTR_POSEDGE);
    //Other EDGES
    //GPIO_INTR_ANYEDGE
    //GPIO_INTR_POSEDGE
    //GPIO_INTR_NEGEDGE
    //GPIO_INTR_LOW_LEVEL
    //GPIO_INTR_HIGH_LEVEL

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);

    xTaskCreate(transition_handling_task, "transition_handling_task", 4096, NULL, 9, NULL);
}

void loop_transitions()
{
}
