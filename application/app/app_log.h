/**
  ******************************************************************************
  * @file    app_log.h
  * @brief   Debug log over the USB-UART bridge (USART1).
  *
  *          The logger is non blocking: the formatted text is queued into the
  *          USART1 TX ring buffer. Messages produced while another message is
  *          being formatted (typically from an interrupt) are dropped.
  ******************************************************************************
  */

#ifndef __APP_LOG_H
#define __APP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

typedef enum
{
  APP_LOG_LEVEL_NONE = 0,
  APP_LOG_LEVEL_ERROR,
  APP_LOG_LEVEL_WARN,
  APP_LOG_LEVEL_INFO,
  APP_LOG_LEVEL_DEBUG
} app_log_level_t;

#ifndef APP_LOG_LEVEL_DEFAULT
#define APP_LOG_LEVEL_DEFAULT   APP_LOG_LEVEL_INFO
#endif

#define APP_LOG_BUF_SIZE        160U

void app_log_init(void);
void app_log_set_level(app_log_level_t level);
app_log_level_t app_log_get_level(void);

void app_log_write(app_log_level_t level, const char *tag, const char *fmt, ...);
void app_log_puts(app_log_level_t level, const char *tag, const char *text);
void app_log_hex(app_log_level_t level, const char *tag, const void *data,
                 uint32_t len);

uint32_t app_log_get_dropped(void);

#define APP_LOGE(tag, ...)  app_log_write(APP_LOG_LEVEL_ERROR, tag, __VA_ARGS__)
#define APP_LOGW(tag, ...)  app_log_write(APP_LOG_LEVEL_WARN,  tag, __VA_ARGS__)
#define APP_LOGI(tag, ...)  app_log_write(APP_LOG_LEVEL_INFO,  tag, __VA_ARGS__)
#define APP_LOGD(tag, ...)  app_log_write(APP_LOG_LEVEL_DEBUG, tag, __VA_ARGS__)
#define APP_LOGHEX(tag, data, len) app_log_hex(APP_LOG_LEVEL_DEBUG, tag, data, len)

#ifdef __cplusplus
}
#endif

#endif /* __APP_LOG_H */
