#ifndef EF_CAPTURE_H
#define EF_CAPTURE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 输入捕获通道编号。
 */
typedef enum {
    /** 编码器 1 step 捕获：PA28/TIMG7_CCP0。 */
    EF_CAPTURE_ENCODER1_STEP = 0,
    /** 编码器 2 step 捕获：PA26/TIMG8_CCP0。 */
    EF_CAPTURE_ENCODER2_STEP,
} ef_capture_id_t;

/**
 * @brief 输入捕获事件回调。
 *
 * @param id 捕获通道编号。
 * @param ctx 用户上下文。
 */
typedef void (*ef_capture_handler_t)(ef_capture_id_t id, void *ctx);

/**
 * @brief 初始化输入捕获驱动。
 */
void ef_capture_init(void);

/**
 * @brief 注册输入捕获事件回调。
 *
 * @param handler 回调函数，可为 `NULL`。
 * @param ctx 用户上下文。
 */
void ef_capture_set_handler(ef_capture_handler_t handler, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
