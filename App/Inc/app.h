#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void (*idle)(void));
void app_run(void);

#ifdef __cplusplus
}
#endif

#endif
