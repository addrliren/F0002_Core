/**
  ******************************************************************************
  * @file    bsp.c
  * @brief   Board support initialisation sequence.
  ******************************************************************************
  */

#include "bsp.h"

void bsp_init(void)
{
  /* The time base must exist before any driver that waits on a pin. */
  bsp_dwt_init();

  bsp_wdg_init();

  bsp_gpio_init();
  bsp_wdg_feed();

  bsp_uart_init();

  bsp_key_init();

  (void)bsp_eeprom_init();
  bsp_wdg_feed();

  (void)bsp_flash_init();
  bsp_wdg_feed();

  bsp_ds18b20_init();

  (void)bsp_adc_init();

  bsp_wdg_feed();
}
