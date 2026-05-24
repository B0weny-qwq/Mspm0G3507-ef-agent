#include "ef_drivers.h"

void ef_drivers_init(void)
{
    ef_spi_init();
    ef_pwm_init();
}

void ef_drivers_service(void)
{
    ef_spi_poll(EF_SPI_BOARD);
}
