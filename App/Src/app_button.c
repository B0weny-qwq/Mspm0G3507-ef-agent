#include "app_button.h"

#include "board_button.h"

#include <stddef.h>

#define APP_BUTTON_MAX_HANDLERS 4U

typedef struct {
    app_button_handler_t handler;
    void *ctx;
} app_button_binding_t;

static ef_button_t g_boot_button;
static uint32_t g_button_time_ms;
static ef_button_event_t g_last_events[APP_BUTTON_COUNT];
static app_button_binding_t g_handlers[APP_BUTTON_MAX_HANDLERS];
static uint8_t g_handler_count;

static bool app_button_raw_pressed(app_button_id_t id)
{
    switch (id) {
    case APP_BUTTON_BOOT:
        return board_button_is_pressed(BOARD_BUTTON_BOOT);
    default:
        return false;
    }
}

static void app_button_dispatch(app_button_id_t id, ef_button_event_t event)
{
    g_last_events[id] = event;

    for (uint8_t i = 0U; i < g_handler_count; i++) {
        if (g_handlers[i].handler != NULL) {
            g_handlers[i].handler(id, event, g_handlers[i].ctx);
        }
    }
}

void app_button_init(void)
{
    board_button_init();
    g_button_time_ms = 0U;
    g_handler_count = 0U;
    for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++) {
        g_last_events[i] = EF_BUTTON_EVENT_NONE;
    }

    ef_button_init(&g_boot_button, NULL, app_button_raw_pressed(APP_BUTTON_BOOT), g_button_time_ms);
}

void app_button_tick_10ms(void)
{
    ef_button_event_t event;

    g_button_time_ms += 10U;
    event = ef_button_update(&g_boot_button, app_button_raw_pressed(APP_BUTTON_BOOT), g_button_time_ms);

    if (event != EF_BUTTON_EVENT_NONE) {
        app_button_dispatch(APP_BUTTON_BOOT, event);
    }
}

bool app_button_register_handler(app_button_handler_t handler, void *ctx)
{
    if ((handler == NULL) || (g_handler_count >= APP_BUTTON_MAX_HANDLERS)) {
        return false;
    }

    g_handlers[g_handler_count].handler = handler;
    g_handlers[g_handler_count].ctx = ctx;
    g_handler_count++;
    return true;
}

bool app_button_is_pressed(app_button_id_t id)
{
    return app_button_raw_pressed(id);
}

ef_button_event_t app_button_get_last_event(app_button_id_t id)
{
    if (id >= APP_BUTTON_COUNT) {
        return EF_BUTTON_EVENT_NONE;
    }

    return g_last_events[id];
}

const char *app_button_name(app_button_id_t id)
{
    switch (id) {
    case APP_BUTTON_BOOT:
        return "BOOT";
    default:
        return "UNKNOWN";
    }
}
