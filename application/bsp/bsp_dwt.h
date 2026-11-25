/**
  ******************************************************************************
  * @file    bsp_dwt.h
  * @brief   DWT (Data Watchpoint and Trace) cycle counter based delay.
  *          Provides a high precision microsecond time base for 1-Wire,
  *          software I2C and logging, independent of the HAL tick.
  ******************************************************************************
  */

#ifndef __BSP_DWT_H
#define __BSP_DWT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

/**
  * @brief  Start the DWT cycle counter. Call once after SystemClock_Config().
  * @retval None
  */
void bsp_dwt_init(void);

/**
  * @brief  Cyclic counter availability.
  * @retval 1 if DWT is running, 0 if the fallback busy loop is used
  */
uint8_t bsp_dwt_is_ready(void);

/**
  * @brief  Raw cycle counter value.
  */
uint32_t bsp_dwt_get_cycles(void);

/**
  * @brief  Microsecond time base since boot (wraps every ~71 minutes).
  */
uint32_t bsp_dwt_get_us(void);

/**
  * @brief  Millisecond time base since boot (wraps every ~49 days).
  */
uint32_t bsp_dwt_get_ms(void);

/**
  * @brief  Busy wait delay, interrupt safe (no interrupt is masked).
  * @param  us microseconds, 0 returns immediately
  */
void bsp_dwt_delay_us(uint32_t us);

/**
  * @brief  Busy wait delay in milliseconds.
  */
void bsp_dwt_delay_ms(uint32_t ms);

#define BSP_DELAY_US(us)    bsp_dwt_delay_us(us)
#define BSP_DELAY_MS(ms)    bsp_dwt_delay_ms(ms)

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DWT_H */
