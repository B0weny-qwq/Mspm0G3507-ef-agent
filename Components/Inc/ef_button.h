#ifndef EF_BUTTON_H
#define EF_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_BUTTON_EVENT_NONE = 0,
    EF_BUTTON_EVENT_DOWN,
    EF_BUTTON_EVENT_UP,
    EF_BUTTON_EVENT_CLICK,
    EF_BUTTON_EVENT_DOUBLE_CLICK,
    EF_BUTTON_EVENT_LONG_PRESS,
} ef_button_event_t;

typedef struct {
    uint16_t debounce_ms;
    uint16_t click_ms;
    uint16_t long_press_ms;
} ef_button_config_t;

typedef struct {
    ef_button_config_t config;
    bool stable_pressed;
    bool last_raw_pressed;
    bool long_reported;
    bool click_pending;
    uint32_t raw_changed_ms;
    uint32_t press_start_ms;
    uint32_t release_ms;
} ef_button_t;

void ef_button_init(ef_button_t *button, const ef_button_config_t *config, bool pressed, uint32_t now_ms);
ef_button_event_t ef_button_update(ef_button_t *button, bool pressed, uint32_t now_ms);
const char *ef_button_event_name(ef_button_event_t event);

#ifdef __cplusplus
}
#endif

#endif
