/**
  ******************************************************************************
  * @file    app_hmi.c
  * @brief   HMI serial screen protocol (reserved driver).
  ******************************************************************************
  */

#include "app_hmi.h"
#include "app_switch.h"
#include "app_log.h"
#include <string.h>

#define APP_HMI_CHECK_BYTES   { APP_HMI_CHECK_0, APP_HMI_CHECK_1, \
                                APP_HMI_CHECK_2, APP_HMI_CHECK_3 }

static const uint8_t s_check[APP_HMI_CHECK_LEN] = APP_HMI_CHECK_BYTES;

static app_hmi_handler_t s_handler = 0;
static uint8_t s_rx[APP_HMI_RX_MAX];
static uint16_t s_rx_len = 0U;
static uint32_t s_frame_count = 0U;
static uint32_t s_error_count = 0U;
static uint8_t s_tx[APP_HMI_RX_MAX];

void app_hmi_init(void)
{
  s_handler = 0;
  s_rx_len = 0U;
  s_frame_count = 0U;
  s_error_count = 0U;

  memset(s_rx, 0, sizeof(s_rx));
}

void app_hmi_set_handler(app_hmi_handler_t handler)
{
  s_handler = handler;
}

bsp_uart_port_t app_hmi_port(void)
{
  return (app_switch_is_hmi_mode() != 0U) ? BSP_UART_USB : BSP_UART_HMI;
}

uint8_t app_hmi_check_valid(const uint8_t *frame, uint16_t len)
{
  if ((frame == 0) || (len < APP_HMI_MIN_FRAME_LEN))
  {
    return 0U;
  }

  return (memcmp(&frame[len - APP_HMI_CHECK_LEN], s_check, APP_HMI_CHECK_LEN) == 0) ? 1U : 0U;
}

uint8_t app_hmi_send(uint8_t cmd, const void *data, uint16_t len)
{
  const uint8_t *payload = (const uint8_t *)data;
  uint16_t total;

  if (len > APP_HMI_MAX_DATA)
  {
    return BSP_ERROR;
  }

  if ((len > 0U) && (payload == 0))
  {
    return BSP_ERROR;
  }

  s_tx[0] = cmd;
  s_tx[1] = (uint8_t)(len & 0xFFU);
  s_tx[2] = (uint8_t)((len >> 8) & 0xFFU);

  if (len > 0U)
  {
    memcpy(&s_tx[APP_HMI_HEADER_LEN], payload, len);
  }

  memcpy(&s_tx[APP_HMI_HEADER_LEN + len], s_check, APP_HMI_CHECK_LEN);

  total = (uint16_t)(APP_HMI_HEADER_LEN + len + APP_HMI_CHECK_LEN);

  return (bsp_uart_write(app_hmi_port(), s_tx, total) == total) ? BSP_OK : BSP_ERROR;
}

static void hmi_dispatch(const uint8_t *frame)
{
  app_hmi_frame_t parsed;

  parsed.cmd = frame[0];
  parsed.len = (uint16_t)frame[1] | ((uint16_t)frame[2] << 8);

  if (parsed.len > APP_HMI_MAX_DATA)
  {
    s_error_count++;
    return;
  }

  if (parsed.len > 0U)
  {
    memcpy(parsed.data, &frame[APP_HMI_HEADER_LEN], parsed.len);
  }

  s_frame_count++;

  if (s_handler != 0)
  {
    s_handler(&parsed);
  }
  else
  {
    APP_LOGW("HMI", "cmd 0x%02X ignored, no handler", (unsigned)parsed.cmd);
  }
}

void app_hmi_poll(void)
{
  uint8_t byte;
  bsp_uart_port_t port = app_hmi_port();

  while (bsp_uart_read_byte(port, &byte) == BSP_OK)
  {
    if (s_rx_len >= APP_HMI_RX_MAX)
    {
      s_rx_len = 0U;
      s_error_count++;
    }

    s_rx[s_rx_len] = byte;
    s_rx_len++;

    for (;;)
    {
      uint16_t payload_len;
      uint16_t total;

      if (s_rx_len < APP_HMI_HEADER_LEN)
      {
        break;
      }

      payload_len = (uint16_t)s_rx[1] | ((uint16_t)s_rx[2] << 8);

      if (payload_len > APP_HMI_MAX_DATA)
      {
        /* Not a header: shift one byte to resynchronise. */
        memmove(s_rx, &s_rx[1], (size_t)(s_rx_len - 1U));
        s_rx_len--;
        s_error_count++;
        continue;
      }

      total = (uint16_t)(APP_HMI_HEADER_LEN + payload_len + APP_HMI_CHECK_LEN);

      if (s_rx_len < total)
      {
        break;
      }

      if (app_hmi_check_valid(s_rx, total) == 0U)
      {
        memmove(s_rx, &s_rx[1], (size_t)(s_rx_len - 1U));
        s_rx_len--;
        s_error_count++;
        continue;
      }

      hmi_dispatch(s_rx);

      if (s_rx_len > total)
      {
        memmove(s_rx, &s_rx[total], (size_t)(s_rx_len - total));
      }

      s_rx_len = (uint16_t)(s_rx_len - total);
    }
  }
}

uint32_t app_hmi_get_frame_count(void)
{
  return s_frame_count;
}

uint32_t app_hmi_get_error_count(void)
{
  return s_error_count;
}
