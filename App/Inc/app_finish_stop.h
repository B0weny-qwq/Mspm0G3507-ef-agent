#ifndef APP_FINISH_STOP_H
#define APP_FINISH_STOP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_FINISH_STOP_DELAY_US 5000000UL
#define APP_FINISH_STOP_REQUIRED_SCANS 2U
/** Middle six grayscale channels (5..10) must all be active. */
#define APP_FINISH_STOP_MASK UINT16_C(0x07E0)

/** Reset the finish-line detector and elapsed-time display state. */
void app_finish_stop_init(void);
/** Start a new run on the first actual nonzero motor PWM output. */
void app_finish_stop_on_pwm_started(uint32_t now_us);
/** Freeze the elapsed time and disarm finish-line detection. */
void app_finish_stop_stop(uint32_t now_us);
/** Return true once after two consecutive middle-six matches past the delay. */
bool app_finish_stop_update(uint32_t now_us, uint16_t active_mask);
/** Return whether a run is currently being timed. */
bool app_finish_stop_is_running(void);
/** Return live or frozen elapsed time for the current run. */
uint32_t app_finish_stop_get_elapsed_us(uint32_t now_us);

#ifdef __cplusplus
}
#endif

#endif
