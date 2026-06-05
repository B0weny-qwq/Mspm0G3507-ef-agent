#include "ef_i2c.h"

#include "ef_platform.h"
#include "ti_msp_dl_config.h"

/* MSPM0 I2C 驱动实现：当前提供阻塞式写、读和先写后读事务。 */

#define EF_I2C_MAX_TRANSFER_LEN 0x0FFFU

/* 根据逻辑总线编号返回底层 I2C 实例。 */
static I2C_Regs *ef_i2c_inst(ef_i2c_id_t id)
{
    switch (id) {
    case EF_I2C_TOF:
        return I2C_TOF_INST;
    default:
        return NULL;
    }
}

/* 判断当前事务是否超时。 */
static bool ef_i2c_timed_out(uint32_t start_ms, uint32_t timeout_ms)
{
    return (timeout_ms != 0U) && ((uint32_t) (ef_platform_millis() - start_ms) >= timeout_ms);
}

/* 等待控制器进入空闲状态。 */
static bool ef_i2c_wait_idle(I2C_Regs *i2c, uint32_t timeout_ms)
{
    const uint32_t start_ms = ef_platform_millis();

    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (ef_i2c_timed_out(start_ms, timeout_ms)) {
            return false;
        }
    }

    return true;
}

/* 等待控制器不再忙，并确认没有错误标志。 */
static bool ef_i2c_wait_not_busy(I2C_Regs *i2c, uint32_t timeout_ms)
{
    const uint32_t start_ms = ef_platform_millis();

    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY) != 0U) {
        if (ef_i2c_timed_out(start_ms, timeout_ms)) {
            return false;
        }
    }

    return ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U);
}

/* 执行一次底层阻塞写事务。 */
static bool ef_i2c_write_impl(I2C_Regs *i2c, uint8_t address_7bit, const uint8_t *data,
    size_t len, uint32_t timeout_ms)
{
    uint32_t sent = 0U;
    const uint32_t start_ms = ef_platform_millis();

    if ((data == NULL) || (len == 0U) || (len > EF_I2C_MAX_TRANSFER_LEN)) {
        return false;
    }

    if (!ef_i2c_wait_idle(i2c, timeout_ms)) {
        return false;
    }

    DL_I2C_flushControllerTXFIFO(i2c);
    DL_I2C_flushControllerRXFIFO(i2c);

    sent = DL_I2C_fillControllerTXFIFO(i2c, data, (uint16_t) len);
    DL_I2C_startControllerTransferAdvanced(i2c, address_7bit, DL_I2C_CONTROLLER_DIRECTION_TX,
        (uint16_t) len, DL_I2C_CONTROLLER_START_ENABLE,
        DL_I2C_CONTROLLER_STOP_ENABLE, DL_I2C_CONTROLLER_ACK_DISABLE);

    while (sent < len) {
        const uint32_t status = DL_I2C_getControllerStatus(i2c);

        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            return false;
        }
        if (ef_i2c_timed_out(start_ms, timeout_ms)) {
            return false;
        }
        if (!DL_I2C_isControllerTXFIFOFull(i2c)) {
            DL_I2C_transmitControllerData(i2c, data[sent]);
            sent++;
        }
    }

    return ef_i2c_wait_not_busy(i2c, timeout_ms);
}

/* 对外暴露的阻塞写接口。 */
bool ef_i2c_write(ef_i2c_id_t id, uint8_t address_7bit, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    I2C_Regs *const i2c = ef_i2c_inst(id);

    if (i2c == NULL) {
        return false;
    }

    return ef_i2c_write_impl(i2c, address_7bit, data, len, timeout_ms);
}

/* 对外暴露的阻塞读接口。 */
bool ef_i2c_read(ef_i2c_id_t id, uint8_t address_7bit, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    I2C_Regs *const i2c = ef_i2c_inst(id);
    const uint32_t start_ms = ef_platform_millis();
    size_t received = 0U;

    if ((i2c == NULL) || (data == NULL) || (len == 0U) || (len > EF_I2C_MAX_TRANSFER_LEN)) {
        return false;
    }

    if (!ef_i2c_wait_idle(i2c, timeout_ms)) {
        return false;
    }

    DL_I2C_flushControllerTXFIFO(i2c);
    DL_I2C_flushControllerRXFIFO(i2c);
    DL_I2C_startControllerTransfer(i2c, address_7bit, DL_I2C_CONTROLLER_DIRECTION_RX, (uint16_t) len);

    while (received < len) {
        const uint32_t status = DL_I2C_getControllerStatus(i2c);

        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            return false;
        }
        if (ef_i2c_timed_out(start_ms, timeout_ms)) {
            return false;
        }
        if (!DL_I2C_isControllerRXFIFOEmpty(i2c)) {
            data[received] = DL_I2C_receiveControllerData(i2c);
            received++;
        }
    }

    return ef_i2c_wait_not_busy(i2c, timeout_ms);
}

/* 先写寄存器地址，再读取返回数据。 */
bool ef_i2c_write_read(ef_i2c_id_t id, uint8_t address_7bit,
    const uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len, uint32_t timeout_ms)
{
    I2C_Regs *const i2c = ef_i2c_inst(id);

    if (i2c == NULL) {
        return false;
    }

    if ((tx != NULL) && (tx_len > 0U) && !ef_i2c_write_impl(i2c, address_7bit, tx, tx_len, timeout_ms)) {
        return false;
    }

    return ef_i2c_read(id, address_7bit, rx, rx_len, timeout_ms);
}
