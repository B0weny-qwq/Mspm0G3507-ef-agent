#ifndef EF_LOWPASS_H
#define EF_LOWPASS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 整数一阶低通滤波器。
 */
typedef struct {
    int32_t value;
    uint8_t shift;
    int32_t zero_threshold;
} ef_lowpass_i32_t;

/**
 * @brief 初始化整数低通滤波器。
 *
 * alpha = 1 / (2 ^ shift)。例如 shift=2 表示每次保留 3/4 旧值、加入 1/4 新值。
 *
 * @param filter 滤波器对象。
 * @param shift 滤波强度，建议 1 到 4。
 * @param zero_threshold 输入为 0 且输出绝对值不大于该阈值时直接归零。
 */
void ef_lowpass_i32_init(ef_lowpass_i32_t *filter, uint8_t shift, int32_t zero_threshold);

/**
 * @brief 更新滤波器并返回滤波后的值。
 *
 * @param filter 滤波器对象。
 * @param input 新输入值。
 * @return int32_t 滤波输出。
 */
int32_t ef_lowpass_i32_update(ef_lowpass_i32_t *filter, int32_t input);

#ifdef __cplusplus
}
#endif

#endif
