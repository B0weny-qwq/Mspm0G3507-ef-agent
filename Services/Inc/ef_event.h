#ifndef EF_EVENT_H
#define EF_EVENT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t ef_event_id_t;
typedef void (*ef_event_handler_t)(ef_event_id_t id, const void *payload, void *ctx);

typedef struct {
    ef_event_id_t id;
    ef_event_handler_t handler;
    void *ctx;
} ef_event_binding_t;

void ef_event_init(const ef_event_binding_t *bindings, size_t binding_count);
void ef_event_publish(ef_event_id_t id, const void *payload);

#ifdef __cplusplus
}
#endif

#endif
