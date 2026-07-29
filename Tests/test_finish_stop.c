#include "app_finish_stop.h"

#include <assert.h>
#include <stdint.h>

int main(void)
{
    uint32_t start_us;
    uint32_t now_us;

    app_finish_stop_init();
    assert(!app_finish_stop_is_running());
    assert(app_finish_stop_get_elapsed_us(6000000U) == 0U);
    assert(!app_finish_stop_update(6000000U, APP_FINISH_STOP_MASK));

    app_finish_stop_on_pwm_started(100U);
    assert(app_finish_stop_is_running());
    assert(app_finish_stop_get_elapsed_us(1100U) == 1000U);
    assert(!app_finish_stop_update(4999999U, APP_FINISH_STOP_MASK));
    assert(!app_finish_stop_update(5000100U, UINT16_C(0xF81F)));
    assert(!app_finish_stop_update(5010100U, APP_FINISH_STOP_MASK));
    assert(!app_finish_stop_update(5020100U, UINT16_C(0x03E0)));
    assert(!app_finish_stop_update(5030100U, UINT16_C(0xF7E1)));
    assert(app_finish_stop_update(5040100U, APP_FINISH_STOP_MASK));
    assert(!app_finish_stop_is_running());
    assert(app_finish_stop_get_elapsed_us(9000000U) == 5040000U);
    assert(!app_finish_stop_update(5050100U, APP_FINISH_STOP_MASK));

    app_finish_stop_on_pwm_started(7000000U);
    app_finish_stop_stop(7500000U);
    assert(app_finish_stop_get_elapsed_us(13000000U) == 500000U);
    assert(!app_finish_stop_update(13000000U, APP_FINISH_STOP_MASK));

    start_us = UINT32_MAX - 1000000U;
    now_us = start_us;
    app_finish_stop_on_pwm_started(start_us);
    now_us += APP_FINISH_STOP_DELAY_US;
    assert(!app_finish_stop_update(now_us, APP_FINISH_STOP_MASK));
    assert(app_finish_stop_update(now_us + 10000U, APP_FINISH_STOP_MASK));

    return 0;
}
