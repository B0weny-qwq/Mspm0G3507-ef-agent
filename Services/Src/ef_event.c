#include "ef_event.h"

/* 轻量事件分发器：按事件 ID 遍历绑定表并同步调用处理函数。 */

static const ef_event_binding_t *g_bindings;
static size_t g_binding_count;

/* 保存当前事件绑定表。 */
void ef_event_init(const ef_event_binding_t *bindings, size_t binding_count)
{
    g_bindings = bindings;
    g_binding_count = binding_count;
}

/* 按顺序向所有匹配的订阅者发布事件。 */
void ef_event_publish(ef_event_id_t id, const void *payload)
{
    for (size_t i = 0U; i < g_binding_count; i++) {
        if (g_bindings[i].id == id && g_bindings[i].handler != NULL) {
            g_bindings[i].handler(id, payload, g_bindings[i].ctx);
        }
    }
}
