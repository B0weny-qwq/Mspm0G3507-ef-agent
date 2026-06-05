#ifndef APP_BOARD_PROBE_H
#define APP_BOARD_PROBE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 执行启动阶段板级外设探测。
 *
 * 该函数只通过 BoardDevices API 访问外设，并把探测结果写入应用状态页。
 */
void app_board_probe_run(void);

#ifdef __cplusplus
}
#endif

#endif
