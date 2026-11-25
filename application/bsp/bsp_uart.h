/**
  ******************************************************************************
  * @file    bsp_uart.h
  * @brief   Interrupt driven UART transport with RX/TX ring buffers.
  *
  *  Port mapping:
  *    BSP_UART_USB : USART1 (PA9/PA10) -> USB to serial bridge, debug console
  *    BSP_UART_HMI : UART4  (PC10/PC11) -> HMI serial screen
  ******************************************************************************
  */

#ifndef __BSP_UART_H
#define __BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

#define BSP_UART_RX_BUF_SIZE    512U
#define BSP_UART_TX_BUF_SIZE    512U

typedef enum
{
  BSP_UART_USB = 0,
  BSP_UART_HMI = 1,
  BSP_UART_COUNT
} bsp_uart_port_t;

/* Start the RX interrupt of every port. Call once after MX_xxx_Init(). */
void bsp_uart_init(void);

/* Buffered, non blocking write. Returns the number of bytes queued. */
uint32_t bsp_uart_write(bsp_uart_port_t port, const void *data, uint32_t len);
uint32_t bsp_uart_write_str(bsp_uart_port_t port, const char *str);

/* Blocking write, used when the answer must be on the wire before the next
   action (e.g. the SWITCH_EN hand shake). Returns BSP_OK / BSP_ERROR. */
uint8_t bsp_uart_write_blocking(bsp_uart_port_t port, const void *data,
                                uint32_t len, uint32_t timeout_ms);

/* Ring buffer access. */
uint32_t bsp_uart_read(bsp_uart_port_t port, void *buf, uint32_t len);
uint8_t  bsp_uart_read_byte(bsp_uart_port_t port, uint8_t *byte);
uint32_t bsp_uart_available(bsp_uart_port_t port);
void     bsp_uart_rx_flush(bsp_uart_port_t port);

/* Wait until the TX ring has been drained. */
uint8_t bsp_uart_tx_flush(bsp_uart_port_t port, uint32_t timeout_ms);
uint8_t bsp_uart_tx_busy(bsp_uart_port_t port);

/* Enable / disable reception (and the RX interrupt) of a port. */
void bsp_uart_rx_enable(bsp_uart_port_t port, uint8_t enable);

/* Recovery hook, call from the main loop. */
void bsp_uart_poll(void);

uint32_t bsp_uart_get_rx_overflow(bsp_uart_port_t port);
uint32_t bsp_uart_get_tx_drop(bsp_uart_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_UART_H */
