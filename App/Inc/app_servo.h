#ifndef APP_SERVO_H
#define APP_SERVO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t center_pulse_us;
    uint16_t minimum_pulse_us;
    uint16_t maximum_pulse_us;
    uint16_t proportional_us;
    uint16_t derivative_us;
} app_servo_config_t;

bool app_servo_init(void);
bool app_servo_set_config(const app_servo_config_t *config);
void app_servo_get_config(app_servo_config_t *config);
void app_servo_set_enabled(bool enabled);
bool app_servo_is_enabled(void);
uint16_t app_servo_get_pulse_us(void);
void app_servo_tick_20ms(void);

#ifdef __cplusplus
}
#endif

#endif
