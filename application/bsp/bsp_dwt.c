/**
  ******************************************************************************
  * @file    bsp_dwt.c
  * @brief   DWT cycle counter based delay / time base.
  ******************************************************************************
  */

#include "bsp_dwt.h"

/* Longest single blocking chunk used by bsp_dwt_delay_us(), keeps the
   uint32_t cycle arithmetic inside one wrap window even at high HCLK. */
#define DWT_CHUNK_US            1000U

static uint32_t s_cycles_per_us = 72U;
static uint32_t s_last_cycles   = 0U;
static uint64_t s_total_cycles  = 0U;
static uint8_t  s_ready         = 0U;

static uint32_t dwt_irq_save(void)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  return primask;
}

static void dwt_irq_restore(uint32_t primask)
{
  __set_PRIMASK(primask);
}

/* Best effort delay when the DWT is not available (debugger attached oddities,
   CYCCNT not implemented). Calibrated roughly for 72 MHz. */
static void dwt_fallback_delay_us(uint32_t us)
{
  volatile uint32_t i;

  while (us-- > 0U)
  {
    for (i = 0U; i < 12U; i++)
    {
      __NOP();
    }
  }
}

void bsp_dwt_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

#if defined(DWT_CTRL_NOCYCCNT_Msk)
  if ((DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) != 0U)
  {
    s_ready = 0U;
    return;
  }
#endif

  DWT->CYCCNT = 0U;
  DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

  s_cycles_per_us = SystemCoreClock / 1000000U;
  if (s_cycles_per_us == 0U)
  {
    s_cycles_per_us = 1U;
  }

  /* Verify that the counter actually counts. */
  {
    uint32_t start = DWT->CYCCNT;
    uint32_t guard = 0U;

    while ((DWT->CYCCNT == start) && (guard < 1000U))
    {
      guard++;
    }
    s_ready = (DWT->CYCCNT != start) ? 1U : 0U;
  }

  s_last_cycles  = DWT->CYCCNT;
  s_total_cycles = 0U;
}

uint8_t bsp_dwt_is_ready(void)
{
  return s_ready;
}

uint32_t bsp_dwt_get_cycles(void)
{
  return DWT->CYCCNT;
}

uint32_t bsp_dwt_get_us(void)
{
  if (s_ready == 0U)
  {
    return HAL_GetTick() * 1000U;
  }

  return DWT->CYCCNT / s_cycles_per_us;
}

uint32_t bsp_dwt_get_ms(void)
{
  if (s_ready == 0U)
  {
    return HAL_GetTick();
  }

  {
    uint32_t primask = dwt_irq_save();
    uint32_t now     = DWT->CYCCNT;
    uint64_t total;

    s_total_cycles += (uint32_t)(now - s_last_cycles);
    s_last_cycles   = now;
    total           = s_total_cycles;
    dwt_irq_restore(primask);

    return (uint32_t)(total / ((uint64_t)s_cycles_per_us * 1000ULL));
  }
}

void bsp_dwt_delay_us(uint32_t us)
{
  if (us == 0U)
  {
    return;
  }

  if (s_ready == 0U)
  {
    dwt_fallback_delay_us(us);
    return;
  }

  while (us > 0U)
  {
    uint32_t chunk = (us > DWT_CHUNK_US) ? DWT_CHUNK_US : us;
    uint32_t start = DWT->CYCCNT;
    uint32_t wait  = chunk * s_cycles_per_us;

    while ((uint32_t)(DWT->CYCCNT - start) < wait)
    {
      /* busy wait, deliberately not masked so UART/systick stay responsive */
    }

    us -= chunk;
  }
}

void bsp_dwt_delay_ms(uint32_t ms)
{
  while (ms-- > 0U)
  {
    bsp_dwt_delay_us(1000U);
  }
}
