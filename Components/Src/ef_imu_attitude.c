#include "ef_imu_attitude.h"

#include <stddef.h>

/**
 * @file ef_imu_attitude.c
 * @brief M0+ 适用的轻量整数 IMU 姿态滤波。
 *
 * @details
 * 本组件保持纯算法属性。当前实现用陀螺仪更新 Q15 四元数，并用加速度计重力方向误差
 * 做互补校正；输出欧拉角使用整数 atan2/sqrt 近似换算为 Q10，不依赖硬件、
 * BoardDevices、Drivers 或浮点库。
 */

enum {
    EF_IMU_Q15_ONE = 32767,
    EF_IMU_Q10_PER_DEG = 1 << EF_IMU_ATTITUDE_EULER_Q,
    EF_IMU_UG_PER_G = 1000000,
    EF_IMU_DEFAULT_ACCEL_SHIFT = 5U,
    EF_IMU_DEFAULT_EULER_LPF_SHIFT = 1U,
    EF_IMU_MAX_SHIFT = 8U,
    EF_IMU_HALF_RAD_Q15_PER_DEG = 286,
    EF_IMU_ATAN_45_DEG_Q10 = 45 * EF_IMU_Q10_PER_DEG,
    EF_IMU_ATAN_CORR_DEG_Q10 = 16015,
};

static uint8_t ef_imu_clamp_shift(uint8_t shift)
{
    if (shift == 0U) {
        return 1U;
    }
    if (shift > EF_IMU_MAX_SHIFT) {
        return EF_IMU_MAX_SHIFT;
    }
    return shift;
}

static int32_t ef_imu_lowpass_i32(int32_t current, int32_t input, uint8_t shift)
{
    const int32_t error = input - current;

    if (error >= 0) {
        return current + ((error + (int32_t) ((1U << shift) - 1U)) >> shift);
    }

    return current - (((-error) + (int32_t) ((1U << shift) - 1U)) >> shift);
}

static int32_t ef_imu_abs_i32(int32_t value)
{
    return (value < 0) ? -value : value;
}

static uint32_t ef_imu_isqrt_u64(uint64_t value)
{
    uint64_t bit = 1ULL << 62;
    uint32_t result = 0U;

    while (bit > value) {
        bit >>= 2;
    }

    while (bit != 0U) {
        if (value >= ((uint64_t) result + bit)) {
            value -= (uint64_t) result + bit;
            result = (uint32_t) (((uint64_t) result >> 1) + bit);
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }

    return result;
}

static uint64_t ef_imu_square_i32(int32_t value)
{
    const int64_t signed_value = value;
    const uint64_t magnitude = (signed_value < 0) ? (uint64_t) (-signed_value) : (uint64_t) signed_value;

    return magnitude * magnitude;
}

static int32_t ef_imu_atan_q10_from_ratio_q15(int32_t ratio_q15)
{
    const int32_t abs_ratio = ef_imu_abs_i32(ratio_q15);
    const int32_t correction = (int32_t) (((int64_t) EF_IMU_ATAN_CORR_DEG_Q10 *
        (EF_IMU_Q15_ONE - ((abs_ratio > EF_IMU_Q15_ONE) ? EF_IMU_Q15_ONE : abs_ratio))) / EF_IMU_Q15_ONE);
    const int32_t term = EF_IMU_ATAN_45_DEG_Q10 + correction;

    return (int32_t) (((int64_t) ratio_q15 * term) / EF_IMU_Q15_ONE);
}

static int32_t ef_imu_atan2_q10(int32_t y_q15, int32_t x_q15)
{
    const int32_t abs_x = ef_imu_abs_i32(x_q15);
    const int32_t abs_y = ef_imu_abs_i32(y_q15);
    int32_t angle;

    if ((x_q15 == 0) && (y_q15 == 0)) {
        return 0;
    }

    if (abs_y <= abs_x) {
        const int32_t ratio = (x_q15 == 0) ? 0 : (int32_t) (((int64_t) y_q15 * EF_IMU_Q15_ONE) / x_q15);
        angle = ef_imu_atan_q10_from_ratio_q15(ratio);
        if (x_q15 < 0) {
            angle += (y_q15 >= 0) ? (180 * EF_IMU_Q10_PER_DEG) : (-180 * EF_IMU_Q10_PER_DEG);
        }
    } else {
        const int32_t ratio = (y_q15 == 0) ? 0 : (int32_t) (((int64_t) x_q15 * EF_IMU_Q15_ONE) / y_q15);
        angle = ((y_q15 >= 0) ? (90 * EF_IMU_Q10_PER_DEG) : (-90 * EF_IMU_Q10_PER_DEG)) -
            ef_imu_atan_q10_from_ratio_q15(ratio);
    }

    return angle;
}

static int16_t ef_imu_sat_i16(int32_t value)
{
    if (value > 32767) {
        return 32767;
    }
    if (value < -32768) {
        return -32768;
    }
    return (int16_t) value;
}

static int32_t ef_imu_q15_mul(int32_t a, int32_t b)
{
    return (int32_t) (((int64_t) a * b) >> EF_IMU_ATTITUDE_QUAT_Q);
}

static int32_t ef_imu_double_q15(int32_t value)
{
    value <<= 1;
    if (value > EF_IMU_Q15_ONE) {
        return EF_IMU_Q15_ONE;
    }
    if (value < -EF_IMU_Q15_ONE) {
        return -EF_IMU_Q15_ONE;
    }
    return value;
}

static int32_t ef_imu_double_i32(int32_t value)
{
    return value << 1;
}

static bool ef_imu_normalize_vector_q15(const int32_t input[3], int32_t output[3])
{
    const uint64_t mag_sq = ef_imu_square_i32(input[0]) +
        ef_imu_square_i32(input[1]) +
        ef_imu_square_i32(input[2]);
    const uint32_t mag = ef_imu_isqrt_u64(mag_sq);

    if (mag == 0U) {
        return false;
    }

    for (uint8_t i = 0U; i < 3U; i++) {
        output[i] = (int32_t) (((int64_t) input[i] * EF_IMU_Q15_ONE) / (int32_t) mag);
    }

    return true;
}

static void ef_imu_normalize_quat(ef_imu_attitude_t *state)
{
    const int32_t q[4] = {
        state->quat_q15.w,
        state->quat_q15.x,
        state->quat_q15.y,
        state->quat_q15.z,
    };
    const uint64_t mag_sq = ef_imu_square_i32(q[0]) +
        ef_imu_square_i32(q[1]) +
        ef_imu_square_i32(q[2]) +
        ef_imu_square_i32(q[3]);
    const uint32_t mag = ef_imu_isqrt_u64(mag_sq);

    if (mag == 0U) {
        state->quat_q15.w = EF_IMU_Q15_ONE;
        state->quat_q15.x = 0;
        state->quat_q15.y = 0;
        state->quat_q15.z = 0;
        return;
    }

    state->quat_q15.w = ef_imu_sat_i16((int32_t) (((int64_t) q[0] * EF_IMU_Q15_ONE) / (int32_t) mag));
    state->quat_q15.x = ef_imu_sat_i16((int32_t) (((int64_t) q[1] * EF_IMU_Q15_ONE) / (int32_t) mag));
    state->quat_q15.y = ef_imu_sat_i16((int32_t) (((int64_t) q[2] * EF_IMU_Q15_ONE) / (int32_t) mag));
    state->quat_q15.z = ef_imu_sat_i16((int32_t) (((int64_t) q[3] * EF_IMU_Q15_ONE) / (int32_t) mag));
}

static void ef_imu_estimate_gravity_q15(const ef_imu_attitude_t *state, int32_t gravity_q15[3])
{
    const int32_t w = state->quat_q15.w;
    const int32_t x = state->quat_q15.x;
    const int32_t y = state->quat_q15.y;
    const int32_t z = state->quat_q15.z;

    gravity_q15[0] = ef_imu_double_q15(ef_imu_q15_mul(x, z) - ef_imu_q15_mul(w, y));
    gravity_q15[1] = ef_imu_double_q15(ef_imu_q15_mul(w, x) + ef_imu_q15_mul(y, z));
    gravity_q15[2] = ef_imu_q15_mul(w, w) - ef_imu_q15_mul(x, x) - ef_imu_q15_mul(y, y) + ef_imu_q15_mul(z, z);
}

static void ef_imu_apply_quat_delta(ef_imu_attitude_t *state, const int32_t half_delta_q15[3])
{
    const int32_t w = state->quat_q15.w;
    const int32_t x = state->quat_q15.x;
    const int32_t y = state->quat_q15.y;
    const int32_t z = state->quat_q15.z;
    const int32_t hx = half_delta_q15[0];
    const int32_t hy = half_delta_q15[1];
    const int32_t hz = half_delta_q15[2];

    state->quat_q15.w = ef_imu_sat_i16(w - ef_imu_q15_mul(x, hx) - ef_imu_q15_mul(y, hy) - ef_imu_q15_mul(z, hz));
    state->quat_q15.x = ef_imu_sat_i16(x + ef_imu_q15_mul(w, hx) + ef_imu_q15_mul(y, hz) - ef_imu_q15_mul(z, hy));
    state->quat_q15.y = ef_imu_sat_i16(y + ef_imu_q15_mul(w, hy) - ef_imu_q15_mul(x, hz) + ef_imu_q15_mul(z, hx));
    state->quat_q15.z = ef_imu_sat_i16(z + ef_imu_q15_mul(w, hz) + ef_imu_q15_mul(x, hy) - ef_imu_q15_mul(y, hx));
    ef_imu_normalize_quat(state);
}

static void ef_imu_update_euler_from_quat(ef_imu_attitude_t *state)
{
    const int32_t w = state->quat_q15.w;
    const int32_t x = state->quat_q15.x;
    const int32_t y = state->quat_q15.y;
    const int32_t z = state->quat_q15.z;
    const int32_t roll_y = ef_imu_double_i32(ef_imu_q15_mul(w, x) + ef_imu_q15_mul(y, z));
    const int32_t roll_x = EF_IMU_Q15_ONE - ef_imu_double_i32(ef_imu_q15_mul(x, x) + ef_imu_q15_mul(y, y));
    const int32_t sin_pitch = ef_imu_double_q15(ef_imu_q15_mul(w, y) - ef_imu_q15_mul(z, x));
    const int32_t yaw_y = ef_imu_double_i32(ef_imu_q15_mul(w, z) + ef_imu_q15_mul(x, y));
    const int32_t yaw_x = EF_IMU_Q15_ONE - ef_imu_double_i32(ef_imu_q15_mul(y, y) + ef_imu_q15_mul(z, z));
    const int64_t pitch_cos_sq = ((int64_t) EF_IMU_Q15_ONE * EF_IMU_Q15_ONE) - ((int64_t) sin_pitch * sin_pitch);
    const int32_t pitch_cos = (pitch_cos_sq <= 0) ? 0 : (int32_t) ef_imu_isqrt_u64((uint64_t) pitch_cos_sq);

    state->target_euler_q10.roll_deg_q10 = ef_imu_atan2_q10(roll_y, roll_x);
    state->target_euler_q10.pitch_deg_q10 = ef_imu_atan2_q10(sin_pitch, pitch_cos);
    state->target_euler_q10.yaw_deg_q10 = ef_imu_atan2_q10(yaw_y, yaw_x);
    state->euler_q10.roll_deg_q10 = ef_imu_lowpass_i32(
        state->euler_q10.roll_deg_q10, state->target_euler_q10.roll_deg_q10, state->config.euler_lowpass_shift);
    state->euler_q10.pitch_deg_q10 = ef_imu_lowpass_i32(
        state->euler_q10.pitch_deg_q10, state->target_euler_q10.pitch_deg_q10, state->config.euler_lowpass_shift);
    state->euler_q10.yaw_deg_q10 = ef_imu_lowpass_i32(
        state->euler_q10.yaw_deg_q10, state->target_euler_q10.yaw_deg_q10, state->config.euler_lowpass_shift);
}

void ef_imu_attitude_init(ef_imu_attitude_t *state, const ef_imu_attitude_config_t *config)
{
    if (state == NULL) {
        return;
    }

    state->quat_q15.w = EF_IMU_Q15_ONE;
    state->quat_q15.x = 0;
    state->quat_q15.y = 0;
    state->quat_q15.z = 0;
    state->target_euler_q10.roll_deg_q10 = 0;
    state->target_euler_q10.pitch_deg_q10 = 0;
    state->target_euler_q10.yaw_deg_q10 = 0;
    state->euler_q10.roll_deg_q10 = 0;
    state->euler_q10.pitch_deg_q10 = 0;
    state->euler_q10.yaw_deg_q10 = 0;
    if (config != NULL) {
        state->config = *config;
    } else {
        state->config.accel_correction_shift = EF_IMU_DEFAULT_ACCEL_SHIFT;
        state->config.euler_lowpass_shift = EF_IMU_DEFAULT_EULER_LPF_SHIFT;
    }
    state->config.accel_correction_shift = ef_imu_clamp_shift(state->config.accel_correction_shift);
    state->config.euler_lowpass_shift = ef_imu_clamp_shift(state->config.euler_lowpass_shift);
    state->valid = false;
}

bool ef_imu_attitude_update(ef_imu_attitude_t *state,
    const int64_t gyro_udps[3],
    const int32_t accel_ug[3],
    uint32_t dt_us)
{
    int32_t accel_norm_q15[3];
    int32_t gravity_q15[3];
    int32_t half_delta_q15[3];

    if ((state == NULL) || (gyro_udps == NULL) || (accel_ug == NULL) || (dt_us == 0U)) {
        return false;
    }

    for (uint8_t i = 0U; i < 3U; i++) {
        half_delta_q15[i] = (int32_t) (((gyro_udps[i] * (int64_t) dt_us) * EF_IMU_HALF_RAD_Q15_PER_DEG) /
            1000000000000LL);
    }

    if (ef_imu_normalize_vector_q15(accel_ug, accel_norm_q15)) {
        int32_t correction_q15[3];

        ef_imu_estimate_gravity_q15(state, gravity_q15);
        correction_q15[0] = ef_imu_q15_mul(accel_norm_q15[1], gravity_q15[2]) -
            ef_imu_q15_mul(accel_norm_q15[2], gravity_q15[1]);
        correction_q15[1] = ef_imu_q15_mul(accel_norm_q15[2], gravity_q15[0]) -
            ef_imu_q15_mul(accel_norm_q15[0], gravity_q15[2]);
        correction_q15[2] = ef_imu_q15_mul(accel_norm_q15[0], gravity_q15[1]) -
            ef_imu_q15_mul(accel_norm_q15[1], gravity_q15[0]);
        half_delta_q15[0] += correction_q15[0] >> state->config.accel_correction_shift;
        half_delta_q15[1] += correction_q15[1] >> state->config.accel_correction_shift;
    }

    ef_imu_apply_quat_delta(state, half_delta_q15);
    ef_imu_update_euler_from_quat(state);
    state->valid = true;
    return true;
}
