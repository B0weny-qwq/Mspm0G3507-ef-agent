#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化应用层。
 *
 * 该函数负责初始化调度器、板级设备、日志和演示逻辑。
 *
 * @param idle 调度器空闲回调，可为 `NULL`。
 */
void app_init(void (*idle)(void));

/**
 * @brief 运行应用主循环。
 *
 * 通常在 `main()` 中初始化完成后调用，不返回。
 */
void app_run(void);

#ifdef __cplusplus
}
#endif

#endif
