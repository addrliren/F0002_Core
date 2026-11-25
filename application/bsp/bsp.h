/**
  ******************************************************************************
  * @file    bsp.h
  * @brief   Aggregated BSP interface. Include this from the application layer.
  ******************************************************************************
  */

#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"
#include "bsp_dwt.h"
#include "bsp_gpio.h"
#include "bsp_wdg.h"
#include "bsp_uart.h"
#include "bsp_key.h"
#include "bsp_soft_i2c.h"
#include "bsp_eeprom.h"
#include "bsp_spi_flash.h"
#include "bsp_ds18b20.h"
#include "bsp_adc.h"

/**
  * @brief  Initialise every board driver.
  * @note   MX_GPIO_Init(), MX_USART1_UART_Init(), MX_UART4_Init(), MX_SPI2_Init(),
  *         MX_ADC1_Init() and MX_IWDG_Init() must already have run.
  */
void bsp_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
