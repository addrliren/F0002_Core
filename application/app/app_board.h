/**
  ******************************************************************************
  * @file    app_board.h
  * @brief   Periodic board supervision: keys, buzzer, TF card, ADC, DS18B20.
  ******************************************************************************
  */

#ifndef __APP_BOARD_H
#define __APP_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

typedef struct
{
  float   vdda;             /* volt  */
  float   adj_voltage;      /* volt  */
  float   adj_current;      /* mA    */
  float   mcu_temperature;  /* degC  */
  float   board_temperature;/* degC  */
  uint8_t adj_valid;
  uint8_t board_temperature_valid;
  uint8_t sdcard_present;
  uint8_t eeprom_present;
  uint8_t flash_present;
  uint8_t ds18b20_present;
  uint32_t key_press_count;
} app_board_status_t;

void app_board_init(void);

/* 10 ms: keys and buzzer state machine. */
void app_board_task_fast(void);

/* 100 ms: TF card detect, ADJ voltage / current. */
void app_board_task_medium(void);

/* 500 ms: VDDA, MCU temperature and the non blocking DS18B20 conversion. */
void app_board_task_slow(void);

const app_board_status_t *app_board_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_BOARD_H */
