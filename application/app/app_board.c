/**
  ******************************************************************************
  * @file    app_board.c
  * @brief   Periodic board supervision.
  ******************************************************************************
  */

#include "app_board.h"
#include "app_log.h"

#define BOARD_DS18B20_CONV_MS   800U
#define BOARD_STATUS_LOG_MS     10000U

static app_board_status_t s_status;
static uint8_t s_conv_pending = 0U;
static uint32_t s_conv_start_ms = 0U;
static uint32_t s_status_timer_ms = 0U;

static char board_key_letter(bsp_key_id_t id)
{
  switch (id)
  {
    case BSP_KEY_A: return 'A';
    case BSP_KEY_B: return 'B';
    case BSP_KEY_C: return 'C';
    case BSP_KEY_D: return 'D';
    default:        return '?';
  }
}

static void board_key_callback(bsp_key_id_t id, bsp_key_event_t event)
{
  if (event == BSP_KEY_EVT_DOWN)
  {
    s_status.key_press_count++;
    bsp_buzzer_beep(40U);
    APP_LOGI("KEY", "key %c pressed", board_key_letter(id));
  }
  else if (event == BSP_KEY_EVT_LONG)
  {
    bsp_buzzer_beep(120U);
    APP_LOGI("KEY", "key %c long press", board_key_letter(id));
  }
  else
  {
    /* release, nothing to do */
  }
}

void app_board_init(void)
{
  s_status.vdda                    = 0.0f;
  s_status.adj_voltage             = 0.0f;
  s_status.adj_current             = 0.0f;
  s_status.mcu_temperature         = 0.0f;
  s_status.board_temperature       = 0.0f;
  s_status.adj_valid               = 0U;
  s_status.board_temperature_valid = 0U;
  s_status.sdcard_present          = 0U;
  s_status.eeprom_present          = bsp_eeprom_is_present();
  s_status.flash_present           = bsp_flash_is_present();
  s_status.ds18b20_present         = bsp_ds18b20_is_present();
  s_status.key_press_count         = 0U;

  s_conv_pending = 0U;
  s_status_timer_ms = HAL_GetTick();

  bsp_key_set_callback(board_key_callback);

  /* Power the TF card only when one is inserted. */
  bsp_sdcard_update();
  s_status.sdcard_present = bsp_sdcard_inserted();

  APP_LOGI("BRD", "eeprom=%u flash=%u", (unsigned)s_status.eeprom_present,
           (unsigned)s_status.flash_present);
}

void app_board_task_fast(void)
{
  bsp_key_scan();
  bsp_buzzer_poll();
}

void app_board_task_medium(void)
{
  float value = 0.0f;

  bsp_sdcard_update();
  s_status.sdcard_present = bsp_sdcard_inserted();

  if (bsp_adc_read_adj_voltage(&value) == BSP_OK)
  {
    s_status.adj_voltage = value;
    s_status.adj_valid = 1U;
  }
  else
  {
    s_status.adj_valid = 0U;
  }

  if (bsp_adc_read_adj_current(&value) == BSP_OK)
  {
    s_status.adj_current = value;
  }
}

void app_board_task_slow(void)
{
  uint32_t now = HAL_GetTick();
  float value = 0.0f;

  bsp_adc_update_vdda();
  s_status.vdda = bsp_adc_get_vdda();

  if (bsp_adc_read_mcu_temperature(&value) == BSP_OK)
  {
    s_status.mcu_temperature = value;
  }

  /* Non blocking DS18B20 sequence: start, then read 800 ms later. */
  if (s_conv_pending == 0U)
  {
    if (bsp_ds18b20_start_conversion() == BSP_OK)
    {
      s_conv_pending = 1U;
      s_conv_start_ms = now;
      s_status.ds18b20_present = 1U;
    }
    else
    {
      s_status.ds18b20_present = 0U;
      s_status.board_temperature_valid = 0U;
    }
  }
  else if ((uint32_t)(now - s_conv_start_ms) >= BOARD_DS18B20_CONV_MS)
  {
    s_conv_pending = 0U;

    if (bsp_ds18b20_read_celsius(&value) == BSP_OK)
    {
      s_status.board_temperature = value;
      s_status.board_temperature_valid = 1U;
    }
    else
    {
      s_status.board_temperature_valid = 0U;
    }
  }
  else
  {
    /* conversion still running */
  }

  if ((uint32_t)(now - s_status_timer_ms) >= BOARD_STATUS_LOG_MS)
  {
    s_status_timer_ms = now;

    APP_LOGI("BRD", "vdda=%umV adj=%umV/%umA mcu=%dC board=%dC sd=%u",
             (unsigned)(s_status.vdda * 1000.0f),
             (unsigned)(s_status.adj_voltage * 1000.0f),
             (unsigned)s_status.adj_current,
             (int)s_status.mcu_temperature,
             (int)s_status.board_temperature,
             (unsigned)s_status.sdcard_present);
  }
}

const app_board_status_t *app_board_get_status(void)
{
  return &s_status;
}
