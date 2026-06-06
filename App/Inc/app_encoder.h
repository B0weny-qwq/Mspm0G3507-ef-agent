#ifndef APP_ENCODER_H
#define APP_ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 应用层编码器编号。
 */
typedef enum {
    /** 左/1 号编码器。 */
    APP_ENCODER_1 = 0,
    /** 右/2 号编码器。 */
    APP_ENCODER_2,
    /** 编码器数量。 */
    APP_ENCODER_COUNT,
} app_encoder_id_t;

/**
 * @brief 编码器应用层快照。
 *
 * `delta_steps` 表示最近一次 50 ms 采样窗口内的有符号增量；`speed_50ms`
 * 表示低通后的 step/50ms 速度。该结构供惯导、里程计和控制模块读取，
 * 不暴露板级定时器、GPIO 或安装方向细节。
 */
typedef struct {
    int32_t delta_steps[APP_ENCODER_COUNT];
    int32_t speed_50ms[APP_ENCODER_COUNT];
    uint32_t timestamp_us;
    bool valid;
} app_encoder_snapshot_t;

/**
 * @brief 初始化应用层编码器采样模块。
 *
 * 该模块封装板级 step/dir 编码器、速度低通滤波和 LCD 状态行更新。
 */
void app_encoder_init(void);

/**
 * @brief 50 ms 周期编码器采样入口。
 *
 * 读取两路编码器增量，做一阶低通滤波，并把结果作为 step/50ms 速度显示。
 */
void app_encoder_tick_50ms(void);

/**
 * @brief 获取最近一次编码器采样快照。
 *
 * 该接口只返回 App 层保存的采样结果，不触发新的底层读取。
 *
 * @param snapshot 输出快照，不能为 NULL。
 * @return true 快照有效。
 * @return false 参数无效或尚未完成首次采样。
 */
bool app_encoder_get_snapshot(app_encoder_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif
