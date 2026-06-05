#include "ef_lowpass.h"

#include <stddef.h>

/* 整数一阶 IIR 低通，适合小 MCU 上的速度显示/控制前处理。 */

static int32_t ef_lowpass_abs_i32(int32_t value)
{
    return (value < 0) ? -value : value;
}

void ef_lowpass_i32_init(ef_lowpass_i32_t *filter, uint8_t shift, int32_t zero_threshold)
{
    if (filter == NULL) {
        return;
    }

    if (shift == 0U) {
        shift = 1U;
    }
    if (shift > 8U) {
        shift = 8U;
    }

    filter->value = 0;
    filter->shift = shift;
    filter->zero_threshold = (zero_threshold < 0) ? -zero_threshold : zero_threshold;
}

int32_t ef_lowpass_i32_update(ef_lowpass_i32_t *filter, int32_t input)
{
    int32_t error;

    if (filter == NULL) {
        return input;
    }

    if ((input == 0) && (ef_lowpass_abs_i32(filter->value) <= filter->zero_threshold)) {
        filter->value = 0;
        return 0;
    }

    error = input - filter->value;
    if (error >= 0) {
        filter->value += (error + (int32_t) ((1U << filter->shift) - 1U)) >> filter->shift;
    } else {
        filter->value -= ((-error) + (int32_t) ((1U << filter->shift) - 1U)) >> filter->shift;
    }

    return filter->value;
}
