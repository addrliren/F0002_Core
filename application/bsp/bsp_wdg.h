/**
  ******************************************************************************
  * @file    bsp_wdg.h
  * @brief   Independent watchdog helpers.
  *
  *          IWDG is started by MX_IWDG_Init() (LSI ~40 kHz, prescaler 8,
  *          reload 4095 -> about 0.82 s timeout). Anything that takes longer
  *          than ~400 ms between two feeds is a design error.
  ******************************************************************************
  */

#ifndef __BSP_WDG_H
#define __BSP_WDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

#define BSP_WDG_TIMEOUT_MS      820U
#define BSP_WDG_FEED_PERIOD_MS  100U

/* Clears the reset flags, records whether the last reset was a watchdog one. */
void bsp_wdg_init(void);

/* Reload the counter. Must be called from the main loop context only. */
void bsp_wdg_feed(void);

/* 1 when the previous boot was caused by the independent watchdog. */
uint8_t bsp_wdg_caused_reset(void);

uint32_t bsp_wdg_get_feed_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_WDG_H */
