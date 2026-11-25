/**
  ******************************************************************************
  * @file    app_switch.c
  * @brief   USB-UART / HMI routing switch handling.
  ******************************************************************************
  */

#include "app_switch.h"
#include "app_sched.h"
#include "app_led.h"
#include <string.h>

/*
 * Command sent by the PC:
 *   \x00\xFF\xFF\xFFSystem.comm_string.txt="#3 S1\n"\xFF\xFF\xFF
 *
 * Both the escaped newline (0x0A) and the literal two characters "\" "n" are
 * accepted so the trigger cannot be lost because of a terminal setting.
 */
static const uint8_t s_cmd_lf[] =
  "\x00\xFF\xFF\xFF"
  "System.comm_string.txt=\"#3 S1\n\""
  "\xFF\xFF\xFF";

static const uint8_t s_cmd_text[] =
  "\x00\xFF\xFF\xFF"
  "System.comm_string.txt=\"#3 S1\\n\""
  "\xFF\xFF\xFF";

#define APP_SWITCH_HIST_SIZE    48U
#define APP_SWITCH_ACK_LEN      (sizeof(APP_SWITCH_ACK) - 1U)

typedef char app_switch_hist_check_t
  [((sizeof(s_cmd_lf) - 1U) <= APP_SWITCH_HIST_SIZE) &&
   ((sizeof(s_cmd_text) - 1U) <= APP_SWITCH_HIST_SIZE) ? 1 : -1];

static uint8_t s_hist[APP_SWITCH_HIST_SIZE];
static uint16_t s_hist_pos = 0U;
static uint16_t s_hist_len = 0U;
static uint8_t s_hmi_mode = 0U;
static uint32_t s_byte_count = 0U;

void app_switch_init(void)
{
  memset(s_hist, 0, sizeof(s_hist));
  s_hist_pos = 0U;
  s_hist_len = 0U;
  s_hmi_mode = 0U;
  s_byte_count = 0U;
}

uint8_t app_switch_is_hmi_mode(void)
{
  return s_hmi_mode;
}

uint32_t app_switch_get_byte_count(void)
{
  return s_byte_count;
}

static void switch_hist_push(uint8_t byte)
{
  s_hist[s_hist_pos] = byte;
  s_hist_pos++;

  if (s_hist_pos >= APP_SWITCH_HIST_SIZE)
  {
    s_hist_pos = 0U;
  }

  if (s_hist_len < APP_SWITCH_HIST_SIZE)
  {
    s_hist_len++;
  }
}

static uint8_t switch_hist_matches(const uint8_t *pattern, uint16_t len)
{
  uint16_t start;
  uint16_t i;

  if ((len == 0U) || (s_hist_len < len))
  {
    return 0U;
  }

  start = (uint16_t)((s_hist_pos + APP_SWITCH_HIST_SIZE - len) % APP_SWITCH_HIST_SIZE);

  for (i = 0U; i < len; i++)
  {
    uint16_t index = (uint16_t)(start + i);

    if (index >= APP_SWITCH_HIST_SIZE)
    {
      index = (uint16_t)(index - APP_SWITCH_HIST_SIZE);
    }

    if (s_hist[index] != pattern[i])
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t switch_command_detected(void)
{
  if (switch_hist_matches(s_cmd_lf, (uint16_t)(sizeof(s_cmd_lf) - 1U)) != 0U)
  {
    return 1U;
  }

  if (switch_hist_matches(s_cmd_text, (uint16_t)(sizeof(s_cmd_text) - 1U)) != 0U)
  {
    return 1U;
  }

  return 0U;
}

uint8_t app_switch_execute(void)
{
  if (s_hmi_mode != 0U)
  {
    return BSP_OK;
  }

  /* No log output here on purpose: the acknowledge must be the only thing the
     PC sees during the hand shake. */

  /* Drain anything still queued so the acknowledge is not interleaved. */
  (void)bsp_uart_tx_flush(BSP_UART_USB, 100U);

  /* 1. Answer before touching SWITCH_EN. The blocking call returns only once
        the last stop bit has been shifted out. */
  if (bsp_uart_write_blocking(BSP_UART_USB, APP_SWITCH_ACK, APP_SWITCH_ACK_LEN, 100U) != BSP_OK)
  {
    return BSP_ERROR;
  }

  (void)bsp_uart_tx_flush(BSP_UART_USB, 100U);

  /* 2. Let the analogue switch settle after the last byte. */
  bsp_dwt_delay_ms(2U);

  /* 3. Nothing may run from now on. */
  app_sched_stop_all();

  bsp_uart_rx_enable(BSP_UART_USB, 0U);
  bsp_uart_rx_enable(BSP_UART_HMI, 0U);

#if (APP_SWITCH_KEEP_LED == 0)
  app_led_stop();
#endif

  /* 4. SWITCH_EN low: the USB-UART bridge goes to the HMI screen and the
        screen input moves from UART4 to USART1. The firmware cannot undo it. */
  bsp_switch_to_hmi();

  s_hmi_mode = 1U;

  return BSP_OK;
}

void app_switch_poll(void)
{
  uint8_t byte;

  if (s_hmi_mode != 0U)
  {
    return;
  }

  while (bsp_uart_read_byte(BSP_UART_USB, &byte) == BSP_OK)
  {
    s_byte_count++;
    switch_hist_push(byte);

    if (switch_command_detected() != 0U)
    {
      s_hist_len = 0U;

      if (app_switch_execute() == BSP_OK)
      {
        return;
      }
    }
  }
}
