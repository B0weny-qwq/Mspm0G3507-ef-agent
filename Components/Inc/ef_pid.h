#ifndef EF_PID_H
#define EF_PID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t kp_q8;
    int32_t ki_q8;
    int32_t kd_q8;
    int32_t integral_limit;
    int32_t output_limit;
} ef_pid_i32_config_t;

typedef struct {
    ef_pid_i32_config_t config;
    int32_t integral;
    int32_t previous_error;
} ef_pid_i32_t;

void ef_pid_i32_init(ef_pid_i32_t *pid, const ef_pid_i32_config_t *config);
void ef_pid_i32_reset(ef_pid_i32_t *pid);
int32_t ef_pid_i32_update(ef_pid_i32_t *pid, int32_t target, int32_t measured);

#ifdef __cplusplus
}
#endif

#endif
