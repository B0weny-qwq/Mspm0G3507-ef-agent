#ifndef EF_EVENT_H
#define EF_EVENT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 事件编号类型。 */
typedef uint16_t ef_event_id_t;

/**
 * @brief 事件处理回调。
 *
 * @param id 事件编号。
 * @param payload 事件负载。
 * @param ctx 用户上下文。
 */
typedef void (*ef_event_handler_t)(ef_event_id_t id, const void *payload, void *ctx);

/**
 * @brief 事件绑定表项。
 */
typedef struct {
    ef_event_id_t id;
    ef_event_handler_t handler;
    void *ctx;
} ef_event_binding_t;

/**
 * @brief 初始化事件系统。
 *
 * @param bindings 事件绑定表。
 * @param binding_count 绑定数量。
 */
void ef_event_init(const ef_event_binding_t *bindings, size_t binding_count);

/**
 * @brief 发布一个事件。
 *
 * @param id 事件编号。
 * @param payload 事件负载。
 */
void ef_event_publish(ef_event_id_t id, const void *payload);

#ifdef __cplusplus
}
#endif

#endif
