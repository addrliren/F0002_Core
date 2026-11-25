/**
  ******************************************************************************
  * @file    app_led.h
  * @brief   300 ms run indicator on LED1 (active low).
  ******************************************************************************
  */

#ifndef __APP_LED_H
#define __APP_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* On for 300 ms, off for 300 ms: one visible blink every 600 ms. */
#ifndef APP_LED_ON_MS
#define APP_LED_ON_MS       300U
#endif

#ifndef APP_LED_OFF_MS
#define APP_LED_OFF_MS      300U
#endif

/* Number of blinks reported before the LED is latched on (0 = keep blinking). */
#ifndef APP_LED_STARTUP_BLINKS
#define APP_LED_STARTUP_BLINKS  0U
#endif

void app_led_init(void);

/* Call every 10 ms. */
void app_led_task(void);

/* Freeze the pattern: the LED is latched on. */
void app_led_stop(void);

uint8_t app_led_is_on(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LED_H */
