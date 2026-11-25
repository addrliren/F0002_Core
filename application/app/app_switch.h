/**
  ******************************************************************************
  * @file    app_switch.h
  * @brief   SWITCH_EN routing control for the USB-UART bridge.
  *
  *          SWITCH_EN high (default) : USB-UART <-> MCU (USART1)
  *          SWITCH_EN low            : USB-UART <-> HMI screen, and the screen
  *                                     input moves from UART4 to USART1.
  *
  *          The transition is one way: after SWITCH_EN is driven low the MCU no
  *          longer receives anything from the PC, therefore the whole
  *          application is stopped and only the watchdog keeps running.
  ******************************************************************************
  */

#ifndef __APP_SWITCH_H
#define __APP_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* Reply sent to the PC before the switch is performed. */
#define APP_SWITCH_ACK          "#3 OK\r\n"

/* Keep the 300 ms LED heartbeat alive after the switch so the box still shows
   that it runs. It is driven from app_loop() because the scheduler is stopped.
   Set to 0 for a strict "watchdog only" behaviour. */
#ifndef APP_SWITCH_KEEP_LED
#define APP_SWITCH_KEEP_LED     1
#endif

void app_switch_init(void);

/* Consumes the USART1 receive buffer and looks for the switch command.
   Call it from the scheduler. */
void app_switch_poll(void);

/* Performs the hand shake and the switch. Returns BSP_OK once SWITCH_EN is low. */
uint8_t app_switch_execute(void);

/* 1 after the switch, the application is stopped. */
uint8_t app_switch_is_hmi_mode(void);

/* Number of received bytes that were examined (diagnostics). */
uint32_t app_switch_get_byte_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SWITCH_H */
