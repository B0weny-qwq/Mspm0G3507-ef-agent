#ifndef APP_ENCODER_H
#define APP_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
