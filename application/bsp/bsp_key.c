/**
  ******************************************************************************
  * @file    bsp_key.c
  * @brief   Key scan with debounce and long press detection.
  ******************************************************************************
  */

#include "bsp_key.h"

#define KEY_DEBOUNCE_COUNT      2U      /* 2 * 10 ms */

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t      pin;
  uint8_t       stable;                 /* 0 released, 1 pressed */
  uint8_t       counter;
  uint32_t      press_ms;
  uint8_t       long_reported;
} bsp_key_dev_t;

static bsp_key_dev_t s_keys[BSP_KEY_NUM] =
{
  { KEY_A_GPIO_Port, KEY_A_Pin, 0U, 0U, 0U, 0U },
  { KEY_B_GPIO_Port, KEY_B_Pin, 0U, 0U, 0U, 0U },
  { KEY_C_GPIO_Port, KEY_C_Pin, 0U, 0U, 0U, 0U },
  { KEY_D_GPIO_Port, KEY_D_Pin, 0U, 0U, 0U, 0U },
};

static bsp_key_callback_t s_callback;

static uint8_t key_raw_pressed(const bsp_key_dev_t *dev)
{
  return (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

void bsp_key_init(void)
{
  uint8_t i;

  for (i = 0U; i < (uint8_t)BSP_KEY_NUM; i++)
  {
    s_keys[i].stable        = key_raw_pressed(&s_keys[i]);
    s_keys[i].counter       = 0U;
    s_keys[i].press_ms      = 0U;
    s_keys[i].long_reported = 0U;
  }
}

void bsp_key_scan(void)
{
  uint8_t  i;
  uint32_t now = HAL_GetTick();

  for (i = 0U; i < (uint8_t)BSP_KEY_NUM; i++)
  {
    bsp_key_dev_t *dev = &s_keys[i];
    uint8_t raw = key_raw_pressed(dev);

    if (raw != dev->stable)
    {
      if (dev->counter < KEY_DEBOUNCE_COUNT)
      {
        dev->counter++;
      }

      if (dev->counter >= KEY_DEBOUNCE_COUNT)
      {
        dev->stable  = raw;
        dev->counter = 0U;

        if (raw != 0U)
        {
          dev->press_ms      = now;
          dev->long_reported = 0U;

          if (s_callback != 0)
          {
            s_callback((bsp_key_id_t)i, BSP_KEY_EVT_DOWN);
          }
        }
        else
        {
          if (s_callback != 0)
          {
            s_callback((bsp_key_id_t)i, BSP_KEY_EVT_UP);
          }
        }
      }
    }
    else
    {
      dev->counter = 0U;

      if ((dev->stable != 0U) && (dev->long_reported == 0U))
      {
        if ((uint32_t)(now - dev->press_ms) >= BSP_KEY_LONG_PRESS_MS)
        {
          dev->long_reported = 1U;

          if (s_callback != 0)
          {
            s_callback((bsp_key_id_t)i, BSP_KEY_EVT_LONG);
          }
        }
      }
    }
  }
}

uint8_t bsp_key_is_down(bsp_key_id_t id)
{
  if (id >= BSP_KEY_NUM)
  {
    return 0U;
  }

  return s_keys[id].stable;
}

void bsp_key_set_callback(bsp_key_callback_t cb)
{
  s_callback = cb;
}
