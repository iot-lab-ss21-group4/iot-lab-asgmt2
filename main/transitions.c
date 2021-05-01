#include "common.h"
#include "transitions.h"

#define FLAG_COUNT 2 // number of barrier bits (flags)
#define INNER_BARRIER_FLAG (1 << 1)
#define OUTER_BARRIER_FLAG (1 << 0)
#define INNER_BARRIER_PIN CONFIG_INNER_BARRIER_PIN
#define OUTER_BARRIER_PIN CONFIG_OUTER_BARRIER_PIN
#define ESP_INTR_FLAG_DEFAULT 0
// minimum count of people in the room
#define MIN_ROOM_COUNT 0
// assuming that count_display_q_item is an unsigned type
#define MAX_ROOM_COUNT ((count_display_q_item)(-1))
#define TRANSITION_TIMER_MS 10
#define TRANSITION_TIMER_TICKS MAX(TRANSITION_TIMER_MS / portTICK_PERIOD_MS, 1)
#define IS_RISING_EDGE_GRIO_INTR_VAL(is_rising) ((is_rising) ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE)
#define WAS_RISING_EDGE_GRIO_INTR_VAL(was_rising) ((was_rising) ? GPIO_INTR_NEGEDGE : GPIO_INTR_POSEDGE)

typedef uint8_t barrier_evt_q_item;

static barrier_evt_q_item FSM_STATE = 0;
static const uint16_t BARRIER_EVT_Q_SIZE = 32;
static xQueueHandle barrier_evt_q = NULL;
static volatile bool is_inner_rising = true;
static volatile bool is_outer_rising = true;
static TimerHandle_t inner_transition_timer = NULL;
static TimerHandle_t outer_transition_timer = NULL;

static void inner_transition_timer_callback(TimerHandle_t);
static void outer_transition_timer_callback(TimerHandle_t);
static void apply_state_change(barrier_evt_q_item);
static void transition_handling_task(void *);
static void initialize_null_handles();
static void IRAM_ATTR inner_barrier_pin_isr(void *_)
{
    static const barrier_evt_q_item state_change = INNER_BARRIER_FLAG;
    xQueueSendFromISR(barrier_evt_q, &state_change, NULL);
    gpio_isr_handler_remove(INNER_BARRIER_PIN);
    // if we fail to reset the transition timer, then we must register
    // the next opposite edge isr ourselves.
    is_inner_rising = LOGICAL_NOT(is_inner_rising);
    if (xTimerResetFromISR(inner_transition_timer, NULL) != pdPASS)
    {
        gpio_set_intr_type(INNER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_inner_rising));
        gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
    }
}
static void IRAM_ATTR outer_barrier_pin_isr(void *_)
{
    static const barrier_evt_q_item state_change = OUTER_BARRIER_FLAG;
    xQueueSendFromISR(barrier_evt_q, &state_change, NULL);
    gpio_isr_handler_remove(OUTER_BARRIER_PIN);
    // if we fail to reset the transition timer, then we must register
    // the next opposite edge isr ourselves.
    is_outer_rising = LOGICAL_NOT(is_outer_rising);
    if (xTimerResetFromISR(outer_transition_timer, NULL) != pdPASS)
    {
        gpio_set_intr_type(OUTER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_outer_rising));
        gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);
    }
}

static void inner_transition_timer_callback(TimerHandle_t timer)
{
    gpio_set_intr_type(INNER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_inner_rising));
    gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
}
static void outer_transition_timer_callback(TimerHandle_t timer)
{
    gpio_set_intr_type(OUTER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_outer_rising));
    gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);
}
static void apply_state_change(barrier_evt_q_item state_change)
{
    barrier_evt_q_item prev_new_bits = FSM_STATE & FIRST_N_BITMASK(FLAG_COUNT);
    // Replace old state bits with new state bits.
    FSM_STATE = (FSM_STATE << FLAG_COUNT) & FIRST_N_BITMASK(2 * FLAG_COUNT);
    // XOR with changed state bits (1 for changed 0 for unchanged).
    FSM_STATE ^= prev_new_bits ^ (state_change & FIRST_N_BITMASK(FLAG_COUNT));
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
static void initialize_null_handles()
{
    if (barrier_evt_q == NULL)
    {
        barrier_evt_q = xQueueCreate(BARRIER_EVT_Q_SIZE, sizeof(barrier_evt_q_item));
    }
    if (inner_transition_timer == NULL)
    {
        inner_transition_timer = xTimerCreate("inner_transition_timer", TRANSITION_TIMER_TICKS, pdFALSE, NULL, inner_transition_timer_callback);
    }
    if (outer_transition_timer == NULL)
    {
        outer_transition_timer = xTimerCreate("outer_transition_timer", TRANSITION_TIMER_TICKS, pdFALSE, NULL, outer_transition_timer_callback);
    }
}

void setup_transitions()
{
    initialize_null_handles();

    gpio_set_direction(INNER_BARRIER_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(OUTER_BARRIER_PIN, GPIO_MODE_INPUT);

    gpio_pulldown_en(INNER_BARRIER_PIN);
    gpio_pulldown_en(OUTER_BARRIER_PIN);

    gpio_set_intr_type(INNER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_inner_rising));
    gpio_set_intr_type(OUTER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_outer_rising));

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
    gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);

    xTaskCreate(transition_handling_task, "transition_handling_task", 4096, NULL, 9, NULL);
}

void loop_transitions()
{
    ESP_LOGI(TAG, "Sensor state: " BYTE_TO_BINARY_PATTERN ", FSM state: " BYTE_TO_BINARY_PATTERN,
             BYTE_TO_BINARY((gpio_get_level(INNER_BARRIER_PIN) * INNER_BARRIER_FLAG) |
                            (gpio_get_level(OUTER_BARRIER_PIN) * OUTER_BARRIER_FLAG)),
             BYTE_TO_BINARY(FSM_STATE));
}
