/**
  ******************************************************************************
  * @file    bsp_gpio.h
  * @brief   Board level GPIO controls (rails, LED, buzzer, UART switch, ...).
  *
  *  Hardware notes (see main.h):
  *    LED1_EN      : low  = LED on
  *    BUZZER1_EN   : high = buzzer on, default low
  *    SWITCH_EN    : high = USB-UART <-> MCU (default), low = USB-UART <-> HMI
  *    OUT_TFT_EN   : high = HMI screen powered (default), low = screen power off
  *    OUT_SDCARD_EN: high = TF card powered
  *    IN_SD_STA    : low  = TF card inserted
  *    OUT_ADJ_EN1  : low  = ADJ input power enabled
  *    OUT_ADJ_EN2  : high = ADJ output enabled
  *    KEY_x        : external pull-up, low = pressed
  ******************************************************************************
  */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

/* Initialise the board to the documented safe default state. */
void bsp_gpio_init(void);

/* LED -----------------------------------------------------------------------*/
void bsp_led1_on(void);
void bsp_led1_off(void);
void bsp_led1_set(uint8_t on);
void bsp_led1_toggle(void);
uint8_t bsp_led1_get(void);

/* Buzzer --------------------------------------------------------------------*/
void bsp_buzzer_on(void);
void bsp_buzzer_off(void);
/* Non blocking single beep, service with bsp_buzzer_poll() every 10 ms. */
void bsp_buzzer_beep(uint16_t ms);
void bsp_buzzer_poll(void);
uint8_t bsp_buzzer_is_on(void);

/* USB-UART <-> MCU / HMI switch ---------------------------------------------*/
/* WARNING: once switched to the HMI the MCU cannot switch it back. */
void bsp_switch_to_mcu(void);     /* SWITCH_EN = 1 */
void bsp_switch_to_hmi(void);     /* SWITCH_EN = 0 */
uint8_t bsp_switch_is_hmi(void);

/* HMI screen supply ---------------------------------------------------------*/
void bsp_tft_power(uint8_t on);

/* TF card -------------------------------------------------------------------*/
void bsp_sdcard_power(uint8_t on);
uint8_t bsp_sdcard_inserted(void);    /* 1 = card present */
void bsp_sdcard_update(void);         /* power follows the detect input */

/* Power rails ---------------------------------------------------------------*/
void bsp_rail_12v(uint8_t on);
void bsp_rail_24v(uint8_t on);
void bsp_rail_5v0(uint8_t on);
void bsp_rail_3v3(uint8_t on);
void bsp_rail_fan(uint8_t on);
void bsp_sd_ch_enable(uint8_t on);

/* ADJ module (voltage / current source) -------------------------------------*/
void bsp_adj_input_enable(uint8_t on);    /* EN1, active low  */
void bsp_adj_output_enable(uint8_t on);   /* EN2, active high */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H */
