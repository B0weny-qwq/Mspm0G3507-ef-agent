#include "ef_drivers.h"

/* 驱动层统一入口：集中初始化和轮询需要前台服务的驱动模块。 */

/* 初始化当前工程启用的驱动模块。 */
void ef_drivers_init(void)
{
    ef_spi_init();
    ef_pwm_init();
}

/* 执行驱动层周期服务。 */
void ef_drivers_service(void)
{
    ef_spi_poll(EF_SPI_BOARD);
}
