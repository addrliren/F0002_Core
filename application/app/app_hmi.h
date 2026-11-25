/**
  ******************************************************************************
  * @file    app_hmi.h
  * @brief   Reserved HMI serial screen protocol (send + receive handling).
  *
  *          Frame on the wire:
  *
  *            +-----+--------+--------+---------------+------+------+------+------+
  *            | CMD | LEN_LO | LEN_HI | DATA[LEN]     | 0x01 | 0xFF | 0xFF | 0xFF |
  *            +-----+--------+--------+---------------+------+------+------+------+
  *
  *          The four trailing bytes are the check word. A frame is only
  *          accepted when its last four bytes are exactly 01 FF FF FF;
  *          anything else (0x1A for instance) is a check failure and the frame
  *          is dropped.
  *
  *          Transport: UART4 while the box is in MCU mode, USART1 once
  *          SWITCH_EN has routed the screen onto USART1.
  ******************************************************************************
  */

#ifndef __APP_HMI_H
#define __APP_HMI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

#define APP_HMI_CHECK_0         0x01U
#define APP_HMI_CHECK_1         0xFFU
#define APP_HMI_CHECK_2         0xFFU
#define APP_HMI_CHECK_3         0xFFU

#define APP_HMI_CHECK_LEN       4U
#define APP_HMI_CMD_LEN         1U
#define APP_HMI_LEN_LEN         2U
#define APP_HMI_HEADER_LEN      (APP_HMI_CMD_LEN + APP_HMI_LEN_LEN)
#define APP_HMI_MIN_FRAME_LEN   (APP_HMI_HEADER_LEN + APP_HMI_CHECK_LEN)

#define APP_HMI_MAX_DATA        64U
#define APP_HMI_MAX_FRAME_LEN   (APP_HMI_HEADER_LEN + APP_HMI_MAX_DATA + APP_HMI_CHECK_LEN)
#define APP_HMI_RX_MAX          APP_HMI_MAX_FRAME_LEN

typedef struct
{
  uint8_t  cmd;
  uint16_t len;
  uint8_t  data[APP_HMI_MAX_DATA];
} app_hmi_frame_t;

typedef void (*app_hmi_handler_t)(const app_hmi_frame_t *frame);

/* Commands reserved for the screen, extend as the HMI project grows. */
typedef enum
{
  APP_HMI_CMD_NONE          = 0x00,
  APP_HMI_CMD_PING          = 0x01,
  APP_HMI_CMD_GET_STATUS    = 0x02,
  APP_HMI_CMD_SET_OUTPUT    = 0x03,
  APP_HMI_CMD_SET_ADJ       = 0x04,
  APP_HMI_CMD_SET_BACKLIGHT = 0x05,
  APP_HMI_CMD_FACTORY_RESET = 0x06
} app_hmi_cmd_t;

void app_hmi_init(void);
void app_hmi_set_handler(app_hmi_handler_t handler);

/* Transport currently used by the HMI link. */
bsp_uart_port_t app_hmi_port(void);

/* Sends one frame, the check word is appended automatically. */
uint8_t app_hmi_send(uint8_t cmd, const void *data, uint16_t len);

/* Answers the "is the check word valid" question for a complete frame. */
uint8_t app_hmi_check_valid(const uint8_t *frame, uint16_t len);

/* Consumes the receive buffer of the active port and dispatches the frames. */
void app_hmi_poll(void);

uint32_t app_hmi_get_frame_count(void);
uint32_t app_hmi_get_error_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_HMI_H */
