/**
  ******************************************************************************
  * @file    bsp_ds18b20.h
  * @brief   DS18B20 board temperature sensor on TEMP_BOARD (PC6), 1-Wire.
  ******************************************************************************
  */

#ifndef __BSP_DS18B20_H
#define __BSP_DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

#define BSP_DS18B20_ROM_SKIP    0xCCU
#define BSP_DS18B20_CONVERT     0x44U
#define BSP_DS18B20_READ_SCRATCH 0xBEU

void bsp_ds18b20_init(void);

uint8_t bsp_ds18b20_reset(void);
uint8_t bsp_ds18b20_is_present(void);

/* Starts a conversion; the result is ready after ~750 ms at 12 bit. */
uint8_t bsp_ds18b20_start_conversion(void);

/* True once the previous conversion has finished. */
uint8_t bsp_ds18b20_conversion_done(void);

/* Reads the scratchpad and converts it to 1/16 degC steps. */
uint8_t bsp_ds18b20_read_raw(int16_t *raw);
uint8_t bsp_ds18b20_read_celsius(float *celsius);

/* Blocking helper: start conversion, wait, read temperature. */
uint8_t bsp_ds18b20_read_blocking(float *celsius, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DS18B20_H */
