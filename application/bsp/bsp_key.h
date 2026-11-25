/**
  ******************************************************************************
  * @file    bsp_key.h
  * @brief   Four on board keys (KEY_A..KEY_D), external pull-ups, active low.
  ******************************************************************************
  */

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

typedef enum
{
  BSP_KEY_A = 0,
  BSP_KEY_B,
  BSP_KEY_C,
  BSP_KEY_D,
  BSP_KEY_NUM
} bsp_key_id_t;

typedef enum
{
  BSP_KEY_EVT_NONE = 0,
  BSP_KEY_EVT_DOWN,
  BSP_KEY_EVT_UP,
  BSP_KEY_EVT_LONG
} bsp_key_event_t;

typedef void (*bsp_key_callback_t)(bsp_key_id_t id, bsp_key_event_t event);

#define BSP_KEY_LONG_PRESS_MS   1500U

void bsp_key_init(void);

/* Call every 10 ms. */
void bsp_key_scan(void);

uint8_t bsp_key_is_down(bsp_key_id_t id);
void bsp_key_set_callback(bsp_key_callback_t cb);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_KEY_H */
