#include "ef_button.h"

#include <stddef.h>

#define EF_BUTTON_DEFAULT_DEBOUNCE_MS 20U
#define EF_BUTTON_DEFAULT_CLICK_MS 300U
#define EF_BUTTON_DEFAULT_LONG_PRESS_MS 800U

static uint16_t ef_button_pick_u16(uint16_t value, uint16_t fallback)
{
    return (value == 0U) ? fallback : value;
}

void ef_button_init(ef_button_t *button, const ef_button_config_t *config, bool pressed, uint32_t now_ms)
{
    if (button == NULL) {
        return;
    }

    if (config != NULL) {
        button->config.debounce_ms = ef_button_pick_u16(config->debounce_ms, EF_BUTTON_DEFAULT_DEBOUNCE_MS);
        button->config.click_ms = ef_button_pick_u16(config->click_ms, EF_BUTTON_DEFAULT_CLICK_MS);
        button->config.long_press_ms = ef_button_pick_u16(config->long_press_ms, EF_BUTTON_DEFAULT_LONG_PRESS_MS);
    } else {
        button->config.debounce_ms = EF_BUTTON_DEFAULT_DEBOUNCE_MS;
        button->config.click_ms = EF_BUTTON_DEFAULT_CLICK_MS;
        button->config.long_press_ms = EF_BUTTON_DEFAULT_LONG_PRESS_MS;
    }

    button->stable_pressed = pressed;
    button->last_raw_pressed = pressed;
    button->long_reported = false;
    button->click_pending = false;
    button->raw_changed_ms = now_ms;
    button->press_start_ms = now_ms;
    button->release_ms = now_ms;
}

ef_button_event_t ef_button_update(ef_button_t *button, bool pressed, uint32_t now_ms)
{
    ef_button_event_t event = EF_BUTTON_EVENT_NONE;

    if (button == NULL) {
        return EF_BUTTON_EVENT_NONE;
    }

    if (pressed != button->last_raw_pressed) {
        button->last_raw_pressed = pressed;
        button->raw_changed_ms = now_ms;
    }

    if ((pressed != button->stable_pressed) &&
        ((uint32_t) (now_ms - button->raw_changed_ms) >= button->config.debounce_ms)) {
        button->stable_pressed = pressed;
        if (pressed) {
            button->press_start_ms = now_ms;
            button->long_reported = false;
            event = EF_BUTTON_EVENT_DOWN;
        } else {
            event = EF_BUTTON_EVENT_UP;
            if (button->long_reported) {
                button->click_pending = false;
            } else if (button->click_pending &&
                ((uint32_t) (now_ms - button->release_ms) <= button->config.click_ms)) {
                button->click_pending = false;
                event = EF_BUTTON_EVENT_DOUBLE_CLICK;
            } else {
                button->click_pending = true;
                button->release_ms = now_ms;
            }
        }
    }

    if ((event == EF_BUTTON_EVENT_NONE) && button->stable_pressed && !button->long_reported &&
        ((uint32_t) (now_ms - button->press_start_ms) >= button->config.long_press_ms)) {
        button->long_reported = true;
        button->click_pending = false;
        event = EF_BUTTON_EVENT_LONG_PRESS;
    }

    if ((event == EF_BUTTON_EVENT_NONE) && button->click_pending &&
        ((uint32_t) (now_ms - button->release_ms) > button->config.click_ms)) {
        button->click_pending = false;
        event = EF_BUTTON_EVENT_CLICK;
    }

    return event;
}

const char *ef_button_event_name(ef_button_event_t event)
{
    switch (event) {
    case EF_BUTTON_EVENT_DOWN:
        return "DOWN";
    case EF_BUTTON_EVENT_UP:
        return "UP";
    case EF_BUTTON_EVENT_CLICK:
        return "CLICK";
    case EF_BUTTON_EVENT_DOUBLE_CLICK:
        return "DOUBLE";
    case EF_BUTTON_EVENT_LONG_PRESS:
        return "LONG";
    case EF_BUTTON_EVENT_NONE:
    default:
        return "NONE";
    }
}
