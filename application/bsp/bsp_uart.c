/**
  ******************************************************************************
  * @file    bsp_uart.c
  * @brief   Interrupt driven UART transport with RX/TX ring buffers.
  *
  *  The receive path uses a one byte HAL interrupt; every received byte is
  *  pushed into the RX ring buffer and the interrupt is re-armed. That keeps
  *  the ISR short and makes the driver insensitive to frame length.
  ******************************************************************************
  */

#include "bsp_uart.h"
#include "usart.h"

#define RX_MASK     (BSP_UART_RX_BUF_SIZE - 1U)
#define TX_MASK     (BSP_UART_TX_BUF_SIZE - 1U)

/* Ring sizes must be a power of two. */
#if ((BSP_UART_RX_BUF_SIZE & RX_MASK) != 0U) || (BSP_UART_RX_BUF_SIZE < 2U)
#error "BSP_UART_RX_BUF_SIZE must be a power of two"
#endif
#if ((BSP_UART_TX_BUF_SIZE & TX_MASK) != 0U) || (BSP_UART_TX_BUF_SIZE < 2U)
#error "BSP_UART_TX_BUF_SIZE must be a power of two"
#endif

typedef struct
{
  UART_HandleTypeDef *huart;

  uint8_t           rx_buf[BSP_UART_RX_BUF_SIZE];
  volatile uint16_t rx_head;
  volatile uint16_t rx_tail;

  uint8_t           tx_buf[BSP_UART_TX_BUF_SIZE];
  volatile uint16_t tx_head;
  volatile uint16_t tx_tail;

  uint8_t           rx_byte;
  uint8_t           tx_byte;

  volatile uint8_t  rx_armed;
  volatile uint8_t  tx_busy;
  volatile uint8_t  rx_enabled;

  volatile uint32_t rx_overflow;
  volatile uint32_t tx_drop;
} bsp_uart_ctx_t;

static bsp_uart_ctx_t s_uart[BSP_UART_COUNT];

static uint32_t uart_irq_save(void)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  return primask;
}

static void uart_irq_restore(uint32_t primask)
{
  __set_PRIMASK(primask);
}

static bsp_uart_ctx_t *uart_ctx(bsp_uart_port_t port)
{
  if ((uint32_t)port >= (uint32_t)BSP_UART_COUNT)
  {
    return 0;
  }

  return &s_uart[port];
}

static bsp_uart_ctx_t *uart_ctx_from_handle(UART_HandleTypeDef *huart)
{
  uint8_t i;

  for (i = 0U; i < (uint8_t)BSP_UART_COUNT; i++)
  {
    if (s_uart[i].huart == huart)
    {
      return &s_uart[i];
    }
  }

  return 0;
}

static void uart_rx_arm(bsp_uart_ctx_t *c)
{
  HAL_StatusTypeDef st;

  if ((c == 0) || (c->rx_armed != 0U) || (c->rx_enabled == 0U))
  {
    return;
  }

  st = HAL_UART_Receive_IT(c->huart, &c->rx_byte, 1U);
  if ((st == HAL_OK) || (st == HAL_BUSY))
  {
    c->rx_armed = 1U;
  }
}

static void uart_tx_kick(bsp_uart_ctx_t *c)
{
  uint32_t primask;

  if ((c == 0) || (c->tx_busy != 0U))
  {
    return;
  }

  primask = uart_irq_save();
  if ((c->tx_busy == 0U) && (c->tx_head != c->tx_tail))
  {
    c->tx_byte = c->tx_buf[c->tx_tail];
    c->tx_tail = (uint16_t)((c->tx_tail + 1U) & TX_MASK);
    c->tx_busy = 1U;
  }
  uart_irq_restore(primask);

  if (c->tx_busy == 0U)
  {
    return;
  }

  if (HAL_UART_Transmit_IT(c->huart, &c->tx_byte, 1U) != HAL_OK)
  {
    /* UART busy or error: roll the byte back, bsp_uart_poll() retries. */
    primask = uart_irq_save();
    c->tx_busy = 0U;
    c->tx_tail = (uint16_t)((c->tx_tail + TX_MASK) & TX_MASK);
    c->tx_drop++;
    uart_irq_restore(primask);
  }
}

void bsp_uart_init(void)
{
  uint8_t i;

  for (i = 0U; i < (uint8_t)BSP_UART_COUNT; i++)
  {
    bsp_uart_ctx_t *c = &s_uart[i];

    c->huart       = (i == (uint8_t)BSP_UART_USB) ? &huart1 : &huart4;
    c->rx_head     = 0U;
    c->rx_tail     = 0U;
    c->tx_head     = 0U;
    c->tx_tail     = 0U;
    c->rx_armed    = 0U;
    c->tx_busy     = 0U;
    c->rx_enabled  = 1U;
    c->rx_overflow = 0U;
    c->tx_drop     = 0U;

    uart_rx_arm(c);
  }
}

uint32_t bsp_uart_write(bsp_uart_port_t port, const void *data, uint32_t len)
{
  bsp_uart_ctx_t *c = uart_ctx(port);
  const uint8_t  *src = (const uint8_t *)data;
  uint32_t        written = 0U;
  uint32_t        primask;

  if ((c == 0) || (src == 0) || (len == 0U))
  {
    return 0U;
  }

  primask = uart_irq_save();
  while (written < len)
  {
    uint16_t next = (uint16_t)((c->tx_head + 1U) & TX_MASK);

    if (next == c->tx_tail)
    {
      break;
    }

    c->tx_buf[c->tx_head] = src[written];
    c->tx_head = next;
    written++;
  }
  uart_irq_restore(primask);

  if (written < len)
  {
    c->tx_drop += (len - written);
  }

  uart_tx_kick(c);

  return written;
}

uint32_t bsp_uart_write_str(bsp_uart_port_t port, const char *str)
{
  uint32_t len = 0U;

  if (str == 0)
  {
    return 0U;
  }

  while (str[len] != '\0')
  {
    len++;
  }

  return bsp_uart_write(port, str, len);
}

uint8_t bsp_uart_write_blocking(bsp_uart_port_t port, const void *data,
                                uint32_t len, uint32_t timeout_ms)
{
  bsp_uart_ctx_t *c = uart_ctx(port);

  if ((c == 0) || (data == 0) || (len == 0U))
  {
    return BSP_ERROR;
  }

  if (bsp_uart_tx_flush(port, timeout_ms) != BSP_OK)
  {
    return BSP_ERROR;
  }

  if (HAL_UART_Transmit(c->huart, (uint8_t *)data, (uint16_t)len, timeout_ms) != HAL_OK)
  {
    return BSP_ERROR;
  }

  return BSP_OK;
}

uint32_t bsp_uart_read(bsp_uart_port_t port, void *buf, uint32_t len)
{
  bsp_uart_ctx_t *c = uart_ctx(port);
  uint8_t        *dst = (uint8_t *)buf;
  uint32_t        count = 0U;
  uint32_t        primask;

  if ((c == 0) || (dst == 0) || (len == 0U))
  {
    return 0U;
  }

  primask = uart_irq_save();
  while ((count < len) && (c->rx_tail != c->rx_head))
  {
    dst[count] = c->rx_buf[c->rx_tail];
    c->rx_tail = (uint16_t)((c->rx_tail + 1U) & RX_MASK);
    count++;
  }
  uart_irq_restore(primask);

  return count;
}

uint8_t bsp_uart_read_byte(bsp_uart_port_t port, uint8_t *byte)
{
  if (byte == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_uart_read(port, byte, 1U) != 1U)
  {
    return BSP_ERROR;
  }

  return BSP_OK;
}

uint32_t bsp_uart_available(bsp_uart_port_t port)
{
  bsp_uart_ctx_t *c = uart_ctx(port);

  if (c == 0)
  {
    return 0U;
  }

  return ((uint32_t)c->rx_head - (uint32_t)c->rx_tail) & RX_MASK;
}

void bsp_uart_rx_flush(bsp_uart_port_t port)
{
  bsp_uart_ctx_t *c = uart_ctx(port);
  uint32_t        primask;

  if (c == 0)
  {
    return;
  }

  primask = uart_irq_save();
  c->rx_tail = c->rx_head;
  uart_irq_restore(primask);
}

uint8_t bsp_uart_tx_busy(bsp_uart_port_t port)
{
  bsp_uart_ctx_t *c = uart_ctx(port);

  if (c == 0)
  {
    return 0U;
  }

  if ((c->tx_busy != 0U) || (c->tx_head != c->tx_tail))
  {
    return 1U;
  }

  return 0U;
}

uint8_t bsp_uart_tx_flush(bsp_uart_port_t port, uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();

  while (bsp_uart_tx_busy(port) != 0U)
  {
    if ((uint32_t)(HAL_GetTick() - start) >= timeout_ms)
    {
      return BSP_ERROR;
    }
  }

  return BSP_OK;
}

void bsp_uart_rx_enable(bsp_uart_port_t port, uint8_t enable)
{
  bsp_uart_ctx_t *c = uart_ctx(port);

  if (c == 0)
  {
    return;
  }

  if (enable != 0U)
  {
    c->rx_enabled = 1U;
    uart_rx_arm(c);
  }
  else
  {
    c->rx_enabled = 0U;

    if (c->rx_armed != 0U)
    {
      (void)HAL_UART_AbortReceive(c->huart);
      c->rx_armed = 0U;
    }

    bsp_uart_rx_flush(port);
  }
}

void bsp_uart_poll(void)
{
  uint8_t i;

  for (i = 0U; i < (uint8_t)BSP_UART_COUNT; i++)
  {
    bsp_uart_ctx_t *c = &s_uart[i];

    uart_rx_arm(c);
    uart_tx_kick(c);
  }
}

uint32_t bsp_uart_get_rx_overflow(bsp_uart_port_t port)
{
  bsp_uart_ctx_t *c = uart_ctx(port);

  return (c == 0) ? 0U : c->rx_overflow;
}

uint32_t bsp_uart_get_tx_drop(bsp_uart_port_t port)
{
  bsp_uart_ctx_t *c = uart_ctx(port);

  return (c == 0) ? 0U : c->tx_drop;
}

/* HAL callbacks ------------------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  bsp_uart_ctx_t *c = uart_ctx_from_handle(huart);

  if (c == 0)
  {
    return;
  }

  {
    uint16_t next = (uint16_t)((c->rx_head + 1U) & RX_MASK);

    if (next != c->rx_tail)
    {
      c->rx_buf[c->rx_head] = c->rx_byte;
      c->rx_head = next;
    }
    else
    {
      c->rx_overflow++;
    }
  }

  c->rx_armed = 0U;
  uart_rx_arm(c);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  bsp_uart_ctx_t *c = uart_ctx_from_handle(huart);

  if (c == 0)
  {
    return;
  }

  c->tx_busy = 0U;
  uart_tx_kick(c);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  bsp_uart_ctx_t *c = uart_ctx_from_handle(huart);
  volatile uint32_t tmp;

  if (c == 0)
  {
    return;
  }

  /* Clear overrun / framing / noise: reading SR then DR clears them. */
  tmp = huart->Instance->SR;
  tmp = huart->Instance->DR;
  BSP_UNUSED(tmp);

  huart->ErrorCode = HAL_UART_ERROR_NONE;
  c->rx_armed = 0U;
  uart_rx_arm(c);

  /* A transmit that died on an error must not stall the ring buffer. */
  if (c->tx_busy != 0U)
  {
    c->tx_busy = 0U;
    c->tx_drop++;
    uart_tx_kick(c);
  }
}
