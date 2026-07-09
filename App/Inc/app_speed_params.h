#ifndef APP_SPEED_PARAMS_H
#define APP_SPEED_PARAMS_H

#include "app_speed_control.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 执行速度参数存储的启动自检。
 *
 * 自检只读取 JEDEC ID 和参数扇区，不擦写 Flash。失败后本次运行禁止保存参数。
 *
 * @return `true` 外部 Flash 可读，允许后续加载和保存。
 * @return `false` 外部 Flash 未读取到或参数扇区不可读。
 */
bool app_speed_params_probe(void);

/**
 * @brief 从非易失存储读取速度环调参数据。
 *
 * 只恢复 Target、Dir、Kp、Ki、Kd，不恢复 enabled，避免上电自动启动车轮。
 *
 * @param config 输出配置。
 * @return `true` 参数有效并已写入 config。
 * @return `false` 没有有效参数，调用方应继续使用默认值。
 */
bool app_speed_params_load(app_speed_control_config_t *config);

/**
 * @brief 保存速度环调参数据到非易失存储。
 *
 * @param config 当前速度环配置。
 * @return `true` 保存成功。
 * @return `false` 保存失败。
 */
bool app_speed_params_save(const app_speed_control_config_t *config);

#ifdef __cplusplus
}
#endif

#endif
