#include "ef_event.h"

static const ef_event_binding_t *g_bindings;
static size_t g_binding_count;

void ef_event_init(const ef_event_binding_t *bindings, size_t binding_count)
{
    g_bindings = bindings;
    g_binding_count = binding_count;
}

void ef_event_publish(ef_event_id_t id, const void *payload)
{
    for (size_t i = 0U; i < g_binding_count; i++) {
        if (g_bindings[i].id == id && g_bindings[i].handler != NULL) {
            g_bindings[i].handler(id, payload, g_bindings[i].ctx);
        }
    }
}
