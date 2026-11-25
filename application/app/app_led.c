/**
  ******************************************************************************
  * @file    app_led.c
  * @brief   300 ms LED1 run indicator.
  ******************************************************************************
  */

#include "app_led.h"

#define APP_LED_TASK_PERIOD_MS  10U

static uint8_t s_on = 0U;
static uint16_t s_elapsed_ms = 0U;
static uint8_t s_stopped = 0U;

void app_led_init(void)
{
  s_on = 0U;
  s_elapsed_ms = 0U;
  s_stopped = 0U;

  bsp_led1_on();
  s_on = 1U;
}

void app_led_task(void)
{
  if (s_stopped != 0U)
  {
    return;
  }

  s_elapsed_ms = (uint16_t)(s_elapsed_ms + APP_LED_TASK_PERIOD_MS);

  if ((s_on != 0U) && (s_elapsed_ms >= APP_LED_ON_MS))
  {
    s_elapsed_ms = 0U;
    s_on = 0U;
    bsp_led1_off();
  }
  else if ((s_on == 0U) && (s_elapsed_ms >= APP_LED_OFF_MS))
  {
    s_elapsed_ms = 0U;
    s_on = 1U;
    bsp_led1_on();
  }
  else
  {
    /* keep waiting */
  }
}

void app_led_stop(void)
{
  s_stopped = 1U;
  s_on = 1U;
  s_elapsed_ms = 0U;
  bsp_led1_on();
}

uint8_t app_led_is_on(void)
{
  return s_on;
}
