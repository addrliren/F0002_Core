/**
  ******************************************************************************
  * @file    bsp_gpio.c
  * @brief   Board level GPIO controls.
  ******************************************************************************
  */

#include "bsp_gpio.h"

#define BSP_LED_ON          GPIO_PIN_RESET
#define BSP_LED_OFF         GPIO_PIN_SET

static uint32_t s_beep_off_ms;

void bsp_gpio_init(void)
{
  /* Safe defaults, matches the CubeMX generated pin levels. */
  HAL_GPIO_WritePin(LED1_EN_GPIO_Port, LED1_EN_Pin, BSP_LED_OFF);
  HAL_GPIO_WritePin(BUZZER1_EN_GPIO_Port, BUZZER1_EN_Pin, GPIO_PIN_RESET);

  /* USB-UART routed to the MCU, HMI screen powered. */
  HAL_GPIO_WritePin(SWITCH_EN_GPIO_Port, SWITCH_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OUT_TFT_EN_GPIO_Port, OUT_TFT_EN_Pin, GPIO_PIN_SET);

  /* ADJ module disabled. */
  HAL_GPIO_WritePin(OUT_ADJ_EN1_GPIO_Port, OUT_ADJ_EN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OUT_ADJ_EN2_GPIO_Port, OUT_ADJ_EN2_Pin, GPIO_PIN_RESET);

  /* TF card supply follows the card detect input. */
  bsp_sdcard_update();

  s_beep_off_ms = 0U;
}

/* LED ----------------------------------------------------------------------*/
void bsp_led1_on(void)
{
  HAL_GPIO_WritePin(LED1_EN_GPIO_Port, LED1_EN_Pin, BSP_LED_ON);
}

void bsp_led1_off(void)
{
  HAL_GPIO_WritePin(LED1_EN_GPIO_Port, LED1_EN_Pin, BSP_LED_OFF);
}

void bsp_led1_set(uint8_t on)
{
  if (on != 0U)
  {
    bsp_led1_on();
  }
  else
  {
    bsp_led1_off();
  }
}

void bsp_led1_toggle(void)
{
  if (HAL_GPIO_ReadPin(LED1_EN_GPIO_Port, LED1_EN_Pin) == BSP_LED_ON)
  {
    bsp_led1_off();
  }
  else
  {
    bsp_led1_on();
  }
}

uint8_t bsp_led1_get(void)
{
  return (HAL_GPIO_ReadPin(LED1_EN_GPIO_Port, LED1_EN_Pin) == BSP_LED_ON) ? 1U : 0U;
}

/* Buzzer -------------------------------------------------------------------*/
void bsp_buzzer_on(void)
{
  s_beep_off_ms = 0U;
  HAL_GPIO_WritePin(BUZZER1_EN_GPIO_Port, BUZZER1_EN_Pin, GPIO_PIN_SET);
}

void bsp_buzzer_off(void)
{
  s_beep_off_ms = 0U;
  HAL_GPIO_WritePin(BUZZER1_EN_GPIO_Port, BUZZER1_EN_Pin, GPIO_PIN_RESET);
}

void bsp_buzzer_beep(uint16_t ms)
{
  if (ms == 0U)
  {
    bsp_buzzer_off();
    return;
  }

  HAL_GPIO_WritePin(BUZZER1_EN_GPIO_Port, BUZZER1_EN_Pin, GPIO_PIN_SET);
  s_beep_off_ms = HAL_GetTick() + ms;
}

void bsp_buzzer_poll(void)
{
  if (s_beep_off_ms == 0U)
  {
    return;
  }

  if ((int32_t)(HAL_GetTick() - s_beep_off_ms) >= 0)
  {
    bsp_buzzer_off();
  }
}

uint8_t bsp_buzzer_is_on(void)
{
  return (HAL_GPIO_ReadPin(BUZZER1_EN_GPIO_Port, BUZZER1_EN_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

/* USB-UART switch ----------------------------------------------------------*/
void bsp_switch_to_mcu(void)
{
  HAL_GPIO_WritePin(SWITCH_EN_GPIO_Port, SWITCH_EN_Pin, GPIO_PIN_SET);
}

void bsp_switch_to_hmi(void)
{
  HAL_GPIO_WritePin(SWITCH_EN_GPIO_Port, SWITCH_EN_Pin, GPIO_PIN_RESET);
}

uint8_t bsp_switch_is_hmi(void)
{
  return (HAL_GPIO_ReadPin(SWITCH_EN_GPIO_Port, SWITCH_EN_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

/* HMI screen supply --------------------------------------------------------*/
void bsp_tft_power(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_TFT_EN_GPIO_Port, OUT_TFT_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* TF card ------------------------------------------------------------------*/
void bsp_sdcard_power(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_SDCARD_EN_GPIO_Port, OUT_SDCARD_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t bsp_sdcard_inserted(void)
{
  return (HAL_GPIO_ReadPin(IN_SD_STA_GPIO_Port, IN_SD_STA_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

void bsp_sdcard_update(void)
{
  bsp_sdcard_power(bsp_sdcard_inserted());
}

/* Power rails --------------------------------------------------------------*/
void bsp_rail_12v(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_12V_EN_GPIO_Port, OUT_12V_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void bsp_rail_24v(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_24V_EN_GPIO_Port, OUT_24V_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void bsp_rail_5v0(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_5V0_EN_GPIO_Port, OUT_5V0_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void bsp_rail_3v3(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_3V3_EN_GPIO_Port, OUT_3V3_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void bsp_rail_fan(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_FAN_EN_GPIO_Port, OUT_FAN_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void bsp_sd_ch_enable(uint8_t on)
{
  HAL_GPIO_WritePin(OUT_SD_CH_EN_GPIO_Port, OUT_SD_CH_EN_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ADJ module ---------------------------------------------------------------*/
void bsp_adj_input_enable(uint8_t on)
{
  /* EN1 is active low. */
  HAL_GPIO_WritePin(OUT_ADJ_EN1_GPIO_Port, OUT_ADJ_EN1_Pin,
                    (on != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void bsp_adj_output_enable(uint8_t on)
{
  /* EN2 is active high. */
  HAL_GPIO_WritePin(OUT_ADJ_EN2_GPIO_Port, OUT_ADJ_EN2_Pin,
                    (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
