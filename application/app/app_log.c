/**
  ******************************************************************************
  * @file    app_log.c
  * @brief   Debug log over USART1.
  ******************************************************************************
  */

#include "app_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static app_log_level_t s_level = APP_LOG_LEVEL_DEFAULT;
static volatile uint8_t s_busy = 0U;
static uint32_t s_dropped = 0U;
static char s_buffer[APP_LOG_BUF_SIZE];

static const char *log_level_tag(app_log_level_t level)
{
  switch (level)
  {
    case APP_LOG_LEVEL_ERROR: return "E";
    case APP_LOG_LEVEL_WARN:  return "W";
    case APP_LOG_LEVEL_INFO:  return "I";
    case APP_LOG_LEVEL_DEBUG: return "D";
    default:                  return "-";
  }
}

void app_log_init(void)
{
  s_level = APP_LOG_LEVEL_DEFAULT;
  s_busy = 0U;
  s_dropped = 0U;

  app_log_write(APP_LOG_LEVEL_INFO, "LOG", "logger started, level %u",
                (unsigned)s_level);
}

void app_log_set_level(app_log_level_t level)
{
  s_level = level;
}

app_log_level_t app_log_get_level(void)
{
  return s_level;
}

uint32_t app_log_get_dropped(void)
{
  return s_dropped;
}

void app_log_write(app_log_level_t level, const char *tag, const char *fmt, ...)
{
  va_list args;
  int header;
  int body;
  uint32_t total;

  if ((level == APP_LOG_LEVEL_NONE) || (level > s_level) || (fmt == 0))
  {
    return;
  }

  if (s_busy != 0U)
  {
    s_dropped++;
    return;
  }

  s_busy = 1U;

  header = snprintf(s_buffer, APP_LOG_BUF_SIZE, "[%8lu] %s/%s: ",
                    (unsigned long)bsp_dwt_get_ms(),
                    log_level_tag(level),
                    (tag != 0) ? tag : "-");

  if (header < 0)
  {
    s_busy = 0U;
    return;
  }

  if ((uint32_t)header >= APP_LOG_BUF_SIZE - 2U)
  {
    header = (int)(APP_LOG_BUF_SIZE - 2U);
  }

  va_start(args, fmt);
  body = vsnprintf(&s_buffer[header], APP_LOG_BUF_SIZE - (uint32_t)header - 1U, fmt, args);
  va_end(args);

  if (body < 0)
  {
    s_busy = 0U;
    return;
  }

  total = (uint32_t)header + (uint32_t)body;

  if (total > APP_LOG_BUF_SIZE - 3U)
  {
    total = APP_LOG_BUF_SIZE - 3U;
  }

  s_buffer[total++] = '\r';
  s_buffer[total++] = '\n';

  (void)bsp_uart_write(BSP_UART_USB, s_buffer, total);

  s_busy = 0U;
}

void app_log_puts(app_log_level_t level, const char *tag, const char *text)
{
  app_log_write(level, tag, "%s", (text != 0) ? text : "");
}

void app_log_hex(app_log_level_t level, const char *tag, const void *data, uint32_t len)
{
  const uint8_t *bytes = (const uint8_t *)data;
  char line[APP_LOG_BUF_SIZE];
  uint32_t offset = 0U;

  if ((bytes == 0) || (len == 0U))
  {
    return;
  }

  while (offset < len)
  {
    uint32_t column = 0U;
    int written = 0;

    written = snprintf(line, sizeof(line), "%04lx: ", (unsigned long)offset);

    if (written < 0)
    {
      return;
    }

    while ((column < 16U) && ((offset + column) < len))
    {
      int room = (int)sizeof(line) - written;

      if (room < 4)
      {
        break;
      }

      written += snprintf(&line[written], (size_t)room, "%02X ",
                          bytes[offset + column]);
      column++;
    }

    app_log_puts(level, tag, line);

    offset += column;
  }
}
