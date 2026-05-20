#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "ef_button.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_BUTTON_BOOT = 0,
    APP_BUTTON_COUNT,
} app_button_id_t;

typedef void (*app_button_handler_t)(app_button_id_t id, ef_button_event_t event, void *ctx);

void app_button_init(void);
void app_button_tick_10ms(void);
bool app_button_register_handler(app_button_handler_t handler, void *ctx);
bool app_button_is_pressed(app_button_id_t id);
ef_button_event_t app_button_get_last_event(app_button_id_t id);
const char *app_button_name(app_button_id_t id);

#ifdef __cplusplus
}
#endif

#endif
