/**
  ******************************************************************************
  * @file    bsp_wdg.c
  * @brief   Independent watchdog helpers.
  ******************************************************************************
  */

#include "bsp_wdg.h"
#include "iwdg.h"

static uint8_t s_caused_reset = 0U;
static uint32_t s_feed_count = 0U;

void bsp_wdg_init(void)
{
  s_caused_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != 0U) ? 1U : 0U;

  __HAL_RCC_CLEAR_RESET_FLAGS();

  s_feed_count = 0U;
}

void bsp_wdg_feed(void)
{
  (void)HAL_IWDG_Refresh(&hiwdg);
  s_feed_count++;
}

uint8_t bsp_wdg_caused_reset(void)
{
  return s_caused_reset;
}

uint32_t bsp_wdg_get_feed_count(void)
{
  return s_feed_count;
}
